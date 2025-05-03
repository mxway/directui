#include <UIDlgBuilder.h>
#include <UIResourceMgr.h>
#include <UIContainer.h>
#include <UIHorizontalLayout.h>
#include <UIVerticalLayout.h>
#include <UILabel.h>
#include <UIButton.h>
#include <UIScrollBar.h>
#include <cassert>
#include <UIText.h>
#include <UIOption.h>
#include <UICheckbox.h>
#include <UITabLayout.h>
#include <UIProgress.h>
#include <UISlider.h>
#include <UIChildLayout.h>
#include <UITileLayout.h>
#include <UIFileHelper.h>
#include <UIEdit.h>
#include <UIList.h>
#include <UICombo.h>
#include <UITreeView.h>
#include "SkinFileReaderService.h"

typedef UIControl* (*LPCREATECONTROL)(const char *pstrType);

UIDlgBuilder::UIDlgBuilder()
    : m_xml{},
    m_callback{nullptr}
{

}

UIControl *UIDlgBuilder::Create(const UIString &xml, UIDlgBuilder::SkinType type, IDialogBuilderCallback *dlgCallback,
                                UIPaintManager *manager, UIControl *parent) {
    if(type == SkinType_XmlFile){
        ByteArray  xmlString = SkinFileReaderFactory::GetSkinFileReader()->ReadFile(xml);
        m_xml.Parse((const char*)xmlString.m_buffer, xmlString.m_bufferSize);
        delete []xmlString.m_buffer;
    }else{
        m_xml.Parse(xml.GetData());
    }
    return this->Create(dlgCallback, manager, parent);
}

static void ParseImageAttribute(XMLElement *imageNode)
{
    //nAttributes = node->;
    const char *pImageName = nullptr;
    const char *pImageResType = nullptr;
    uint32_t mask = 0;
    bool shared = false;
    char *pstr = nullptr;
    const XMLAttribute *attribute = imageNode->FirstAttribute();
    while(attribute){
        const char *pstrName = attribute->Name();
        const char *pstrValue = attribute->Value();
        if( strcasecmp(pstrName, "name") == 0 ) {
            pImageName = pstrValue;
        }
        else if( strcasecmp(pstrName, "restype") == 0 ) {
            pImageResType = pstrValue;
        }
        else if( strcasecmp(pstrName, "mask") == 0 ) {
            if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
            mask = strtoul(pstrValue, &pstr, 16);
        }
        else if( strcasecmp(pstrName, "shared") == 0 ) {
            shared = (strcasecmp(pstrValue, "true") == 0);
        }
        attribute = attribute->Next();
    }
    //TODO AddImage Default
    //UIResourceMgr::GetInstance().AddImage()
    //if( pImageName ) pManager->AddImage(pImageName, pImageResType, mask, shared);
}

static void ParseFontAttribute(XMLElement *fontNode)
{
    int id = -1;
    const char *pFontName = nullptr;
    int size = 12;
    bool bold = false;
    bool underline = false;
    bool italic = false;
    bool defaultfont = false;
    bool shared = false;
    const XMLAttribute *attribute = fontNode->FirstAttribute();
    while(attribute){
        char *pstr = nullptr;
        const char *pstrName = attribute->Name();
        const char *pstrValue = attribute->Value();
        if( strcasecmp(pstrName, "id") == 0 ) {
            id = strtol(pstrValue, &pstr, 10);
        }
        else if( strcasecmp(pstrName, "name") == 0 ) {
            pFontName = pstrValue;
        }
        else if( strcasecmp(pstrName, "size") == 0 ) {
            size = strtol(pstrValue, &pstr, 10);
        }
        else if( strcasecmp(pstrName, "bold") == 0 ) {
            bold = (strcasecmp(pstrValue, "true") == 0);
        }
        else if( strcasecmp(pstrName, "underline") == 0 ) {
            underline = (strcasecmp(pstrValue, "true") == 0);
        }
        else if( strcasecmp(pstrName, "italic") == 0 ) {
            italic = (strcasecmp(pstrValue, "true") == 0);
        }
        else if( strcasecmp(pstrName, "default") == 0 ) {
            defaultfont = (strcasecmp(pstrValue, "true") == 0);
        }
        else if( strcasecmp(pstrName, "shared") == 0 ) {
            shared = (strcasecmp(pstrValue, "true") == 0);
        }
        attribute = attribute->Next();
    }
    if( id >= 0 && pFontName ) {
        UIResourceMgr::GetInstance().AddFont(id, UIString{pFontName},
                                             defaultfont,size,bold,underline,italic);
        //pManager->AddFont(id, pFontName, size, bold, underline, italic, shared);
        //if( defaultfont ) pManager->SetDefaultFont(pFontName, size, bold, underline, italic, shared);
    }
}

