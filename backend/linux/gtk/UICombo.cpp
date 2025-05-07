#include <UICombo.h>
#include "../../src/UIComboWnd.h"
#include <UIRect.h>

static gboolean wrap_focus_out_event(GtkWidget *widget, GdkEventFocus  event, gpointer userdata)
{
    gtk_widget_destroy(widget);
    return true;
}

void UIComboWnd::Init(UICombo *pOwner) {
    m_pOwner = pOwner;
    m_pLayout = nullptr;
    m_oldSel = m_pOwner->GetCurSel();
    m_scrollbarClicked = false;

    // Position the popup window in absolute space
    SIZE szDrop = m_pOwner->GetDropBoxSize();
    RECT rcOwner = pOwner->GetPos();
    RECT rc = rcOwner;
    rc.top = rc.bottom;		// 父窗口left、bottom位置作为弹出窗口起点
    rc.bottom = rc.top + szDrop.cy;	// 计算弹出窗口高度
    if( szDrop.cx > 0 ) rc.right = rc.left + szDrop.cx;	// 计算弹出窗口宽度

    SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
    int cyFixed = 0;
    for( int it = 0; it < pOwner->GetCount(); it++ ) {
        auto* pControl = static_cast<UIControl*>(pOwner->GetItemAt(it));
        if( !pControl->IsVisible() ) continue;
        SIZE sz = pControl->EstimateSize(szAvailable);
        cyFixed += sz.cy;
    }
    cyFixed += 4; // CVerticalLayoutUI 默认的Inset 调整
    rc.bottom = rc.top + std::min((long)cyFixed, szDrop.cy);


    //::MapWindowRect(pOwner->GetManager()->GetPaintWindow(), HWND_DESKTOP, &rc);
    gint windowLeft = 0;
    gint windowTop = 0;
    gdk_window_get_position(gtk_widget_get_window(pOwner->GetManager()->GetPaintWindow()),&windowLeft, &windowTop);
    rc.left += windowLeft;
    rc.right += windowLeft;
    rc.top += windowTop;
    rc.bottom += windowTop;

    GdkDisplay *display = gtk_widget_get_display(pOwner->GetManager()->GetPaintWindow());
    GdkMonitor *monitor = gdk_display_get_monitor_at_window(display, gtk_widget_get_window(pOwner->GetManager()->GetPaintWindow()));
    GdkRectangle    workRectangle{0};
    gdk_monitor_get_workarea(monitor, &workRectangle);
    UIRect rcWork{workRectangle.x, workRectangle.y, workRectangle.x+workRectangle.width, workRectangle.y+workRectangle.height};
    //UIRect rcWork{workRectangle};
    if( rc.bottom > rcWork.bottom ) {
        rc.left = rcOwner.left;
        rc.right = rcOwner.right;
        if( szDrop.cx > 0 ) rc.right = rc.left + szDrop.cx;
        rc.top = rcOwner.top - MIN((long)cyFixed, szDrop.cy);
        rc.bottom = rcOwner.top;
        rc.left += windowLeft;
        rc.right += windowLeft;
        rc.top += windowTop;
        rc.bottom += windowTop;
        //::MapWindowRect(pOwner->GetManager()->GetPaintWindow(), HWND_DESKTOP, &rc);
    }

    Create(pOwner->GetManager()->GetPaintWindow(), UIString{"ComboWnd"}, GTK_WINDOW_TOPLEVEL, 0, rc);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(this->GetWND()), true);
    gtk_window_set_transient_for(GTK_WINDOW(this->GetWND()), GTK_WINDOW(pOwner->GetManager()->GetPaintWindow()));

    g_signal_connect(G_OBJECT(this->GetWND()), "focus-out-event",G_CALLBACK(wrap_focus_out_event),this);
    if(gtk_window_get_modal(GTK_WINDOW(pOwner->GetManager()->GetPaintWindow()))) {
        this->ShowModal();
    }else {
        this->ShowWindow();
    }
}

