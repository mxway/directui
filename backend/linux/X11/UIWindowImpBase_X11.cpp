#include <UIWindowImpBase.h>

long UIWindowImpBase::HandleMessage(uint32_t uMsg, WPARAM wParam, LPARAM lParam) {
    bool bHandled = false;
    long lRes = 0;
    switch(uMsg){
        case DUI_WM_CREATE:
            lRes = this->OnCreate(uMsg, wParam, lParam, bHandled);
            break;
        case DUI_WM_MOUSEPRESS:
            //lRes = OnMousePress(this->GetWND(),uMsg,&m_pm,wParam,lParam,bHandled);
            break;
        case DUI_WM_MOUSEMOVE:
            //lRes = OnMouseMove(this->GetWND(),uMsg, wParam, lParam);
            break;
        case DUI_WM_MOUSERELEASE:
            //lRes = OnMouseRelease(this->GetWND(), uMsg, wParam, lParam);
            break;
        case DUI_WM_SIZE:
            lRes = OnSize(uMsg, wParam, lParam, bHandled);
            break;
        case DUI_WM_DESTROY:
            lRes = OnDestroy(uMsg, wParam, lParam, bHandled);
            break;
        case DUI_WM_CLOSE:
            lRes = OnClose(uMsg, wParam, lParam, bHandled);
            break;
        default:
            break;
    }
    if(bHandled)return lRes;
    if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
    return UIBaseWindow::HandleMessage(uMsg, wParam, lParam);
}

long UIWindowImpBase::OnCreate(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) {
    X11Window *window = this->GetWND();
    // 移除窗口装饰
    Atom hints = XInternAtom(window->display, "_MOTIF_WM_HINTS", False);
    struct {
        unsigned long flags;
        unsigned long functions;
        unsigned long decorations;
        long input_mode;
        unsigned long status;
    } motif_hints = {2, 0, 0, 0, 0}; // decorations = 0 表示无装饰

    XChangeProperty(
            window->display, window->window,
            hints, hints, 32, PropModeReplace,
            (unsigned char *)&motif_hints, sizeof(motif_hints) / sizeof(long)
    );
    return 0;
}

long UIWindowImpBase::OnClose(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) {
    return 0;
}

long UIWindowImpBase::OnDestroy(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) {
    return 0;
}

long UIWindowImpBase::OnSize(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) {
    return 0;
}
