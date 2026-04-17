#include <UIRichEdit.h>
#include "EncodingTransform.h"
#include "../../src/UIRichEditInternal_impl.h"
#include <algorithm>
#include <vector>
#include <unordered_map>

namespace {
struct TextStyleKey {
    int fontSize;
    bool bold;
    bool italic;
    std::wstring fontFamily;

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
        h ^= (std::hash<std::wstring>{}(key.fontFamily) << 3);
        return h;
    }
};

HFONT GetCachedFont(const TextStyle& style) {
    static std::unordered_map<TextStyleKey, HFONT, TextStyleKeyHash> s_fontCache;
    TextStyleKey key{style.fontSize, style.bold, style.italic, style.fontFamily};
    auto it = s_fontCache.find(key);
    if (it != s_fontCache.end()) {
        return it->second;
    }
    HFONT font = CreateFontFromStyle(style);
    s_fontCache.emplace(std::move(key), font);
    return font;
}
}

void TextRun::SetFontFamily(const UIString& fontFamily) {
    wchar_t *ucs2String = Utf8ToUcs2(fontFamily.GetData(), fontFamily.GetLength());
    style.fontFamily = ucs2String;
    delete []ucs2String;
}

void TextRun::SetText(const UIString& utf8Text) {
    wchar_t *ucs2String = Utf8ToUcs2(utf8Text.GetData(), utf8Text.GetLength());
    text = ucs2String;
    delete []ucs2String;
}

bool IsBreakable(wchar_t c) {
    wchar_t forbiddenChars[] = {L'，', L'。', L'、', L'！', L'？', L'：', L'；', L'（', L'）',L'【',L'】',L'“',L'”',
        L'《',L'》',L'「',L'」',L'『',L'』',L'"', L',', L'.', L'!', L'?', L':', L';', L'(', L')', L'[', L']'};
    for (int i=0;i<sizeof(forbiddenChars)/sizeof(wchar_t);i++) {
        if (c == forbiddenChars[i]) {
            return false;
        }
    }
    return true;
}

size_t ConsumeNewLine(const std::wstring& text, size_t index) {
    if (index >= text.size()) {
        return 0;
    }

    if (text[index] == L'\r') {
        return (index + 1 < text.size() && text[index + 1] == L'\n') ? 2 : 1;
    }

    return text[index] == L'\n' ? 1 : 0;
}

size_t NextTextUnit(const std::wstring& text, size_t index) {
    return index < text.size() ? index + 1 : text.size();
}

bool IsBreakableAt(const std::wstring& text, size_t index) {
    return index < text.size() && IsBreakable(text[index]);
}

HANDLE_FONT CreateFontFromStyle(const TextStyle& s) {
    return CreateFontW(
        -s.fontSize, 0, 0, 0,
        s.bold ? FW_BOLD : FW_NORMAL,
        s.italic ? TRUE : FALSE,
        FALSE, FALSE,
        DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        s.fontFamily.c_str()
    );
}

int MeasureTextWidthRange(HANDLE_DC hdc, const TextStyle& st, const wchar_t* text, int length) {
    if (text == nullptr || length <= 0) {
        return 0;
    }
    HFONT f = GetCachedFont(st);
    HFONT old = (HFONT)SelectObject(hdc, f);
    SIZE sz{};
    GetTextExtentPoint32W(hdc, text, length, &sz);
    SelectObject(hdc, old);
    return sz.cx;
}

int GetTextFitMetrics(HANDLE_DC hdc, const TextStyle& st, const wchar_t* text, int length, int maxWidth,
                      int* fitWidth) {
    if (text == nullptr || length <= 0 || maxWidth <= 0) {
        if (fitWidth) {
            *fitWidth = 0;
        }
        return 0;
    }
    HFONT f = GetCachedFont(st);
    HFONT old = (HFONT)SelectObject(hdc, f);
    int fitCount = 0;
    SIZE sz{};
    if (!GetTextExtentExPointW(hdc, text, length, maxWidth, &fitCount, nullptr, &sz)) {
        fitCount = 0;
        if (fitWidth) {
            *fitWidth = 0;
        }
    } else if (fitWidth) {
        *fitWidth = (fitCount > 0) ? sz.cx : 0;
    }
    SelectObject(hdc, old);
    return fitCount;
}

