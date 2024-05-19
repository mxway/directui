#include <UIWindowImpBase.h>
#include <UIControl.h>

DUI_BEGIN_MESSAGE_MAP(UIWindowImpBase, UINotifyPump)
DUI_END_MESSAGE_MAP()


long UIWindowImpBase::HandleMessage(uint32_t uMsg, WPARAM wParam, LPARAM lParam) {
    long lRes = 0;
    if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
    return UIBaseWindow::HandleMessage(uMsg, wParam, lParam);
}

void UIWindowImpBase::Notify(TNotifyUI &msg) {
    UINotifyPump::NotifyPump(msg);
}
