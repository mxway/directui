#include <UIResourceMgr.h>

bool
UIResourceMgr::AddFont(int fontId, const UIString &faceName, bool defaultFont, int size, bool bold, bool underLine,
                       bool italic) {
    UIString key = UIString::ToUIString(fontId);
    if(m_fontMapping.Find(key) != nullptr){
        return false;
    }
    auto *font = new UIFont{faceName,size,bold,underLine,italic};
    font->Create();
    if(defaultFont){
        m_defaultFont = font;
    }
    return m_fontMapping.Insert(key, font);
}

UIFont *UIResourceMgr::GetFont(int fontId) {
    if(fontId<0){
        return this->GetDefaultFont();
    }
    UIString key = UIString::ToUIString(fontId);
    auto *font = static_cast<UIFont*>(m_fontMapping.Find(key));
    return font;
}

void UIResourceMgr::ReleaseAllFont() {
    for (uint32_t i = 0; i < m_fontMapping.GetSize(); i++) {
        UIString key = m_fontMapping.GetAt(i);
        auto *uiFont = static_cast<UIFont *>(m_fontMapping.Find(key));
        delete uiFont;
        //UIFont *uiFont = static_cast<UIFont*>(m_fontMapping.GetAt(0));
    }
    m_fontMapping.RemoveAll();
}

uint32_t UIResourceMgr::GetFontHeight(int fontId, HANDLE_DC hdc) {
    if(fontId < 0){
        return this->GetDefaultFontHeight(hdc);
    }
    UIFont *font = static_cast<UIFont*>(m_fontMapping.Find(UIString::ToUIString(fontId)));
    return font->GetFontHeight(hdc);
}

uint32_t UIResourceMgr::GetDefaultFontHeight(HANDLE_DC hdc) {
    return this->GetDefaultFont()->GetFontHeight(hdc);
}

UIFont *UIResourceMgr::GetFont(const UIString &fontName, int size, bool bold, bool underLine, bool italic) {
    UIFont* pFontInfo = nullptr;
    for( int i = 0; i< m_fontMapping.GetSize(); i++ ) {
        UIString key = m_fontMapping.GetAt(i);
        if(!key.IsEmpty()) {
            pFontInfo = static_cast<UIFont*>(m_fontMapping.Find(key));
            if (pFontInfo && pFontInfo->GetFontName() == fontName && pFontInfo->GetSize() == size &&
                pFontInfo->GetBold() == bold && pFontInfo->GetUnderline() == underLine && pFontInfo->GetItalic() == italic)
                return pFontInfo;
        }
    }
    return nullptr;
}
