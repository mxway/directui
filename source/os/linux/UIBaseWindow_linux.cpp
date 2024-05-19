#include <UIBaseWindow.h>
#include <gtk/gtk.h>
#include <iostream>

using namespace std;

bool glbWindowShowed = false;

class UIBaseWindowPrivate {
public:
    UIBaseWindowPrivate(){

    }
    ~UIBaseWindowPrivate(){

    }
    GtkWidget  *widget;
};

static gboolean wrap_draw(GtkWidget *widget, cairo_t *cr, UIBaseWindow *pWindow){
    //g_print("draw event\n");
    return (gboolean)pWindow->HandleMessage(DUI_WM_PAINT, (LPARAM)cr, nullptr);
}

static void wrap_size(GtkWidget *widget, GdkRectangle *allocation, UIBaseWindow *pWindow)
{
    SIZE size = {allocation->width, allocation->height};
    pWindow->HandleMessage(DUI_WM_SIZE, (WPARAM)&size, nullptr);
}

static gboolean wrap_motion_notify(GtkWidget *widget, GdkEventMotion *event, UIBaseWindow *pWindow)
{
    return (gboolean)pWindow->HandleMessage(DUI_WM_MOUSEMOVE, (WPARAM)event, nullptr);
}

static gboolean wrap_button_press(GtkWidget *widget, GdkEventButton *event, UIBaseWindow *pWindow)
{
    if(event->type == GDK_BUTTON_PRESS){
        g_print("WM_BUTTONDOWN event %d button\n",event->button);
    }else if(event->type == GDK_BUTTON_RELEASE){
        g_print("WM_BUTTONUP event %d button\n",event->button);
    }else if(event->type == GDK_DOUBLE_BUTTON_PRESS){
        g_print("WM_BUTTONDBLCLK event %d button\n", event->button);
    }
    return (gboolean)pWindow->HandleMessage(DUI_WM_MOUSEPRESS, (WPARAM)event, nullptr);
}

static gboolean wrap_button_release(GtkWidget *widget, GdkEventButton *event, UIBaseWindow *pWindow)
{
    g_print("button_release event\n");
    return (gboolean)pWindow->HandleMessage(DUI_WM_MOUSERELEASE, (WPARAM)event, nullptr);
}

static gboolean wrap_enter_notify(GtkWidget *widget, GdkEventCrossing *event, UIBaseWindow *pWindow)
{
    return (gboolean)pWindow->HandleMessage(DUI_WM_MOUSEENTER, (WPARAM)event, nullptr);
}

static gboolean wrap_leave_notify(GtkWidget *widget, GdkEventCrossing *event, UIBaseWindow *pWindow)
{
    return (gboolean)pWindow->HandleMessage(DUI_WM_MOUSELEAVE, (WPARAM)event, nullptr);
}

static gboolean wrap_key_press(GtkWidget *widget, GdkEventKey  *event, UIBaseWindow *pWindow)
{
    return (gboolean)pWindow->HandleMessage(DUI_WM_KEYPRESS, (WPARAM)event, nullptr);
}

static gboolean wrap_key_release(GtkWidget *widget, GdkEventKey *event,UIBaseWindow *pWindow)
{
    return (gboolean)pWindow->HandleMessage(DUI_WM_KEYRELEASE, (WPARAM)event,nullptr);
}

static gboolean wrap_delete_event(GtkWidget *widget, GdkEvent *event, UIBaseWindow *pWindow)
{
    g_print("delete event\n");
    return (gboolean)pWindow->HandleMessage(DUI_WM_CLOSE, (WPARAM)event, nullptr);
}

static gboolean wrap_scroll_event(GtkWidget *widget, GdkEvent *event,UIBaseWindow *pWindow)
{
    return (gboolean)pWindow->HandleMessage(DUI_WM_MOUSEWHEEL, (WPARAM)event, nullptr);
}

static void wrap_destroy(GtkWidget *widget, UIBaseWindow *pWindow)
{
    pWindow->HandleMessage(DUI_WM_DESTROY, nullptr, nullptr);
    glbWindowShowed = false;
    pWindow->OnFinalMessage(nullptr);
}

static void wrap_screen_change(GtkWidget *widget, GdkScreen *old_screen, gpointer userdata)
{
    GdkScreen *screen = gtk_widget_get_screen(widget);
    GdkVisual *colormap = gdk_screen_get_rgba_visual(screen);

    if (!colormap)
    {
        colormap = gdk_screen_get_system_visual(screen);
    }
    else
    {
        
    }
    gtk_widget_set_visual(widget, colormap);
}

UIBaseWindow::UIBaseWindow()
    :m_data {make_shared<UIBaseWindowPrivate>()}
{

}

