#ifndef DIRECTUI_UICHILDLAYOUT_H
#define DIRECTUI_UICHILDLAYOUT_H
#include <UIContainer.h>

class UIChildLayout : public UIContainer
{
public:
    UIChildLayout();

    void        Init();
    void        SetAttribute(const char *pstrName, const char *pstrValue)override;
    void        SetChildLayoutXML(const UIString &xmlFile);
    UIString    GetChildLayoutXML()const;

    UIString GetClass() const override;

    LPVOID GetInterface(const UIString &name) override;
private:
    UIString    m_xmlFile;
};

#endif //DIRECTUI_UICHILDLAYOUT_H