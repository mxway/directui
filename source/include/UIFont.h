#ifndef DIRECTUI_UIFONT_H
#define DIRECTUI_UIFONT_H
#include <UIDefine.h>
#include <UIString.h>

class UIFont
{
public:
    UIFont();
    explicit UIFont(const UIString &strFaceName, int size=0, bool bold=false,
           bool underline=false,bool italic=false);
    ~UIFont();
    void    SetItalic(bool italic);
    void    SetBold(bool bold);
    void    SetUnderline(bool underline);
    void    SetSize(int size);
    void    SetFaceName(const UIString &faceName);
    void    ReleaseFont();
    uint32_t GetFontHeight(HANDLE_DC hdc);
    HANDLE_FONT     GetHandle();
    HANDLE_FONT     Create();
    UIString        GetFontName()const{
        return m_strFontName;
    }
    int     GetSize()const{
        return m_size;
    }
    bool    GetBold()const{
        return m_bold;
    }
    bool    GetUnderline()const{
        return m_underline;
    }
    bool    GetItalic()const{
        return m_italic;
    }

private:
    UIString        m_strFontName;
    int             m_size;
    bool            m_bold;
    bool            m_underline;
    bool            m_italic;
    HANDLE_FONT     m_font;
};

#endif //DIRECTUI_UIFONT_H