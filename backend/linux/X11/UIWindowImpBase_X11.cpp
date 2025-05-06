#include <UIWindowImpBase.h>

// 发送 _NET_WM_MOVERESIZE 消息以启动窗口移动
void start_window_move_resize(Display *display, Window window, int x_root, int y_root, int operateCode) {
    XEvent event;
    memset(&event, 0, sizeof(event));

    XUngrabPointer(display,CurrentTime);

    event.xclient.type = ClientMessage;
    event.xclient.window = window;
    event.xclient.message_type = XInternAtom(display, "_NET_WM_MOVERESIZE", False);
    event.xclient.format = 32;
    event.xclient.data.l[0] = x_root; // 鼠标指针的根窗口 X 坐标
    event.xclient.data.l[1] = y_root; // 鼠标指针的根窗口 Y 坐标
    event.xclient.data.l[2] = operateCode;      // 动作：8 表示移动窗口
    event.xclient.data.l[3] = Button1; // 使用鼠标左键
    event.xclient.data.l[4] = 0;      // 保留字段

    // 发送事件到根窗口
    XSendEvent(display, DefaultRootWindow(display), False,
               SubstructureRedirectMask | SubstructureNotifyMask, &event);
    XFlush(display); // 刷新事件队列
}

static unsigned long LastClickTime = 0;
static const int DOUBLE_CLICK_INTERVAL = 300;

static long OnMousePress(UIBaseWindow *baseWindow,uint32_t uMsg, UIPaintManager *paintManager,WPARAM wParam, LPARAM lParam,bool &bHandled) {
    HANDLE_WND wndHandle = baseWindow->GetWND();
    XButtonEvent *buttonEvent = static_cast<XButtonEvent*>(wParam);
    RECT    rcCaption = paintManager->GetCaptionRect();
    if(buttonEvent->y < rcCaption.bottom){
        POINT pt = {(long)buttonEvent->x, (long)buttonEvent->y};
        auto* pControl = static_cast<UIControl*>(paintManager->FindControl(pt));
        if( pControl && (pControl->GetClass() != DUI_CTR_BUTTON) &&
            (pControl->GetClass() != DUI_CTR_OPTION) &&
            (pControl->GetClass() != DUI_CTR_TEXT) ) {
            if (buttonEvent->button == Button1) {
                if(buttonEvent->time - LastClickTime < DOUBLE_CLICK_INTERVAL){
                    //double click...
                    LastClickTime = 0;
                    baseWindow->Maximize();
                    return 1;
                    //this->DoDoubleClick(msg);
                    //return;
                }
                LastClickTime = buttonEvent->time;
                start_window_move_resize(wndHandle->display,wndHandle->window,buttonEvent->x_root,buttonEvent->y_root,8);
            }

        }
    }
    return 1;
}

long UIWindowImpBase::HandleMessage(uint32_t uMsg, WPARAM wParam, LPARAM lParam) {
    bool bHandled = false;
    long lRes = 0;
    switch(uMsg){
        case DUI_WM_CREATE:
            lRes = this->OnCreate(uMsg, wParam, lParam, bHandled);
            break;
        case DUI_WM_MOUSEPRESS:
            lRes = OnMousePress(this,uMsg,&m_pm,wParam,lParam,bHandled);
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
    X11Window *window = this->GetWND();
    XDestroyWindow(window->display,window->window);
    window->window = 0;
    return 0;
}

long UIWindowImpBase::OnDestroy(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) {
    printf("OnDestroy.....\n");
    return 0;
}

long UIWindowImpBase::OnSize(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) {
    return 0;
}
