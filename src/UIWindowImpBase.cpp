#include <UIWindowImpBase.h>
#include <UIControl.h>

DUI_BEGIN_MESSAGE_MAP(UIWindowImpBase, UINotifyPump)
DUI_END_MESSAGE_MAP()

void UIWindowImpBase::Notify(TNotifyUI &msg) {
    UINotifyPump::NotifyPump(msg);
}

long UIWindowImpBase::OnKillFocus(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) {
    bHandled = false;
    return 0;
}

long UIWindowImpBase::OnKeyPress(uint32_t uMsg, uint32_t keyCode, bool& bHandled) {
    return 0;
}
