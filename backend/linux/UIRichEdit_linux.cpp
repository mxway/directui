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
#include <pango/pangoft2.h>
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

PangoContext* CreateRichEditMeasurePangoContext(HANDLE_DC hdc) {
#ifdef GTK_BACKEND
    return pango_cairo_create_context(hdc);
#else
    static PangoFontMap* s_measureFontMap = []() -> PangoFontMap* {
        PangoFontMap* fontMap = pango_ft2_font_map_new();
        pango_ft2_font_map_set_resolution(PANGO_FT2_FONT_MAP(fontMap), 96, 96);
        return fontMap;
    }();
    return pango_font_map_create_context(s_measureFontMap);
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

void ComputeWrappedTextSlices(HANDLE_DC hdc,
                              const TextStyle& st,
                              const char* text,
                              int length,
                              int lineWidth,
                              std::vector<WrappedTextSlice>& outSlices) {
    outSlices.clear();
    if (text == nullptr || length <= 0 || lineWidth <= 0) {
        return;
    }

    HANDLE_FONT font = GetCachedFont(st);
    PangoContext* context = CreateRichEditMeasurePangoContext(hdc);
    PangoLayout* layout = pango_layout_new(context);
    ApplyFontDescription(layout, font);
    pango_layout_set_text(layout, text, length);
    pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);
    pango_layout_set_width(layout, lineWidth * PANGO_SCALE);

    PangoLayoutIter* iter = pango_layout_get_iter(layout);
    do {
        const int startIndex = pango_layout_iter_get_index(iter);
        PangoLayoutLine* line = pango_layout_iter_get_line_readonly(iter);
        if (line == nullptr) {
            continue;
        }

        int lineLength = std::max(0, line->length);
        if (lineLength == 0 && startIndex < length) {
            const char* begin = text;
            const char* current = begin + startIndex;
            const char* next = CharNext(current);
            if (next > current) {
                lineLength = static_cast<int>(next - current);
            }
        }

        const int clampedStart = std::max(0, std::min(startIndex, length));
        const int available = std::max(0, length - clampedStart);
        const int clampedLength = std::max(0, std::min(lineLength, available));
        if (clampedLength <= 0) {
            continue;
        }

        PangoRectangle logicalRect = {};
        pango_layout_line_get_extents(line, nullptr, &logicalRect);
        const int width = std::max(0, PANGO_PIXELS(logicalRect.width));
        outSlices.emplace_back(static_cast<size_t>(clampedStart), static_cast<size_t>(clampedLength), width);
    } while (pango_layout_iter_next_line(iter));

    pango_layout_iter_free(iter);
    g_object_unref(layout);
    g_object_unref(context);
}

size_t HitTestTextCaret(HANDLE_DC hdc, const TextStyle& st, const char* text, int length, int x,
                        int* caretX) {
    if (caretX != nullptr) {
        *caretX = 0;
    }
    if (text == nullptr || length <= 0) {
        return 0;
    }

    HANDLE_FONT font = GetCachedFont(st);
    PangoContext* context = CreateRichEditMeasurePangoContext(hdc);
    PangoLayout* layout = pango_layout_new(context);
    ApplyFontDescription(layout, font);
    pango_layout_set_single_paragraph_mode(layout, TRUE);
    pango_layout_set_text(layout, text, length);

    size_t offset = 0;
    if (x > 0) {
        int index = 0;
        int trailing = 0;
        if (pango_layout_xy_to_index(layout, x * PANGO_SCALE, 0, &index, &trailing)) {
            index = std::max(0, std::min(index, length));
            offset = static_cast<size_t>(index);
            for (int i = 0; i < trailing && static_cast<int>(offset) < length; ++i) {
                const char* current = text + offset;
                const char* next = CharNext(current);
                if (next <= current || next > text + length) {
                    break;
                }
                offset = static_cast<size_t>(next - text);
            }
        } else {
            offset = static_cast<size_t>(length);
        }
    }

    PangoRectangle strongPos = {};
    pango_layout_get_cursor_pos(layout, static_cast<int>(offset), &strongPos, nullptr);
    if (caretX != nullptr) {
        *caretX = std::max(0, PANGO_PIXELS(strongPos.x));
    }

    g_object_unref(layout);
    g_object_unref(context);
    return std::min(static_cast<size_t>(length), offset);
}

void GetTextMetricsForStyle(HANDLE_DC hdc, const TextStyle& st, int& ascent, int& descent, int& lineHeight) {
    HANDLE_FONT font = GetCachedFont(st);
    PangoContext* context = CreateRichEditMeasurePangoContext(hdc);
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

