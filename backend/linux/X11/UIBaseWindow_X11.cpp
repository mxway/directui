#include <UIBaseWindow.h>
#include <poll.h>
#include "DispatchMessage.h"
#include "DisplayInstance.h"
#include "TimerContext.h"
#include "UIBaseWindowObjects.h"
#include "X11HDC.h"

class UIBaseWindowPrivate {
public:
    UIBaseWindowPrivate()
        :m_window{nullptr},
        m_parent{nullptr},
        m_duiResponseVal{DUI_RESPONSE_CLOSE}
    {
        m_window = new X11Window;
    }
    UIBaseWindowPrivate(const UIBaseWindowPrivate&)=delete;
    UIBaseWindowPrivate &operator=(const UIBaseWindowPrivate&)=delete;
    ~UIBaseWindowPrivate(){
        if(m_window->hdc != nullptr){
            ReleaseHDC(m_window->hdc);
        }
        delete m_window;
    }
    HANDLE_WND CreateWindow(HANDLE_WND parent,const UIString &className, uint32_t style, const RECT& rc);
    void SetTitle(const char *title) const ;

    X11Window *m_window;
    X11Window *m_parent;
    DuiResponseVal  m_duiResponseVal;
};

HANDLE_WND UIBaseWindowPrivate::CreateWindow(HANDLE_WND parent, const UIString &className, uint32_t style, const RECT& rc) {
    m_parent = parent;
    setlocale(LC_ALL, "");
    if(DisplayInstance::GetInstance().GetDisplay() == nullptr){
        return nullptr;
    }
    m_window->display = DisplayInstance::GetInstance().GetDisplay();
    m_window->screen = DisplayInstance::GetInstance().GetScreenNumber();
    m_window->depth = DisplayInstance::GetInstance().GetDisplayDepth();
    m_window->visual = DisplayInstance::GetInstance().GetVisual();
    m_window->colormap = DisplayInstance::GetInstance().GetColormap();

    if(m_window->window != 0){
        fprintf(stderr,"Window has already been created...\n");
        return nullptr;
    }

    m_window->window = XCreateSimpleWindow(m_window->display, RootWindow(m_window->display,m_window->screen),
                                           rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,0,BlackPixel(m_window->display, m_window->screen),
                                           WhitePixel(m_window->display, m_window->screen));
    m_window->hdc = new X11WindowHDC ;
    memset(m_window->hdc,0,sizeof(X11WindowHDC));
    m_window->hdc->x11Window = m_window;
    m_window->hdc->drawablePixmap = 0;
    m_window->hdc->gc = XCreateGC(m_window->display,m_window->window,0,nullptr);
    m_window->x = rc.left;
    m_window->y = rc.top;
    m_window->width = rc.right-rc.left;
    m_window->height = rc.bottom-rc.top;

    this->SetTitle(className.GetData());

    if(style != UI_WNDSTYLE_FRAME){
        XSetTransientForHint(m_window->display,m_window->window,parent->window);
    }
    XSelectInput(m_window->display, m_window->window, ExposureMask | KeyPressMask | KeyReleaseMask
                                                      |ButtonReleaseMask | ButtonPressMask | PointerMotionMask | StructureNotifyMask
                                                      |EnterWindowMask |LeaveWindowMask);
    XMapWindow(m_window->display,m_window->window);
    return m_window;
}

void UIBaseWindowPrivate::SetTitle(const char *title) const {
    Atom net_wm_name = XInternAtom(m_window->display, "_NET_WM_NAME", False);
    Atom utf8_string = XInternAtom(m_window->display, "UTF8_STRING", False);

    XChangeProperty(m_window->display, m_window->window, net_wm_name, utf8_string, 8,
                    PropModeReplace, reinterpret_cast<const unsigned char*>(title), strlen(title));
}

UIBaseWindow::UIBaseWindow()
        :m_data {make_shared<UIBaseWindowPrivate>()}
{

}

