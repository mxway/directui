#include "UIFont.h"

UIFont::UIFont()
    :m_underline{false},
    m_italic{false},
    m_font{nullptr},
    m_size{8}
{

}

UIFont::UIFont(const UIString &fontName, int size, bool bold, bool underline, bool italic)
        :m_strFontName {fontName},
         m_bold {bold},
         m_underline {underline},
         m_italic {italic}
{
    m_size = size;
}

void  UIFont::ReleaseFont()
{
    if(m_font){
        pango_font_description_free(m_font);
    }
}
uint32_t UIFont::GetFontHeight(HANDLE_DC hdc)
{
    UNUSED_PARAMETER(hdc);
    if(m_font){
        if(this->GetUnderline()){
            return pango_font_description_get_size(m_font)/PANGO_SCALE + 4;
        }
        return pango_font_description_get_size(m_font)/PANGO_SCALE + 2;
    }
    return 0;
}

HANDLE_FONT UIFont::Create()
{
    PangoFontDescription *desc = pango_font_description_new();
    if(desc == nullptr){
        return nullptr;
    }

    if(!m_strFontName.IsEmpty()){
        pango_font_description_set_family(desc, m_strFontName.GetData());
    }
    if(m_size){
        pango_font_description_set_absolute_size(desc, m_size * PANGO_SCALE);
        //pango_font_description_set_size(desc, m_size);
    }
    if(m_italic){
        pango_font_description_set_style(desc, PANGO_STYLE_ITALIC);
    }else{
        pango_font_description_set_style(desc, PANGO_STYLE_NORMAL);
    }
    if(m_bold){
        pango_font_description_set_weight(desc, PANGO_WEIGHT_BOLD);
    }else{
        pango_font_description_set_weight(desc, PANGO_WEIGHT_NORMAL);
    }
    m_font = desc;
    return m_font;
}
