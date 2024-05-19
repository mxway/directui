#ifndef DIRECTUI_UIDLGBUILDER_H
#define DIRECTUI_UIDLGBUILDER_H
#include <IDialogBuilderCallback.h>
#include <UIString.h>
#include <UIPaintManager.h>
#include <UIControl.h>
#include <tinyxml2.h>
using namespace tinyxml2;

class UIDlgBuilder
{
public:
    enum SkinType : uint8_t
    {
        SkinType_XmlFile,
        SkinType_XmlString,
    };
public:
    UIDlgBuilder();
    UIControl   *Create(const UIString &xml, UIDlgBuilder::SkinType type= SkinType_XmlFile,
                                IDialogBuilderCallback *dlgCallback=nullptr,
                                UIPaintManager *manager = nullptr,
                                UIControl *parent = nullptr);
private:
    UIControl   *Create(IDialogBuilderCallback *callback,UIPaintManager *manager, UIControl *parent);
    UIControl*  _Parse(tinyxml2::XMLElement* parent, UIControl* pParent = nullptr, UIPaintManager* manager = nullptr);
private:
    tinyxml2::XMLDocument   m_xml;
    IDialogBuilderCallback  *m_callback;

};

#endif //DIRECTUI_UIDLGBUILDER_H