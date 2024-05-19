#include <UIFont.h>

UIFont::~UIFont() {
    ReleaseFont();
}

void UIFont::SetItalic(bool italic) {
    m_italic = italic;
}

void UIFont::SetBold(bool bold) {
    m_bold = bold;
}

void UIFont::SetUnderline(bool underline) {
    m_underline = underline;
}

void UIFont::SetSize(int size) {
    m_size = size;
}

void UIFont::SetFaceName(const UIString &faceName) {
    m_strFontName = faceName;
}

HANDLE_FONT UIFont::GetHandle() {
    return m_font;
}
