#ifndef DIRECTUI_GAMEFRAMEWND_H
#define DIRECTUI_GAMEFRAMEWND_H
#include "ControlEx.h"
#include "UIButton.h"
#include <UIWindowImpBase.h>

class GameFrameWnd : public UIWindowImpBase, public IListCallbackUI{
public:
    GameFrameWnd(){};
    void    OnFinalMessage(HANDLE_WND )override{delete this;}
    void    Init();
    void    OnPrepare();
    void    Notify(TNotifyUI &msg)override;
    UIString GetItemText(UIControl *pList,int iItem, int iSubItem)override;

    long OnCreate(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) override;

private:
    UIButton        *m_closeBtn;
    UIButton        *m_maxBtn;
    UIButton        *m_restoreBtn;
    UIButton        *m_minBtn;
};

#endif //DIRECTUI_GAMEFRAMEWND_H