HANDLE_WND UIBaseWindow::Create(HANDLE_WND parent, const UIString &className, int x, int y, int nWidth, int nHeight) {
    GtkWidget *widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    if(GTK_IS_WINDOW(parent)){
        gtk_widget_set_parent(widget, parent);
    }
    this->SetWND(widget);
    gtk_widget_set_app_paintable(widget, TRUE);
    gtk_window_set_title(GTK_WINDOW(widget), className.GetData());

    gtk_window_set_default_size(GTK_WINDOW(widget),nWidth,nHeight);
    if(x != (int)0x80000000 && y!=(int)0x80000000){
        gtk_window_move(GTK_WINDOW(widget),x,y);
    }
    gtk_widget_add_events(widget,
                          GDK_EXPOSURE_MASK         |
                          GDK_SCROLL_MASK                   |
                          GDK_POINTER_MOTION_MASK           |
                          GDK_POINTER_MOTION_HINT_MASK      |
                          GDK_BUTTON_MOTION_MASK            |
                          GDK_BUTTON1_MOTION_MASK           |
                          GDK_BUTTON2_MOTION_MASK           |
                          GDK_BUTTON3_MOTION_MASK           |
                          GDK_BUTTON_PRESS_MASK             |
                          GDK_BUTTON_RELEASE_MASK           |
                          GDK_KEY_PRESS_MASK                |
                          GDK_KEY_RELEASE_MASK              |
                          GDK_ENTER_NOTIFY_MASK             |
                          GDK_LEAVE_NOTIFY_MASK             |
                          GDK_FOCUS_CHANGE_MASK);
    g_signal_connect(G_OBJECT(widget),"size-allocate", G_CALLBACK(wrap_size),this);
    g_signal_connect(G_OBJECT(widget), "motion-notify-event", G_CALLBACK(wrap_motion_notify), this);
    g_signal_connect(G_OBJECT(widget), "button-press-event", G_CALLBACK(wrap_button_press), this);
    g_signal_connect(G_OBJECT(widget), "button-release-event", G_CALLBACK(wrap_button_release), this);
    g_signal_connect(G_OBJECT(widget), "scroll-event", G_CALLBACK(wrap_scroll_event),this);
    g_signal_connect(G_OBJECT(widget), "enter-notify-event", G_CALLBACK(wrap_enter_notify), this);
    g_signal_connect(G_OBJECT(widget), "leave-notify-event", G_CALLBACK(wrap_leave_notify), this);
    g_signal_connect(G_OBJECT(widget), "key-press-event", G_CALLBACK(wrap_key_press), this);
    g_signal_connect(G_OBJECT(widget), "key-release-event", G_CALLBACK(wrap_key_release), this);
    g_signal_connect(G_OBJECT(widget), "delete-event", G_CALLBACK(wrap_delete_event), this);
    g_signal_connect(G_OBJECT(widget), "destroy", G_CALLBACK(wrap_destroy), this);
    g_signal_connect(G_OBJECT(widget), "draw", G_CALLBACK(wrap_draw), this);
    g_signal_connect(G_OBJECT(widget), "screen-changed", G_CALLBACK(wrap_screen_change),this);
    this->HandleMessage(DUI_WM_CREATE, (WPARAM)widget, (LPARAM)nullptr);
    wrap_screen_change(widget, nullptr, nullptr);
    return widget;
}

void UIBaseWindow::ShowWindow(bool bShow) {
    if(bShow){
        gtk_widget_show(this->GetWND());
        glbWindowShowed = true;
    }else{
        gtk_widget_hide(this->GetWND());
        glbWindowShowed = false;
    }
}

void UIBaseWindow::Maximize() {
    gtk_window_maximize(GTK_WINDOW(this->GetWND()));
}

void UIBaseWindow::Restore() {
    gtk_window_unmaximize(GTK_WINDOW(this->GetWND()));
}

void UIBaseWindow::Minimize() {
    gtk_window_iconify(GTK_WINDOW(this->GetWND()));
}

HANDLE_WND UIBaseWindow::GetWND() {
    return m_data->widget;
}

void UIBaseWindow::SetWND(HANDLE_WND wndHandle) {
    m_data->widget = wndHandle;
}

long OnPaint(HANDLE_WND wndHandle, uint32_t uMsg, WPARAM wParam, LPARAM lParam){
    return 1;
}

long OnKeyPress(HANDLE_WND wndHandle, uint32_t uMsg, WPARAM wParam, LPARAM)
{
    auto *eventKey = (GdkEventKey*)wParam;
    if(eventKey->keyval == GDK_KEY_Escape || eventKey->keyval == GDK_KEY_Return){
        UI_DESTROY_WINDOW(wndHandle);
    }
    return 1;
}

static bool glbLeftButtonPressed = false;
static gint glbMouseX = 0;
static gint glbMouseY = 0;

