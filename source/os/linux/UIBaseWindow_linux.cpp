#include <UIBaseWindow.h>
#include <gtk/gtk.h>
#include <iostream>

using namespace std;

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
    g_print("destroy event\n");
    pWindow->HandleMessage(DUI_WM_DESTROY, nullptr, nullptr);
    //glbWindowShowed = false;
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

static HANDLE_WND CreateWindow(HANDLE_WND parent, const UIString &className, uint32_t style, int x,int y,int nWidth,int nHeight,UIBaseWindow *window){
    GtkWidget *widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    if(parent!=nullptr){
        //gtk_widget_get_window()
        gtk_widget_set_parent_window(widget, gtk_widget_get_window(parent));
    }
    window->SetWND(widget);
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
    g_signal_connect(G_OBJECT(widget),"size-allocate", G_CALLBACK(wrap_size),window);
    g_signal_connect(G_OBJECT(widget), "motion-notify-event", G_CALLBACK(wrap_motion_notify), window);
    g_signal_connect(G_OBJECT(widget), "button-press-event", G_CALLBACK(wrap_button_press), window);
    g_signal_connect(G_OBJECT(widget), "button-release-event", G_CALLBACK(wrap_button_release), window);
    g_signal_connect(G_OBJECT(widget), "scroll-event", G_CALLBACK(wrap_scroll_event),window);
    g_signal_connect(G_OBJECT(widget), "enter-notify-event", G_CALLBACK(wrap_enter_notify), window);
    g_signal_connect(G_OBJECT(widget), "leave-notify-event", G_CALLBACK(wrap_leave_notify), window);
    g_signal_connect(G_OBJECT(widget), "key-press-event", G_CALLBACK(wrap_key_press), window);
    g_signal_connect(G_OBJECT(widget), "key-release-event", G_CALLBACK(wrap_key_release), window);
    g_signal_connect(G_OBJECT(widget), "delete-event", G_CALLBACK(wrap_delete_event), window);
    //g_signal_connect(G_OBJECT(widget), "focus-out-event", G_CALLBACK(wrap_focus_out_event),window);
    g_signal_connect(G_OBJECT(widget), "destroy", G_CALLBACK(wrap_destroy), window);
    g_signal_connect(G_OBJECT(widget), "draw", G_CALLBACK(wrap_draw), window);
    g_signal_connect(G_OBJECT(widget), "screen-changed", G_CALLBACK(wrap_screen_change),window);
    window->HandleMessage(DUI_WM_CREATE, (WPARAM)widget, (LPARAM)nullptr);
    //if(style == GTK_WINDOW_TOPLEVEL){
    wrap_screen_change(widget, nullptr, nullptr);
    //}
    return widget;
}

HANDLE_WND
UIBaseWindow::Create(HANDLE_WND parent, const UIString &className, uint32_t style, uint32_t exStyle, RECT rc) {
    return CreateWindow(parent, className, style,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top, this);
}

HANDLE_WND
UIBaseWindow::Create(HANDLE_WND parent, const UIString &className, uint32_t style, uint32_t exStyle, int x, int y,
                     int cx, int cy) {
    return CreateWindow(parent, className, style, x, y, cx, cy ,this);
}

void UIBaseWindow::ShowWindow(bool bShow) {
    if(bShow){
        gtk_widget_show(this->GetWND());
    }else{
        gtk_widget_hide(this->GetWND());
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
        gtk_window_set_position(GTK_WINDOW(this->GetWND()),GTK_WIN_POS_CENTER_ALWAYS);
    }
}

long UIBaseWindow::HandleMessage(uint32_t uMsg, WPARAM wParam, LPARAM lParam) {
    return 0;
}

void UIBaseWindow::OnFinalMessage(HANDLE_WND hWnd) {

}
