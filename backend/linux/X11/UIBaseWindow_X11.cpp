#include <UIBaseWindow.h>
#include "DisplayInstance.h"
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
    ~UIBaseWindowPrivate(){
        if(m_window->hdc != nullptr){
            ReleaseHDC(m_window->hdc);
        }
        delete m_window;
    }
    HANDLE_WND CreateWindow(HANDLE_WND parent,const UIString &className, uint32_t style,RECT rc);
    void SetTitle(const char *title) ;

    X11Window *m_window;
    X11Window *m_parent;
    DuiResponseVal  m_duiResponseVal;
};

HANDLE_WND UIBaseWindowPrivate::CreateWindow(HANDLE_WND parent, const UIString &className, uint32_t style, RECT rc) {
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
                                                      |ButtonReleaseMask | ButtonPressMask | ButtonMotionMask | StructureNotifyMask
                                                      |EnterWindowMask |LeaveWindowMask);
    return m_window;
}

void UIBaseWindowPrivate::SetTitle(const char *title) {
    Atom net_wm_name = XInternAtom(m_window->display, "_NET_WM_NAME", False);
    Atom utf8_string = XInternAtom(m_window->display, "UTF8_STRING", False);

    XChangeProperty(m_window->display, m_window->window, net_wm_name, utf8_string, 8,
                    PropModeReplace, (unsigned char *)title, strlen(title));
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

void UIBaseWindow::ShowWindow(bool bShow) {
    if(bShow){
        XMapWindow(m_data->m_window->display,m_data->m_window->window);
    }else{
        XUnmapWindow(m_data->m_window->display,m_data->m_window->window);
    }
}

static void set_modal_hint(Display *display, Window window) {
    Atom wm_state = XInternAtom(display, "_NET_WM_STATE", False);
    Atom modal = XInternAtom(display, "_NET_WM_STATE_MODAL", False);

    XChangeProperty(display, window, wm_state, XA_ATOM, 32, PropModeReplace,
                    (unsigned char *)&modal, 1);
}

DuiResponseVal UIBaseWindow::ShowModal(){
    set_modal_hint(m_data->m_window->display,m_data->m_window->window);
    XMapWindow(m_data->m_window->display,m_data->m_window->window);
    XEvent event;

    Atom wm_delete_window = XInternAtom(m_data->m_window->display, "WM_DELETE_WINDOW", False);

    XSetWMProtocols(m_data->m_window->display, m_data->m_window->window, &wm_delete_window, 1);
    bool running = true;
    while (running) {
        XNextEvent(m_data->m_window->display, &event);

        if(XFilterEvent(&event,None)){
            continue;
        }
        if(event.type == DestroyNotify){
            this->OnFinalMessage(m_data->m_window);
            running = false;
            continue;
        }
        //this->MessageHandler(event);
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
               (XEvent *)&event);
    XFlush(display); // 确保事件被发送
}

void UIBaseWindow::Maximize() {
    Atom vert = XInternAtom(m_data->m_window->display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
    Atom horz = XInternAtom(m_data->m_window->display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
    send_wm_state(m_data->m_window->display, m_data->m_window->window, vert, horz, 1); // 1 表示 _NET_WM_STATE_ADD
}

void UIBaseWindow::Restore() {
    Atom vert = XInternAtom(m_data->m_window->display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
    Atom horz = XInternAtom(m_data->m_window->display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
    send_wm_state(m_data->m_window->display, m_data->m_window->window, vert, horz, 0); // 0 表示 _NET_WM_STATE_REMOVE
}

void UIBaseWindow::Minimize() {
    Atom hidden_state = XInternAtom(m_data->m_window->display, "_NET_WM_STATE_HIDDEN", False);
    send_wm_state(m_data->m_window->display,m_data->m_window->window,hidden_state,0,1);
}

HANDLE_WND UIBaseWindow::GetWND() {
    return m_data->m_window;
}

void UIBaseWindow::SetWND(HANDLE_WND wndHandle) {
    m_data->m_window = wndHandle;
}

void UIBaseWindow::CenterWindow() {

    /*if(!GTK_IS_WINDOW(this->GetWND())){
        return;
    }
    if(gtk_widget_get_parent_window(this->GetWND())){
        RECT currentWndRect = GetWidgetRectangle(this->GetWND());
        RECT parentWndRect = GetWidgetRectangle(gtk_widget_get_parent(this->GetWND()));
        long     dlgWidth    = currentWndRect.right - currentWndRect.left;
        long     dlgHeight   = currentWndRect.bottom - currentWndRect.top;
        long xLeft = (parentWndRect.left + parentWndRect.right) / 2 - dlgWidth / 2;
        long yTop = (parentWndRect.top + parentWndRect.bottom) / 2 - dlgHeight / 2;
        gtk_window_move(GTK_WINDOW(this->GetWND()),(gint)xLeft, (gint)yTop);
    }else{
        gtk_window_set_position(GTK_WINDOW(this->GetWND()),GTK_WIN_POS_CENTER_ALWAYS);
    }*/
}

long UIBaseWindow::HandleMessage(uint32_t uMsg, WPARAM wParam, LPARAM lParam) {
    if(uMsg == DUI_WM_CREATE){
        //this->
    }
    return 0;
}

void UIBaseWindow::OnFinalMessage(HANDLE_WND hWnd) {

}