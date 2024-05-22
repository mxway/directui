#include <UIFont.h>
#include <cstring>
#include "EncodingTransform.h"

UIFont::UIFont() {

}

UIFont::UIFont(const UIString &strFaceName, int size, bool bold, bool underline, bool italic)
    : m_strFontName {strFaceName},
      m_size{size},
      m_bold {bold},
      m_underline{underline},
      m_italic{italic}
{

}

void    UIFont::ReleaseFont()
{
    if(m_font){
        ::DeleteObject(m_font);
    }
}
uint32_t UIFont::GetFontHeight(HANDLE_DC hdc)
{
    TEXTMETRICW textmetric = {0};
    auto hOldFont = (HFONT)::SelectObject(hdc, m_font);
    ::GetTextMetricsW(hdc, &textmetric);
    ::SelectObject(hdc, hOldFont);
    return textmetric.tmHeight;
}

HANDLE_FONT  UIFont::Create(){
    //HFONT hguiFont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
    //TODO for wchar_t
    LOGFONTW lf = {0};
    ::GetObjectW(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONTW),&lf);
//    int copyBytes = m_strFontName.GetLength() >= LF_FACESIZE
//                ? (LF_FACESIZE -1)*sizeof(wchar_t): m_strFontName.GetLength() * sizeof(wchar_t);
    wchar_t *wideFontName = Utf8ToUcs2(m_strFontName.GetData(), -1);
    size_t copyBytes = wcslen(wideFontName) >= LF_FACESIZE?(LF_FACESIZE-1)*sizeof(wchar_t):(wcslen(wideFontName)+1)*sizeof(wchar_t);
    memcpy(lf.lfFaceName, wideFontName,copyBytes );
    delete []wideFontName;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfHeight = -m_size;
    if(m_bold)lf.lfWeight += FW_BOLD;
    if(m_underline)lf.lfUnderline = true;
    if(m_italic)lf.lfItalic = true;
    m_font = ::CreateFontIndirectW(&lf);
    return m_font;
}