static long OnMousePress(HANDLE_WND wndHandle,uint32_t uMsg, UIPaintManager *paintManager,WPARAM wParam, LPARAM lParam,bool &bHandled)
{
    auto *event = (GdkEventButton*)(wParam);
    if(event->type != GDK_BUTTON_PRESS && event->type!=GDK_2BUTTON_PRESS){
        bHandled = false;
        return 0;
    }
    if(event->button != 1){
        bHandled = false;
        return 0;
    }

    RECT    rcCaption = paintManager->GetCaptionRect();
    if(event->y < rcCaption.bottom){
        POINT pt = {(long)event->x, (long)event->y};
        auto* pControl = static_cast<UIControl*>(paintManager->FindControl(pt));
        if( pControl && (pControl->GetClass() != DUI_CTR_BUTTON) &&
            (pControl->GetClass() != DUI_CTR_OPTION) &&
            (pControl->GetClass() != DUI_CTR_TEXT) ) {
            if (event->type == GDK_BUTTON_PRESS) {
                glbLeftButtonPressed = true;
                glbMouseX = event->x_root;
                glbMouseY = event->y_root;
            } else {
                GdkWindow *gdkWindow = gtk_widget_get_window(wndHandle);
                GdkWindowState state = gdk_window_get_state(gdkWindow);
                if (state & GDK_WINDOW_STATE_MAXIMIZED) {
                    gtk_window_unmaximize(GTK_WINDOW(wndHandle));
                } else {
                    gtk_window_maximize(GTK_WINDOW(wndHandle));
                }
            }
        }
    }
    return 1;
}

static long OnMouseMove(HANDLE_WND widget, uint32_t uMsg, WPARAM wParam, LPARAM lParam)
{
    if(!glbLeftButtonPressed){
        return 0;
    }
    auto *eventMotion = (GdkEventMotion*)wParam;
    glbLeftButtonPressed = false;
    gtk_window_begin_move_drag(GTK_WINDOW(widget),1,glbMouseX, glbMouseY,eventMotion->time);
    return 1;
}

static long OnMouseRelease(HANDLE_WND widget, uint32_t uMsg, WPARAM wParam, LPARAM lParam)
{
    glbLeftButtonPressed = false;
    return 0;
}

static RECT GetWidgetRectangle(HANDLE_WND widget)
{
    RECT rect = {0};
    GdkRectangle  rectangle {0};
    gtk_window_get_position(GTK_WINDOW(widget),&rectangle.x, &rectangle.y);
    gtk_window_get_size(GTK_WINDOW(widget),&rectangle.width, &rectangle.height);
    rect.left = rectangle.x;
    rect.top = rectangle.y;
    rect.right = rect.left + rectangle.width;
    rect.bottom = rect.top + rectangle.height;
    return rect;
    //return rectangle;
}

void UIBaseWindow::CenterWindow() {
    if(!GTK_IS_WINDOW(this->GetWND())){
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
#if 0
        gint    x = 0;
        gint    y = 0;
        gint    width = 0;
        gint    height = 0;
        gtk_window_get_position(GTK_WINDOW(this->GetWND()),&x, &y);
        gtk_window_get_size(GTK_WINDOW(this->GetWND()),&width, &height);
        //GdkScreen *screen = gtk_widget_get_screen(this->GetWND());
        GdkDisplay  *display = gtk_widget_get_display(this->GetWND());
        GdkWindow *gdkWindow = gtk_widget_get_window(this->GetWND());
        //gdk_display_get_monitor_at_window()
        GdkMonitor  *monitor = gdk_display_get_monitor_at_window(display,gdkWindow);
        GdkRectangle   monitorRect = {0};
        gdk_monitor_get_geometry(monitor, &monitorRect);
#endif
        gtk_window_set_position(GTK_WINDOW(this->GetWND()),GTK_WIN_POS_CENTER_ALWAYS);
    }
}

long UIBaseWindow::HandleMessage(uint32_t uMsg, WPARAM wParam, LPARAM lParam) {
    bool bHandled = true;
    long lRes = 0;
    switch(uMsg){
        case DUI_WM_CREATE:
            lRes = this->OnCreate(uMsg, wParam, lParam, bHandled);
            break;
        case DUI_WM_PAINT:
            return OnPaint(this->GetWND(), uMsg, wParam, lParam);
        case DUI_WM_KEYPRESS:
            return OnKeyPress(this->GetWND(), uMsg, wParam, lParam);
        case DUI_WM_MOUSEPRESS:
            lRes = OnMousePress(this->GetWND(),uMsg,&m_pm,wParam,lParam,bHandled);
            break;
        case DUI_WM_MOUSEMOVE:
            lRes = OnMouseMove(this->GetWND(),uMsg, wParam, lParam);
            break;
        case DUI_WM_MOUSERELEASE:
            lRes = OnMouseRelease(this->GetWND(), uMsg, wParam, lParam);
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
    return lRes;
}

void UIBaseWindow::OnFinalMessage(HANDLE_WND hWnd) {

}

long UIBaseWindow::OnCreate(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) {
    GtkWidget *widget = GetWND();
    gtk_window_set_decorated(GTK_WINDOW(widget), FALSE);
    return 0;
}

long UIBaseWindow::OnClose(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) {
    return 0;
}

long UIBaseWindow::OnDestroy(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) {
    return 0;
}

long UIBaseWindow::OnSize(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) {
    return 0;
}