size_t HitTestTextCaret(HANDLE_DC hdc, const TextStyle& st, const wchar_t* text, int length, int x,
                       int* caretX) {
    if (caretX) {
        *caretX = 0;
    }
    if (text == nullptr || length <= 0) {
        return 0;
    }

    HFONT f = GetCachedFont(st);
    HFONT old = (HFONT)SelectObject(hdc, f);
    std::vector<int> extents(static_cast<size_t>(length), 0);
    SIZE sz{};
    const BOOL ok = GetTextExtentExPointW(hdc, text, length, 0x7fffffff, nullptr, extents.data(), &sz);
    SelectObject(hdc, old);
    if (!ok || extents.empty()) {
        return 0;
    }

    if (x <= 0) {
        return 0;
    }

    const int totalWidth = extents.back();
    if (x >= totalWidth) {
        if (caretX) {
            *caretX = totalWidth;
        }
        return static_cast<size_t>(length);
    }

    for (int i = 0; i < length; ++i) {
        const int current = extents[static_cast<size_t>(i)];
        if (x > current) {
            continue;
        }

        const int previous = (i > 0) ? extents[static_cast<size_t>(i - 1)] : 0;
        if ((x - previous) < (current - x)) {
            if (caretX) {
                *caretX = previous;
            }
            return static_cast<size_t>(i);
        }

        if (caretX) {
            *caretX = current;
        }
        return static_cast<size_t>(i + 1);
    }

    if (caretX) {
        *caretX = totalWidth;
    }
    return static_cast<size_t>(length);
}

void GetTextMetricsForStyle(HANDLE_DC hdc, const TextStyle& st, int& ascent, int& descent, int& lineHeight) {
    HFONT f = GetCachedFont(st);
    HFONT old = (HFONT)SelectObject(hdc, f);
    TEXTMETRICW tm{};
    GetTextMetricsW(hdc, &tm);
    ascent = tm.tmAscent;
    descent = tm.tmDescent + st.lineExtra;
    lineHeight = tm.tmHeight + tm.tmExternalLeading + st.lineExtra;
    SelectObject(hdc, old);
}

void DrawTextRunSegment(HANDLE_DC hdc, const TextStyle& st, const UIRect& rc, const std::wstring& text) {
    if (text.empty()) {
        return;
    }

    HFONT font = GetCachedFont(st);
    HFONT old = (HFONT)SelectObject(hdc, font);
    ::SetTextColor(hdc, RGB(GetBValue(st.textColor), GetGValue(st.textColor), GetRValue(st.textColor)));
    SetBkMode(hdc, TRANSPARENT);
    TextOutW(hdc, rc.left, rc.top, text.c_str(), static_cast<int>(text.size()));
    SelectObject(hdc, old);
}

void ComputeTextSliceForLine(HANDLE_DC hdc,
                                    const TextRun& textRun,
                                    const std::wstring& s,
                                    size_t i,
                                    int avail,
                                    size_t& cut,
                                    int& bestW,
                                    bool& forceCommitAfterSoftBreak) {
    cut = i;
    bestW = 0;
    forceCommitAfterSoftBreak = false;
    const size_t n = s.size();
    size_t runEnd = i;
    while (runEnd < n && ConsumeNewLine(s, runEnd) == 0) {
        const size_t next = NextTextUnit(s, runEnd);
        if (next <= runEnd) {
            break;
        }
        runEnd = next;
    }

    const int maxLen = static_cast<int>(runEnd - i);
    int fitWidth = 0;
    const int fitCount = GetTextFitMetrics(
        hdc,
        textRun.style,
        s.c_str() + i,
        maxLen,
        avail,
        &fitWidth);

    if (fitCount <= 0) {
        cut = NextTextUnit(s, i);
        bestW = MeasureTextWidthRange(hdc, textRun.style, s.c_str() + i, static_cast<int>(cut - i));
        return;
    }

    cut = i + static_cast<size_t>(fitCount);
    bestW = fitWidth;

    if (cut < runEnd) {
        if (!IsBreakableAt(s, cut)) {
            cut--;
        }
        bestW = avail;
        forceCommitAfterSoftBreak = true;
    }
}