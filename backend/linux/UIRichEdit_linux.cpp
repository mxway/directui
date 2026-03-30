#include <UIRichEdit.h>

#include <UIFont.h>
#include <algorithm>
#include <cstring>
#include <unordered_map>
#include <pango/pango-layout.h>

#include "UIRenderEngine.h"

#ifdef GTK_BACKEND
#include <pango/pangocairo.h>
#else
#include <pango/pangoxft.h>
#include "X11/X11HDC.h"
#include "X11/X11Window.h"
#endif

#include "../../src/UIRichEditInternal_impl.h"

namespace {

struct TextStyleKey {
    int fontSize;
    bool bold;
    bool italic;
    std::string fontFamily;

    bool operator==(const TextStyleKey& other) const {
        return fontSize == other.fontSize &&
               bold == other.bold &&
               italic == other.italic &&
               fontFamily == other.fontFamily;
    }
};

struct TextStyleKeyHash {
    size_t operator()(const TextStyleKey& key) const {
        size_t h = std::hash<int>{}(key.fontSize);
        h ^= (std::hash<bool>{}(key.bold) << 1);
        h ^= (std::hash<bool>{}(key.italic) << 2);
        h ^= (std::hash<std::string>{}(key.fontFamily) << 3);
        return h;
    }
};

PangoContext* CreateRichEditPangoContext(HANDLE_DC hdc) {
#ifdef GTK_BACKEND
    return pango_cairo_create_context(hdc);
#else
    PangoFontMap* fontMap = pango_xft_get_font_map(hdc->x11Window->display, hdc->x11Window->screen);
    return pango_font_map_create_context(fontMap);
#endif
}

void ApplyFontDescription(PangoLayout* layout, HANDLE_FONT font) {
    if (font != nullptr) {
        pango_layout_set_font_description(layout, font);
    }
}

HANDLE_FONT GetCachedFont(const TextStyle& style) {
    static std::unordered_map<TextStyleKey, HANDLE_FONT, TextStyleKeyHash> s_fontCache;
    TextStyleKey key{style.fontSize, style.bold, style.italic, style.fontFamily};
    auto it = s_fontCache.find(key);
    if (it != s_fontCache.end()) {
        return it->second;
    }

    HANDLE_FONT font = CreateFontFromStyle(style);
    if (font != nullptr) {
        s_fontCache.emplace(std::move(key), font);
    }
    return font;
}

} // namespace

void TextRun::SetFontFamily(const UIString& fontFamily) {
    style.fontFamily.assign(fontFamily.GetData(), fontFamily.GetLength());
}

void TextRun::SetText(const UIString& utf8Text) {
    text.assign(utf8Text.GetData(), utf8Text.GetLength());
}

bool IsNewLine(char ch) {
    return ch == '\n' || ch == '\r';
}

bool IsBreakable(char c) {
    return c == ' ' || c == '\t' || c == '-' || c == ',' || c == ';' || c == ':';
}

HANDLE_FONT CreateFontFromStyle(const TextStyle& s) {
    PangoFontDescription* desc = pango_font_description_new();
    if (desc == nullptr) {
        return nullptr;
    }

    if (!s.fontFamily.empty()) {
        pango_font_description_set_family(desc, s.fontFamily.c_str());
    }

    pango_font_description_set_absolute_size(desc, s.fontSize * PANGO_SCALE);
    pango_font_description_set_weight(desc, s.bold ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL);
    pango_font_description_set_style(desc, s.italic ? PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL);
    return desc;
}

int MeasureTextWidth(HANDLE_DC hdc, const TextStyle& st, const std::string& text) {
    return MeasureTextWidthRange(hdc, st, text.c_str(), static_cast<int>(text.size()));
}

int MeasureTextWidthRange(HANDLE_DC hdc, const TextStyle& st, const char* text, int length) {
    if (text == nullptr || length <= 0) {
        return 0;
    }
    HANDLE_FONT font = GetCachedFont(st);
    PangoContext* context = CreateRichEditPangoContext(hdc);
    PangoLayout* layout = pango_layout_new(context);
    ApplyFontDescription(layout, font);
    pango_layout_set_text(layout, text, length);

    int width = 0;
    int height = 0;
    pango_layout_get_pixel_size(layout, &width, &height);

    g_object_unref(layout);
    g_object_unref(context);
    return width;
}

