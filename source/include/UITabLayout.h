#ifndef DIRECTUI_UITABLAYOUT_H
#define DIRECTUI_UITABLAYOUT_H
#include <UIContainer.h>

class UITabLayout : public UIContainer
{
public:
    UITabLayout();

    UIString    GetClass() const override;

    LPVOID      GetInterface(const UIString &name) override;

    bool        Add(UIControl *pControl) override;

    bool        AddAt(UIControl *pControl, int iIndex) override;

    bool        Remove(UIControl *pControl, bool bDoNotDestroy) override;

    void        RemoveAll() override;

    int         GetCurSel()const;
    bool        SelectItem(int iIndex, bool triggerEvent=true);
    bool        SelectItem(UIControl *control, bool triggerEvent=true);

    void SetPos(RECT rc, bool bNeedInvalidate) override;
    void SetAttribute(const char *pstrName, const char *pstrValue) override;
protected:
    int     m_curSel;
};

#endif //DIRECTUI_UITABLAYOUT_H