HANDLE_WND
UIBaseWindow::Create(HANDLE_WND parent, const UIString &className, uint32_t style, uint32_t exStyle, RECT rc) {
    m_data->CreateWindow(parent,className,style,rc);
    this->HandleMessage(DUI_WM_CREATE, (WPARAM)m_data->m_window, (LPARAM)nullptr);
    UIBaseWindowObjects::GetInstance().AddObject(m_data->m_window->window, this);
    return m_data->m_window;
}

HANDLE_WND
UIBaseWindow::Create(HANDLE_WND parent, const UIString &className, uint32_t style, uint32_t exStyle, int x, int y,
                     int cx, int cy) {
    RECT rc = {x,y,x+cx,y+cy};
    return this->Create(parent, className,style, exStyle, rc);
}

void UIBaseWindow::ShowWindow(bool bShow) const {
    if(bShow){
        XMapWindow(m_data->m_window->display,m_data->m_window->window);
    }else{
        XUnmapWindow(m_data->m_window->display,m_data->m_window->window);
    }
}

DuiResponseVal UIBaseWindow::ShowModal(){
    Display *display = DisplayInstance::GetInstance().GetDisplay();
    if (display == nullptr) {
        fprintf(stderr,"Can't Open Display");
        return DUI_RESPONSE_CLOSE;
    }
    struct pollfd fd = {
        .fd = ConnectionNumber(display),
        .events =  POLLIN
    };
    XMapWindow(display,m_data->m_window->window);
    XEvent event;

    Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, m_data->m_window->window, &wm_delete_window, 1);
    bool running = true;
    while (running) {

        bool ret = XPending(display)> 0 || poll(&fd,1,TimerContext::GetInstance().GetMinimumTimeout())>0;
        if (!ret) {
            //timeout
            TimerContext::GetInstance().ProcessTimeout();
            continue;
        }
        //有XEvent事件的时候，也有可能正好超时事件发生。
        TimerContext::GetInstance().ProcessTimeout();

        XNextEvent(display, &event);

        if(XFilterEvent(&event,None)){
            continue;
        }
        if (m_data->m_window->window!=0 && event.xany.window != m_data->m_window->window) {
            //模态对话框，仅处理当前窗口事件。
            continue;
        }
        DispatchMessage(event);
        if(event.type == DestroyNotify){
            running = false;
        }
    }
    return m_data->m_duiResponseVal;
}

// 发送 _NET_WM_STATE 消息以更改窗口状态
static void send_wm_state(Display *display, Window window, Atom state1, Atom state2, int action) {
    XClientMessageEvent event;
    event.type = ClientMessage;
    event.window = window;
    event.message_type = XInternAtom(display, "_NET_WM_STATE", False);
    event.format = 32;
    event.data.l[0] = action; // 动作：_NET_WM_STATE_ADD 或 _NET_WM_STATE_REMOVE
    event.data.l[1] = state1; // 状态1: _NET_WM_STATE_MAXIMIZED_VERT 或其他状态
    event.data.l[2] = state2; // 状态2: _NET_WM_STATE_MAXIMIZED_HORZ 或 0
    event.data.l[3] = 0;      // 时间戳 (通常为 0)
    event.data.l[4] = 0;      // 保留字段

    // 将事件发送到根窗口
    XSendEvent(display, DefaultRootWindow(display), False,
               SubstructureRedirectMask | SubstructureNotifyMask,
               reinterpret_cast<XEvent *>(&event));
    XFlush(display); // 确保事件被发送
}