int GetTextFitMetrics(HANDLE_DC hdc, const TextStyle& st, const char* text, int length, int maxWidth,
                      int* fitWidth) {
    if (fitWidth) {
        *fitWidth = 0;
    }
    if (text == nullptr || length <= 0 || maxWidth <= 0) {
        return 0;
    }
    HANDLE_FONT font = GetCachedFont(st);
    PangoContext* context = CreateRichEditPangoContext(hdc);
    PangoLayout* layout = pango_layout_new(context);
    ApplyFontDescription(layout, font);
    pango_layout_set_text(layout, text, length);
    pango_layout_set_single_paragraph_mode(layout, TRUE);
    pango_layout_set_width(layout, -1);

    int index = 0;
    int trailing = 0;
    pango_layout_xy_to_index(layout, maxWidth * PANGO_SCALE, 0, &index, &trailing);

    int fitCount = std::max(0, std::min(index, length));
    if (fitWidth) {
        PangoRectangle strongPos = {};
        pango_layout_get_cursor_pos(layout, fitCount, &strongPos, nullptr);
        *fitWidth = std::max(0, PANGO_PIXELS(strongPos.x));
    }

    g_object_unref(layout);
    g_object_unref(context);
    return fitCount;
}

void GetTextMetricsForStyle(HANDLE_DC hdc, const TextStyle& st, int& ascent, int& descent, int& lineHeight) {
    HANDLE_FONT font = GetCachedFont(st);
    PangoContext* context = CreateRichEditPangoContext(hdc);
    PangoFontMetrics* metrics = pango_context_get_metrics(context, font, pango_context_get_language(context));

    ascent = PANGO_PIXELS(pango_font_metrics_get_ascent(metrics));
    descent = PANGO_PIXELS(pango_font_metrics_get_descent(metrics)) + st.lineExtra;
    lineHeight = ascent + descent;

    pango_font_metrics_unref(metrics);
    g_object_unref(context);
}

void DrawTextRunSegment(HANDLE_DC hdc, const TextStyle& st, const UIRect& rc, const std::string& text) {
    if (text.empty()) {
        return;
    }
    HANDLE_FONT  font = GetCachedFont(st);
    RECT tmpRc = {rc.left, rc.top, rc.right, rc.bottom};
    UIRenderEngine::DrawText(hdc,tmpRc,UIString{text.c_str()},st.textColor,font,0);
}

size_t ConsumeNewLine(const std::string& text, size_t index) {
    if (index >= text.size()) {
        return 0;
    }

    if (text[index] == '\r') {
        return (index + 1 < text.size() && text[index + 1] == '\n') ? 2 : 1;
    }

    return text[index] == '\n' ? 1 : 0;
}

size_t NextTextUnit(const std::string& text, size_t index) {
    if (index >= text.size()) {
        return text.size();
    }

    const char* begin = text.c_str();
    const char* next = CharNext(begin + index);
    return std::min(text.size(), static_cast<size_t>(next - begin));
}

uint32_t DecodeUtf8CodePoint(const std::string& text, size_t index) {
    if (index >= text.size()) {
        return 0;
    }

    const unsigned char lead = static_cast<unsigned char>(text[index]);
    if ((lead & 0x80U) == 0U) {
        return lead;
    }

    const size_t nextIndex = NextTextUnit(text, index);
    const size_t length = nextIndex - index;
    if (length == 2 && index + 1 < text.size()) {
        return ((lead & 0x1FU) << 6U) |
               (static_cast<unsigned char>(text[index + 1]) & 0x3FU);
    }

    if (length == 3 && index + 2 < text.size()) {
        return ((lead & 0x0FU) << 12U) |
               ((static_cast<unsigned char>(text[index + 1]) & 0x3FU) << 6U) |
               (static_cast<unsigned char>(text[index + 2]) & 0x3FU);
    }

    if (length == 4 && index + 3 < text.size()) {
        return ((lead & 0x07U) << 18U) |
               ((static_cast<unsigned char>(text[index + 1]) & 0x3FU) << 12U) |
               ((static_cast<unsigned char>(text[index + 2]) & 0x3FU) << 6U) |
               (static_cast<unsigned char>(text[index + 3]) & 0x3FU);
    }

    return lead;
}

bool IsBreakableCodePoint(uint32_t codePoint) {
    switch (codePoint) {
        case ' ':
        case '\t':
        case '-':
        case ',':
        case ';':
        case ':':
        case 0xFF0C: // ，
        case 0x3002: // 。
        case 0xFF1B: // ；
        case 0xFF1A: // ：
        case 0x3001: // 、
            return true;
        default:
            return false;
    }
}

bool IsBreakableAt(const std::string& text, size_t index) {
    return IsBreakableCodePoint(DecodeUtf8CodePoint(text, index));
}