long UIComboWnd::HandleMessage_Internal(uint32_t uMsg, WPARAM wParam, LPARAM lParam) {
    if( uMsg == DUI_WM_MOUSEPRESS ) {
        //else if( uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONDBLCLK ) {
        //POINT pt = { 0 };
        auto *Event = (GdkEventButton*)wParam;

        POINT pt = {(long)Event->x, (long)Event->y};
        //m_ptLastMousePos = pt;
        //::GetCursorPos(&pt);
        //::ScreenToClient(m_pm.GetPaintWindow(), &pt);
        UIControl* pControl = m_pm.FindControl(pt);
        if( pControl && pControl->GetClass() == DUI_CTR_SCROLLBAR ) {
            m_scrollbarClicked = true;
        }
    }
    else if( uMsg == DUI_WM_MOUSERELEASE ) {
        //else if( uMsg == WM_LBUTTONUP ) {
        if (m_scrollbarClicked) {
            m_scrollbarClicked = false;
        }
        else {
            //POINT pt = { 0 };
            //::GetCursorPos(&pt);
            //::ScreenToClient(m_pm.GetPaintWindow(), &pt);
            auto *Event = (GdkEventButton*)wParam;
            POINT pt = {(long)Event->x, (long)Event->y};
            UIControl* pControl = m_pm.FindControl(pt);
            if( pControl && pControl->GetClass() != DUI_CTR_SCROLLBAR ) {
                g_signal_emit_by_name(G_OBJECT(this->GetWND()),"focus-out-event");
                //::PostMessageW(this->GetWND(),WM_KILLFOCUS,0,0);
                //PostMessage(WM_KILLFOCUS);
            }
        }
    }
    else if( uMsg == DUI_WM_KEYPRESS ) {
        auto  *gdkEventKey = (GdkEventKey*)(wParam);
        switch( (uint16_t)gdkEventKey->keyval) {
            case VK_ESCAPE:
                m_pOwner->SelectItem(m_oldSel, true);
                EnsureVisible(m_oldSel);
                break;
            case VK_RETURN:
                this->Close();
                break;
                // FALL THROUGH...
            //case VK_RETURN:
                //this->Close();
                //g_signal_emit_by_name(G_OBJECT(this->GetWND()),"delete-event");
                //::PostMessageW(this->GetWND(), WM_KILLFOCUS, 0, 0);
                //g_signal_emit_by_name(G_OBJECT(this->GetWND()),"focus-out-event");
                //PostMessage(WM_KILLFOCUS);

            default:
                TEventUI event;
                event.Type = UIEVENT_KEYDOWN;
                event.chKey = (uint16_t)gdkEventKey->keyval;
                m_pOwner->DoEvent(event);
                EnsureVisible(m_pOwner->GetCurSel());
                return 0;
        }
    }
    else if( uMsg == DUI_WM_MOUSEWHEEL ) {
        auto *EventScroll = (GdkEventScroll *)wParam;
        int zDelta = (int) (short) HIWORD(wParam);
        TEventUI event = { 0 };
        event.Type = UIEVENT_SCROLLWHEEL;
        //event.wParam = MAKELPARAM(zDelta < 0 ? SB_LINEDOWN : SB_LINEUP, 0);
        if(EventScroll->direction == GDK_SCROLL_DOWN){
            event.wParam = (WPARAM)SB_LINEDOWN;
        }else if(EventScroll->direction == GDK_SCROLL_UP){
            event.wParam = (WPARAM)SB_LINEUP;
        }else{
            return false;
        }
        event.lParam = lParam;
        event.dwTimestamp = ::UIGetTickCount();
        m_pOwner->DoEvent(event);
        EnsureVisible(m_pOwner->GetCurSel());
        return 0;
    }
    else if( uMsg == DUI_WM_KILLFOCUS ) {
        GtkWidget *focusWidget = gtk_window_get_focus(GTK_WINDOW(this->GetWND()));
        if(focusWidget == nullptr){
            // top window及子 widget都没有获得焦点。发出window关闭信号
            g_signal_emit_by_name(G_OBJECT(this->GetWND()),"delete-event");
            return 1;
        }
    }
    return 1;
}
