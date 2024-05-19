#include <UIChildLayout.h>
#include <UIDlgBuilder.h>

UIChildLayout::UIChildLayout() = default;

void UIChildLayout::Init() {
    if(m_xmlFile.IsEmpty()){
        return;
    }
    UIDlgBuilder    dlgBuilder;
    UIControl *control = dlgBuilder.Create(m_xmlFile,
                                  UIDlgBuilder::SkinType_XmlFile,
                                  nullptr,m_manager);
    auto *childWindow = dynamic_cast<UIContainer*>(control);
    if(childWindow){
        this->Add(childWindow);
    }else{
        this->RemoveAll();
    }
}

void UIChildLayout::SetAttribute(const char *pstrName, const char *pstrValue) {
    if( strcasecmp(pstrName, "xmlfile") == 0 )
        SetChildLayoutXML(UIString{pstrValue});
    else
        UIContainer::SetAttribute(pstrName,pstrValue);
}

void UIChildLayout::SetChildLayoutXML(const UIString &xmlFile) {
    m_xmlFile = xmlFile;
}

UIString UIChildLayout::GetChildLayoutXML() const {
    return m_xmlFile;
}

UIString UIChildLayout::GetClass() const {
    return UIString{DUI_CTR_CHILDLAYOUT};
}

LPVOID UIChildLayout::GetInterface(const UIString &name) {
    if(name == DUI_CTR_CHILDLAYOUT){
        return static_cast<UIChildLayout*>(this);
    }
    return UIContainer::GetInterface(name);
}