static void ParseDefaultAttribute(XMLElement *defaultNode, UIPaintManager *manager)
{
    const XMLAttribute *attribute = defaultNode->FirstAttribute();
    const char *pControlName = nullptr;
    const char *pControlValue = nullptr;
    bool shared = false;
    while(attribute != nullptr){
        const char *pstrName = attribute->Name();
        const char *pstrValue = attribute->Value();
        if( strcasecmp(pstrName, "name") == 0 ) {
            pControlName = pstrValue;
        }
        else if( strcasecmp(pstrName, "value") == 0 ) {
            pControlValue = pstrValue;
        }
        else if( strcasecmp(pstrName, "shared") == 0 ) {
            shared = (strcasecmp(pstrValue, "true") == 0);
        }
        attribute = attribute->Next();
    }
    if(pControlName){
        manager->AddDefaultAttributeList(pControlName,
                                                             pControlValue,shared);
    }
}

static void ParseWindowAttribute(XMLElement *root, UIPaintManager *manager)
{
    if(manager == nullptr || manager->GetPaintWindow()==nullptr){
        return;
    }
    const char *pstrClass = root->Name();
    if( strcasecmp(pstrClass, "Window") != 0 ){
        return;
    }
    const XMLAttribute *attribute = root->FirstAttribute();
    while(attribute != nullptr){
        const char *pstrName = attribute->Name();
        const char *pstrValue = attribute->Value();
        manager->SetWindowAttribute(pstrName, pstrValue);
        attribute = attribute->Next();
    }
}

UIControl *UIDlgBuilder::Create(IDialogBuilderCallback *callback, UIPaintManager *manager, UIControl *parent) {
    m_callback = callback;
    XMLElement *root = m_xml.RootElement();
    if(!root){
        return nullptr;
    }
    if( manager ) {
        const char *pstrClass = nullptr;
        for( XMLElement *node = root->FirstChildElement() ; node!=nullptr; node = node->NextSiblingElement() ) {
            pstrClass = node->Name();
            if( strcasecmp(pstrClass, "Image") == 0 ) {
                ParseImageAttribute(node);
            }
            else if( strcasecmp(pstrClass, "Font") == 0 ) {
                ParseFontAttribute(node);
            }
            else if( strcasecmp(pstrClass, "Default") == 0 ) {
                ParseDefaultAttribute(node, manager);
            }
        }
        ParseWindowAttribute(root, manager);

    }
    return _Parse(root, parent, manager);
}

