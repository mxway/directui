#include <UIRichEdit.h>
#include "EncodingTransform.h"
#include "../../src/UIRichEditInternal_impl.h"

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

bool IsNewLine(wchar_t ch) {
    return ch == L'\n' || ch == L'\r';
}

bool IsBreakable(wchar_t c) {
    return c == L' ' || c == L'\t' || c == L'-' || c == L',' || c == L'，' || c == L'。' || c == L';' || c == L'：' || c == L':';
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

int MeasureTextWidth(HANDLE_DC hdc, const TextStyle& st, const std::wstring& text) {
    HFONT f = CreateFontFromStyle(st);
    HFONT old = (HFONT)SelectObject(hdc, f);
    SIZE sz{};
    if (!text.empty()) GetTextExtentPoint32W(hdc, text.c_str(), (int)text.size(), &sz);
    SelectObject(hdc, old);
    DeleteObject(f);
    return sz.cx;
}

void GetTextMetricsForStyle(HANDLE_DC hdc, const TextStyle& st, int& ascent, int& descent, int& lineHeight) {
    HFONT f = CreateFontFromStyle(st);
    HFONT old = (HFONT)SelectObject(hdc, f);
    TEXTMETRICW tm{};
    GetTextMetricsW(hdc, &tm);
    ascent = tm.tmAscent;
    descent = tm.tmDescent + st.lineExtra;
    lineHeight = tm.tmHeight + tm.tmExternalLeading + st.lineExtra;
    SelectObject(hdc, old);
    DeleteObject(f);
}

void DrawTextRunSegment(HANDLE_DC hdc, const TextStyle& st, const UIRect& rc, const std::wstring& text) {
    if (text.empty()) {
        return;
    }

    HFONT font = CreateFontFromStyle(st);
    HFONT old = (HFONT)SelectObject(hdc, font);
    ::SetTextColor(hdc, RGB(GetBValue(st.textColor), GetGValue(st.textColor), GetRValue(st.textColor)));
    SetBkMode(hdc, TRANSPARENT);
    TextOutW(hdc, rc.left, rc.top, text.c_str(), static_cast<int>(text.size()));
    SelectObject(hdc, old);
    DeleteObject(font);
}
