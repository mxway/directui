#ifndef DIRECTUI_UIHORIZONTALLAYOUT_H
#define DIRECTUI_UIHORIZONTALLAYOUT_H
#include <UIContainer.h>

class UIHorizontalLayout : public UIContainer
{
public:
    UIHorizontalLayout();

    UIString    GetClass() const override;
    LPVOID      GetInterface(const UIString &name) override;
    uint32_t    GetControlFlags() const override;

    void        SetSepWidth(int width);
    int         GetSepWidth()const;
    void        SetSepImmMode(bool immediately);
    bool        IsSepImmMode()const;
    void        SetAttribute(const char *pstrName, const char *pstrValue)override;
    void        DoEvent(TEventUI &event)override;
    void        SetPos(RECT rc, bool needInvalidate = true)override;
    void        DoPostPaint(HANDLE_DC hDC, const RECT &rcPaint)override;

    RECT        GetThumbRect(bool useNew=false)const;
protected:
    int         m_sepWidth;
    uint32_t    m_buttonState;
    POINT       m_ptLastMouse;
    RECT        m_rcNewPos;
    bool        m_immMode;
};

#endif //DIRECTUI_UIHORIZONTALLAYOUT_H