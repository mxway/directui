#ifndef UICOMBOWND_H
#define UICOMBOWND_H
#include <UIBaseWindow.h>
#include <UICombo.h>
#include <UIVerticalLayout.h>

class UIComboWnd : public UIBaseWindow
{
public:
    void            Init(UICombo *pOwner);
    //UIString        GetWindowClassName()const;
    void            OnFinalMessage(HANDLE_WND  wnd) override;

    long            HandleMessage(uint32_t uMsg, WPARAM wParam, LPARAM lParam) override ;
    void            EnsureVisible(int iIndex);
    void            Scroll(int dx, int dy);
public:
    //UIPaintManager      m_pm;
    UICombo             *m_pOwner;
    UIVerticalLayout    *m_pLayout;
    int                 m_oldSel;
    bool                m_scrollbarClicked;
private:
    long        HandleMessage_Internal(uint32_t uMsg,WPARAM wParam, LPARAM lParam);
};

#endif //UICOMBOWND_H
