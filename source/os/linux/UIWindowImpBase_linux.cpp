#include <UIWindowImpBase.h>

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

long UIWindowImpBase::HandleMessage(uint32_t uMsg, WPARAM wParam, LPARAM lParam) {
    bool bHandled = false;
    long lRes = 0;
    switch(uMsg){
        case DUI_WM_CREATE:
            lRes = this->OnCreate(uMsg, wParam, lParam, bHandled);
            break;
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
    if(bHandled)return lRes;
    if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
    return UIBaseWindow::HandleMessage(uMsg, wParam, lParam);
    //return lRes;
}

long UIWindowImpBase::OnCreate(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) {
    GtkWidget *widget = GetWND();
    gtk_window_set_decorated(GTK_WINDOW(widget), FALSE);
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
