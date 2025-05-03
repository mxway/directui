#include <UIWindowImpBase.h>
#include <UIControl.h>

DUI_BEGIN_MESSAGE_MAP(UIWindowImpBase, UINotifyPump)
DUI_END_MESSAGE_MAP()

#if 0
long UIWindowImpBase::HandleMessage(uint32_t uMsg, WPARAM wParam, LPARAM lParam) {
    long lRes = 0;

    if(uMsg == DUI_WM_KILLFOCUS)
    {
        bool bHandled = true;
        lRes = OnKillFocus(uMsg, wParam, lParam, bHandled);
        if(bHandled){
            return lRes;
        }
    }
    if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
    return UIBaseWindow::HandleMessage(uMsg, wParam, lParam);
}
#endif

void UIWindowImpBase::Notify(TNotifyUI &msg) {
    UINotifyPump::NotifyPump(msg);
}

long UIWindowImpBase::OnKillFocus(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) {
    bHandled = false;
    return 0;
}