UIControl *UIDlgBuilder::_Parse(tinyxml2::XMLElement *parent, UIControl *pParent, UIPaintManager *manager) {
    IContainerUI* pContainer = nullptr;
    UIControl* pReturn = nullptr;
    for( XMLElement *node = parent->FirstChildElement() ; node != nullptr ; node = node->NextSiblingElement() ) {
        const char *pstrClass = node->Name();
        if( strcasecmp(pstrClass, "Image") == 0 || strcasecmp(pstrClass, "Font") == 0 \
            || strcasecmp(pstrClass, "Default") == 0
            || strcasecmp(pstrClass, "MultiLanguage") == 0 ) continue;

        UIControl* pControl = nullptr;
        if( strcasecmp(pstrClass, "Include") == 0 ) {
            if( !node->FirstAttribute() ) continue;
            int count = 1;
            if(node->FindAttribute("count")){
                node->QueryIntAttribute("count", &count);
            }
            const char *sourceValue = nullptr;
            node->QueryStringAttribute("source",&sourceValue);
            UIDlgBuilder builder;
            pControl = builder.Create(UIString{sourceValue},SkinType_XmlFile,
                                      m_callback,manager,pParent);
            continue;
        }
        else if( strcasecmp(pstrClass, "TreeNode") == 0 ) {
            UITreeNode* pParentNode	= static_cast<UITreeNode*>(pParent->GetInterface(UIString{"TreeNode"}));
            auto* pNode			= new UITreeNode();
            if(pParentNode){
                if(!pParentNode->Add(pNode)){
                    delete pNode;
                    continue;
                }
            }

            // 若有控件默认配置先初始化默认属性
            if( manager ) {
                pNode->SetManager(manager, nullptr, false);
                const char *pDefaultAttributes = manager->GetDefaultAttributeList(pstrClass);
                if( pDefaultAttributes ) {
                    pNode->SetAttributeList(pDefaultAttributes);
                }
            }
            // 解析所有属性并覆盖默认属性
            const XMLAttribute *xmlAttribute = node->FirstAttribute();
            while(xmlAttribute){
                pNode->SetAttribute(xmlAttribute->Name(),xmlAttribute->Value());
                xmlAttribute = xmlAttribute->Next();
            }

            //检索子节点及附加控件
            if(node->FirstChildElement()!=nullptr){
                UIControl* pSubControl = _Parse(node,pNode,manager);
                if(pSubControl && strcasecmp(pSubControl->GetClass().GetData(),"TreeNodeUI") != 0)
                {
                    // 					pSubControl->SetFixedWidth(30);
                    // 					CHorizontalLayoutUI* pHorz = pNode->GetTreeNodeHoriznotal();
                    // 					pHorz->Add(new CEditUI());
                    // 					continue;
                }
            }

            if(!pParentNode){
                UITreeView* pTreeView = static_cast<UITreeView*>(pParent->GetInterface(UIString{"TreeView"}));
                assert(pTreeView);
                if( pTreeView == nullptr ) return nullptr;
                if( !pTreeView->Add(pNode) ) {
                    delete pNode;
                    continue;
                }
            }
            continue;
        }
        else {
            size_t cchLen = strlen(pstrClass);
            switch( cchLen ) {
                case 4:
                    if( strcasecmp(pstrClass, DUI_CTR_EDIT) == 0 ) {
                        pControl = new UIEdit;
                    }
                    else if( strcasecmp(pstrClass, DUI_CTR_LIST) == 0 ){
                        pControl = new UIList;
                    }
                    else if( strcasecmp(pstrClass, DUI_CTR_TEXT) == 0 ){
                        pControl = new UIText;
                    }
                    else if( strcasecmp(pstrClass, DUI_CTR_TREE) == 0 ){
                        pControl = new UITreeView;
                    }
                    else if( strcasecmp(pstrClass, DUI_CTR_HBOX) == 0 )
                    {
                        pControl = new UIHorizontalLayout;
                    }
                    else if( strcasecmp(pstrClass, DUI_CTR_VBOX) == 0 ){
                        pControl = new UIVerticalLayout;
                    }
                    break;
                case 5:
                    if( strcasecmp(pstrClass, DUI_CTR_COMBO) == 0 ){
                        pControl = new UICombo;
                    }
                    else if( strcasecmp(pstrClass, DUI_CTR_LABEL) == 0 ){
                        pControl = new UILabel;
                    }
                    //else if( _tcsicmp(pstrClass, DUI_CTR_FLASH) == 0 )           pControl = new CFlashUI;
                    break;
                case 6:
                    if( strcasecmp(pstrClass, DUI_CTR_BUTTON) == 0 ){
                        pControl = new UIButton;
                    }
                    else if( strcasecmp(pstrClass, DUI_CTR_OPTION) == 0 ){
                        pControl = new UIOption;
                    }
                    else if( strcasecmp(pstrClass, DUI_CTR_SLIDER) == 0 ){
                        pControl = new UISlider;
                    }
                    break;
                case 7:
                    if( strcasecmp(pstrClass, DUI_CTR_CONTROL) == 0 ){
                        pControl = new UIControl;
                    }
                    //else if (strcasecmp(pstrClass, DUI_CTR_GIFANIM) == 0)            pControl = new CGifAnimUI;
                    break;
                case 8:
                    if( strcasecmp(pstrClass, DUI_CTR_PROGRESS) == 0 ){
                        pControl = new UIProgress;
                    }
                    else if( strcasecmp(pstrClass, DUI_CTR_RICHEDIT) == 0 ){
                        //pControl = new CRichEditUI;
                    }
                    else if( strcasecmp(pstrClass, DUI_CTR_CHECKBOX) == 0 ){
                        pControl = new UICheckbox;
                    }
                    else if( strcasecmp(pstrClass, DUI_CTR_COMBOBOX) == 0 ){
                        pControl = new UICombo;
                    }
                    else if( strcasecmp(pstrClass, DUI_CTR_DATETIME) == 0 ){
                        //pControl = new CDateTimeUI;
                    }
                    else if( strcasecmp(pstrClass, DUI_CTR_TREEVIEW) == 0 ){
                        pControl = new UITreeView;
                    }
                    else if( strcasecmp(pstrClass, DUI_CTR_TREENODE) == 0 ){
                        pControl = new UITreeNode;
                    }
                    break;
                case 9:
                    if( strcasecmp(pstrClass, DUI_CTR_CONTAINER) == 0 ){
                        pControl = new UIContainer;
                    }
                    else if( strcasecmp(pstrClass, DUI_CTR_TABLAYOUT) == 0 ){
                        pControl = new UITabLayout;
                    }
                    else if( strcasecmp(pstrClass, DUI_CTR_SCROLLBAR) == 0 ){
                        pControl = new UIScrollBar;
                    }
                    break;
                case 10:
                    if( strcasecmp(pstrClass, DUI_CTR_LISTHEADER) == 0 ){
                        pControl = new UIListHeader;
                    }
                    else if( strcasecmp(pstrClass, DUI_CTR_TILELAYOUT) == 0 ){
                        pControl = new UITileLayout;
                    }
                    //else if( _tcsicmp(pstrClass, DUI_CTR_WEBBROWSER) == 0 )       pControl = new CWebBrowserUI;
                    break;
                case 11:
                    if (strcasecmp(pstrClass, DUI_CTR_CHILDLAYOUT) == 0){
                        pControl = new UIChildLayout;
                    }
                    break;
                case 14:
                    if( strcasecmp(pstrClass, DUI_CTR_VERTICALLAYOUT) == 0 ){
                        pControl = new UIVerticalLayout;
                    }
                    else if( strcasecmp(pstrClass, DUI_CTR_LISTHEADERITEM) == 0 ){
                        pControl = new UIListHeaderItem;
                    }
                    break;
                case 15:
                    if( strcasecmp(pstrClass, DUI_CTR_LISTTEXTELEMENT) == 0 ){
                        pControl = new UIListTextElement;
                    }
                    else if( strcasecmp(pstrClass, DUI_CTR_LISTHBOXELEMENT) == 0 ){
                        pControl = new UIListHBoxElement;
                    }
                    break;
                case 16:
                    if( strcasecmp(pstrClass, DUI_CTR_HORIZONTALLAYOUT) == 0 ){
                        pControl = new UIHorizontalLayout;
                    }
                    else if( strcasecmp(pstrClass, DUI_CTR_LISTLABELELEMENT) == 0 ){
                        pControl = new UIListLabelElement;
                    }
                    break;
                case 20:
                    if( strcasecmp(pstrClass, DUI_CTR_LISTCONTAINERELEMENT) == 0 ){
                        pControl = new UIListContainerElement;
                    }
                    break;
            }
            //TODO Use GetPlugins For Create Control;

#if 0
            // User-supplied control factory
            if( pControl == nullptr ) {
                UIPtrArray* pPlugins = UIPaintManager::GetPlugins();
                LPCREATECONTROL lpCreateControl = nullptr;
                for( int i = 0; i < pPlugins->GetSize(); ++i ) {
                    lpCreateControl = (LPCREATECONTROL)pPlugins->GetAt(i);
                    if( lpCreateControl != nullptr ) {
                        pControl = lpCreateControl(pstrClass);
                        if( pControl != nullptr ) break;
                    }
                }
            }
#endif
            if( pControl == nullptr && m_callback != nullptr ) {
                pControl = m_callback->CreateControl(pstrClass);
            }
        }

#ifndef _DEBUG
        //assert(pControl);
#endif // _DEBUG
        if( pControl == nullptr )
        {
            continue;
        }

        // Add children
        if( node->FirstChildElement() != nullptr ) {
            _Parse(node, pControl, manager);
        }
        // 因为某些属性和父窗口相关，比如selected，必须先Add到父窗口
        if( pParent != nullptr ) {
            const char *coverValue = nullptr;
            node->QueryAttribute("cover", &coverValue);
            if(coverValue != nullptr && strcasecmp(coverValue, "true")==0){
                pParent->SetCover(pControl);
            }else{
                auto* pContainerNode = static_cast<UITreeNode*>(pParent->GetInterface(UIString{DUI_CTR_TREENODE}));
                if(pContainerNode)
                    pContainerNode->GetTreeNodeHorizontal()->Add(pControl);
                else
                {
                    if( pContainer == nullptr ) pContainer = static_cast<IContainerUI*>(pParent->GetInterface(UIString{DUI_CTR_ICONTAINER}));
                    assert(pContainer);
                    if( pContainer == nullptr ) return nullptr;
                    if( !pContainer->Add(pControl) ) {
                        pControl->Delete();
                        continue;
                    }
                }

            }
        }
        // Init default attributes
        if( manager ) {
            pControl->SetManager(manager, nullptr, false);
            const char *pDefaultAttributes = manager->GetDefaultAttributeList(pstrClass);
            if( pDefaultAttributes ) {
                pControl->SetAttributeList(pDefaultAttributes);
            }
        }
        // Process attributes
        if( node->FirstAttribute() != nullptr ) {
            // Set ordinary attributes
            const XMLAttribute *attribute = node->FirstAttribute();
            while(attribute != nullptr){
                pControl->SetAttribute(attribute->Name(), attribute->Value());
                attribute = attribute->Next();
            }
        }
        if( manager ) {
            pControl->SetManager(nullptr, nullptr, false);
        }
        // Return first item
        if( pReturn == nullptr ) pReturn = pControl;
    }
    return pReturn;
}