bool IsWindowMaximized(Display *display, Window window) {
    Atom net_wm_state = XInternAtom(display, "_NET_WM_STATE", True);
    Atom max_vert = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
    Atom max_horz = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);

    Atom prop;
    Atom *props = NULL;
    //int num_items;
    int actual_foramt = 0;
    ulong num_items = 0;
    ulong bytes_returned = 0;

    if (XGetWindowProperty(display, window, net_wm_state, 0L, (~0L), False,
                           AnyPropertyType, &prop, &actual_foramt, &num_items,
                           &bytes_returned, (unsigned char **)&props) == Success) {
        bool maximized = false;
        for (int i = 0; i < num_items; ++i) {
            if (props[i] == max_vert || props[i] == max_horz) {
                maximized = true;
                break;
            }
        }
        if (props) {
            XFree(props);
        }
        return maximized;
    }
    return false;
}

void UIBaseWindow::Maximize() const {
    const Atom vert = XInternAtom(m_data->m_window->display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
    const Atom horz = XInternAtom(m_data->m_window->display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
    if (IsWindowMaximized(m_data->m_window->display, m_data->m_window->window)) {
        send_wm_state(m_data->m_window->display, m_data->m_window->window,vert,horz,0);
        return;
    }
    send_wm_state(m_data->m_window->display, m_data->m_window->window, vert, horz, 1); // 1 表示 _NET_WM_STATE_ADD
}

void UIBaseWindow::Restore() const {
    const Atom vert = XInternAtom(m_data->m_window->display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
    const Atom horz = XInternAtom(m_data->m_window->display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
    send_wm_state(m_data->m_window->display, m_data->m_window->window, vert, horz, 0); // 0 表示 _NET_WM_STATE_REMOVE
}

void UIBaseWindow::Minimize() const {
    XIconifyWindow(m_data->m_window->display, m_data->m_window->window,m_data->m_window->screen);
}

HANDLE_WND UIBaseWindow::GetWND() const {
    return m_data->m_window;
}

void UIBaseWindow::SetWND(HANDLE_WND wndHandle) const {
    m_data->m_window = wndHandle;
}

void UIBaseWindow::Close(DuiResponseVal val) const {
    X11Window *wnd = this->GetWND();
    Atom my_dialog_response = XInternAtom(wnd->display, "UI_WINDOW_CLOSE_RESPONSE", False);
    XEvent event;
    event.type = ClientMessage;
    event.xclient.window = wnd->window;
    event.xclient.message_type = my_dialog_response;
    event.xclient.format = 32;
    event.xclient.data.l[0] = val;
    XSendEvent(wnd->display, wnd->window, False, NoEventMask, &event);
    XFlush(wnd->display);
    m_data->m_duiResponseVal = val;
}


void UIBaseWindow::CenterWindow() const {
    if (this->m_data->m_parent != nullptr) {
        RECT parentWndRect{m_data->m_parent->x,
        m_data->m_parent->y,
        m_data->m_parent->x + m_data->m_parent->width,
        m_data->m_parent->y + m_data->m_parent->height
        };

        long xLeft = (parentWndRect.left + parentWndRect.right) / 2 - m_data->m_window->width / 2;
        long yTop = (parentWndRect.top + parentWndRect.bottom) / 2 - m_data->m_window->height / 2;
        XMoveWindow(m_data->m_window->display,m_data->m_window->window,xLeft,yTop);
    }else {
        XWindowAttributes windowAttributes{0};
        XGetWindowAttributes(m_data->m_window->display,RootWindow(m_data->m_window->display,m_data->m_window->screen),&windowAttributes);
        RECT parentWndRect{windowAttributes.x,windowAttributes.y,
            windowAttributes.x+windowAttributes.width,
        windowAttributes.y+windowAttributes.height};

        long xLeft = (parentWndRect.left + parentWndRect.right) / 2 - m_data->m_window->width / 2;
        long yTop = (parentWndRect.top + parentWndRect.bottom) / 2 - m_data->m_window->height / 2;
        XMoveWindow(m_data->m_window->display,m_data->m_window->window,xLeft,yTop);
    }
}

long UIBaseWindow::HandleMessage(uint32_t uMsg, WPARAM wParam, LPARAM lParam) {
    return 0;
}

void UIBaseWindow::OnFinalMessage(HANDLE_WND hWnd) {

}