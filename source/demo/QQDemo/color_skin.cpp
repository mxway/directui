#include "color_skin.h"

ColorSkinWindow::ColorSkinWindow(MainFrame *main_frame, RECT recParentWindow) {

}

void ColorSkinWindow::OnFinalMessage(HANDLE_WND wnd) {
    UIBaseWindow::OnFinalMessage(wnd);
}

void ColorSkinWindow::Notify(TNotifyUI &msg) {
    UIWindowImpBase::Notify(msg);
}

void ColorSkinWindow::InitWindow() {

}

UIString ColorSkinWindow::GetSkinFile()const {
    return UIString();
}

long ColorSkinWindow::OnKillFocus(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) {
    return UIWindowImpBase::OnKillFocus(uMsg, wParam, lParam, bHandled);
}
