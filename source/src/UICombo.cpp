#include <UICombo.h>
#include <UIRenderClip.h>
#include <UIScrollBar.h>
#include <UIRenderEngine.h>
#include <UIBaseWindow.h>
#include <cassert>
#include "UIResourceMgr.h"

#ifdef WIN32
#include <windowsx.h>
#include "UIResourceMgr.h"

#endif

class UIComboBody : public UIVerticalLayout
{
public:
    explicit UIComboBody(UICombo *pOwner);
    bool        DoPaint(HANDLE_DC hDC, const RECT &rcPaint,UIControl *stopControl) override;
protected:
    UICombo     *m_pOwner;
};

UIComboBody::UIComboBody(UICombo *pOwner)
    :m_pOwner{pOwner}
{

}

bool UIComboBody::DoPaint(HANDLE_DC hDC, const RECT &rcPaint, UIControl *stopControl) {
    RECT rcTemp = { 0 };
    if( !::UIIntersectRect(&rcTemp, &rcPaint, &m_rcItem) ) return true;

    TListInfoUI* pListInfo = nullptr;
    if( m_pOwner ) pListInfo = m_pOwner->GetListInfo();

    UIRenderClip clip;
    UIRenderClip::GenerateClip(hDC, rcTemp, clip);
    UIControl::DoPaint(hDC, rcPaint, stopControl);

    if( m_items.GetSize() > 0 ) {
        RECT rc = m_rcItem;
        rc.left += m_rcInset.left;
        rc.top += m_rcInset.top;
        rc.right -= m_rcInset.right;
        rc.bottom -= m_rcInset.bottom;
        if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
        if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

        if( !::UIIntersectRect(&rcTemp, &rcPaint, &rc) ) {
            for( int it = 0; it < m_items.GetSize(); it++ ) {
                auto* pControl = static_cast<UIControl*>(m_items[it]);
                if( pControl == stopControl ) return false;
                if( !pControl->IsVisible() ) continue;
                if( !::UIIntersectRect(&rcTemp, &rcPaint, &pControl->GetPos()) ) continue;
                if( pControl->IsFloat() ) {
                    if( !::UIIntersectRect(&rcTemp, &m_rcItem, &pControl->GetPos()) ) continue;
                    if( !pControl->Paint(hDC, rcPaint, stopControl) ) return false;
                }
            }
        }
        else {
            int iDrawIndex = 0;
            UIRenderClip childClip;
            UIRenderClip::GenerateClip(hDC, rcTemp, childClip);
            for( int it = 0; it < m_items.GetSize(); it++ ) {
                auto* pControl = static_cast<UIControl*>(m_items[it]);
                if( pControl == stopControl ) return false;
                if( !pControl->IsVisible() ) continue;
                if( !pControl->IsFloat() ) {
                    auto* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(UIString{DUI_CTR_ILISTITEM}));
                    if( pListItem != nullptr ) {
                        pListItem->SetDrawIndex(iDrawIndex);
                        iDrawIndex += 1;
                    }
                    if (pListInfo && pListInfo->iHLineSize > 0) {
                        // 因为没有为最后一个预留分割条长度，如果list铺满，最后一条不会显示
                        RECT rcPadding = pControl->GetPadding();
                        const RECT& rcPos = pControl->GetPos();
                        RECT rcBottomLine = { rcPos.left, rcPos.bottom + rcPadding.bottom, rcPos.right, rcPos.bottom + rcPadding.bottom + pListInfo->iHLineSize };
                        if( ::UIIntersectRect(&rcTemp, &rcPaint, &rcBottomLine) ) {
                            rcBottomLine.top += pListInfo->iHLineSize / 2;
                            rcBottomLine.bottom = rcBottomLine.top;
                            UIRenderEngine::DrawLine(hDC, rcBottomLine, pListInfo->iHLineSize, GetAdjustColor(pListInfo->dwHLineColor),0);
                        }
                    }
                }
                if( !::UIIntersectRect(&rcTemp, &rcPaint, &pControl->GetPos()) ) continue;
                if( pControl->IsFloat() ) {
                    if( !::UIIntersectRect(&rcTemp, &m_rcItem, &pControl->GetPos()) ) continue;
                    UIRenderClip::UseOldClipBegin(hDC, childClip);
                    if( !pControl->Paint(hDC, rcPaint, stopControl) ) {
                        UIRenderClip::UseOldClipEnd(hDC, childClip);
                        return false;
                    }
                    UIRenderClip::UseOldClipEnd(hDC, childClip);
                }
                else {
                    if( !::UIIntersectRect(&rcTemp, &rc, &pControl->GetPos()) ) continue;
                    if( !pControl->Paint(hDC, rcPaint, stopControl) ) return false;
                }
            }
        }
    }

    if( m_pVerticalScrollBar != nullptr ) {
        if( m_pVerticalScrollBar == stopControl ) return false;
        if (m_pVerticalScrollBar->IsVisible()) {
            if( ::UIIntersectRect(&rcTemp, &rcPaint, &m_pVerticalScrollBar->GetPos()) ) {
                if( !m_pVerticalScrollBar->Paint(hDC, rcPaint, stopControl) ) return false;
            }
        }
    }

    if( m_pHorizontalScrollBar != nullptr ) {
        if( m_pHorizontalScrollBar == stopControl ) return false;
        if (m_pHorizontalScrollBar->IsVisible()) {
            if( ::UIIntersectRect(&rcTemp, &rcPaint, &m_pHorizontalScrollBar->GetPos()) ) {
                if( !m_pHorizontalScrollBar->Paint(hDC, rcPaint, stopControl) ) return false;
            }
        }
    }
    return true;
}

class UIComboWnd : public UIBaseWindow
{
public:
    void            Init(UICombo *pOwner);
    //UIString        GetWindowClassName()const;
    void            OnFinalMessage(HANDLE_WND  wnd) override;

    long            HandleMessage(uint32_t uMsg, WPARAM wParam, LPARAM lParam) override ;
    void            EnsureVisible(int iIndex);
    void            Scroll(int dx, int dy);
public:
    //UIPaintManager      m_pm;
    UICombo             *m_pOwner;
    UIVerticalLayout    *m_pLayout;
    int                 m_oldSel;
    bool                m_scrollbarClicked;
};

#ifdef WIN32
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
    rc.bottom = rc.top + min((LONG)cyFixed, szDrop.cy);

    ::MapWindowRect(pOwner->GetManager()->GetPaintWindow(), HWND_DESKTOP, &rc);

    MONITORINFO oMonitor = {};
    oMonitor.cbSize = sizeof(oMonitor);
    ::GetMonitorInfo(::MonitorFromWindow(pOwner->GetManager()->GetPaintWindow(), MONITOR_DEFAULTTOPRIMARY), &oMonitor);
    UIRect rcWork{oMonitor.rcWork};
    if( rc.bottom > rcWork.bottom ) {
        rc.left = rcOwner.left;
        rc.right = rcOwner.right;
        if( szDrop.cx > 0 ) rc.right = rc.left + szDrop.cx;
        rc.top = rcOwner.top - MIN((LONG)cyFixed, szDrop.cy);
        rc.bottom = rcOwner.top;
        ::MapWindowRect(pOwner->GetManager()->GetPaintWindow(), HWND_DESKTOP, &rc);
    }
    Create(pOwner->GetManager()->GetPaintWindow(), UIString{"ComboWnd"}, WS_POPUP, WS_EX_TOOLWINDOW, rc);
    // HACK: Don't deselect the parent's caption
    HWND hWndParent = this->GetWND();
    while( ::GetParent(hWndParent) != nullptr ) hWndParent = ::GetParent(hWndParent);
    ::ShowWindow(this->GetWND(), SW_SHOW);

}
#else

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
#endif

void UIComboWnd::OnFinalMessage(HANDLE_WND wnd) {
    m_pOwner->m_pWindow = nullptr;
    m_pOwner->m_buttonState &= ~ UISTATE_PUSHED;
    m_pOwner->Invalidate();
    delete this;
}

long UIComboWnd::HandleMessage(uint32_t uMsg, WPARAM wParam, LPARAM lParam) {
    if( uMsg == DUI_WM_CREATE ) {
#ifdef __linux__
        gtk_window_set_decorated(GTK_WINDOW(this->GetWND()),false);
#endif
        m_pm.Init(this->GetWND());
        // The trick is to add the items to the new container. Their owner gets
        // reassigned by this operation - which is why it is important to reassign
        // the items back to the righfull owner/manager when the window closes.
        m_pLayout = new UIComboBody(m_pOwner);
        m_pLayout->SetManager(&m_pm, NULL, true);
        const char *pDefaultAttributes = m_pOwner->GetManager()->GetDefaultAttributeList("VerticalLayout");
        if( pDefaultAttributes ) {
            m_pLayout->SetAttributeList(pDefaultAttributes);
        }
        m_pLayout->SetInset(UIRect{1, 1, 1, 1});
        m_pLayout->SetBkColor(0xFFFFFFFF);
        m_pLayout->SetBorderColor(0xFFC6C7D2);
        m_pLayout->SetBorderSize(1);
        m_pLayout->SetAutoDestroy(false);
        m_pLayout->EnableScrollBar();
        m_pLayout->SetAttributeList(m_pOwner->GetDropBoxAttributeList().GetData());
        for( int i = 0; i < m_pOwner->GetCount(); i++ ) {
            m_pLayout->Add(static_cast<UIControl*>(m_pOwner->GetItemAt(i)));
        }
        m_pm.AttachDialog(m_pLayout);

        return 0;
    }
    else if( uMsg == DUI_WM_CLOSE ) {
        m_pOwner->SetManager(m_pOwner->GetManager(), m_pOwner->GetParent(), false);
        if( !m_pOwner->IsFloat() ) m_pOwner->SetPos(m_pOwner->GetPos(), false);
        else m_pOwner->SetPos(m_pOwner->GetRelativePos(), false);
        m_pOwner->SetFocus();
    }
#ifdef WIN32
    else if( uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONDBLCLK ) {
        POINT pt = { 0 };
        ::GetCursorPos(&pt);
        ::ScreenToClient(m_pm.GetPaintWindow(), &pt);
        UIControl* pControl = m_pm.FindControl(pt);
        if( pControl && pControl->GetClass() == DUI_CTR_SCROLLBAR) {
            m_scrollbarClicked = true;
        }
    }
    else if( uMsg == WM_LBUTTONUP ) {
        if (m_scrollbarClicked) {
            m_scrollbarClicked = false;
        }
        else {
            POINT pt = { 0 };
            ::GetCursorPos(&pt);
            ::ScreenToClient(m_pm.GetPaintWindow(), &pt);
            UIControl* pControl = m_pm.FindControl(pt);
            if( pControl && pControl->GetClass() != DUI_CTR_SCROLLBAR) PostMessageW(this->GetWND(),WM_KILLFOCUS,0,0);
        }
    }
    else if( uMsg == WM_KEYDOWN ) {
        switch( wParam ) {
            case VK_ESCAPE:
                m_pOwner->SelectItem(m_oldSel, true);
                EnsureVisible(m_oldSel);
                // FALL THROUGH...
            case VK_RETURN:
                PostMessageW(this->GetWND(),WM_KILLFOCUS,0,0);
                break;
            default:
                TEventUI event;
                event.Type = UIEVENT_KEYDOWN;
                event.chKey = (uint16_t )wParam;
                m_pOwner->DoEvent(event);
                EnsureVisible(m_pOwner->GetCurSel());
                return 0;
        }
    }
    else if( uMsg == WM_MOUSEWHEEL ) {
        int zDelta = (int) (short) HIWORD(wParam);
        TEventUI event = { 0 };
        event.Type = UIEVENT_SCROLLWHEEL;
        event.wParam = MAKELPARAM(zDelta < 0 ? SB_LINEDOWN : SB_LINEUP, 0);
        event.lParam = lParam;
        event.dwTimestamp = ::GetTickCount();
        m_pOwner->DoEvent(event);
        EnsureVisible(m_pOwner->GetCurSel());
        return 0;
    }
    else if( uMsg == WM_KILLFOCUS ) {
        if( this->GetWND() != (HWND) wParam ) {
            HWND hWnd = ::GetFocus();
            HWND hParentWnd = NULL;
            bool bIsChildFocus = false;
            while( (hParentWnd = ::GetParent(hWnd)) && hParentWnd!=nullptr ) {
                if( this->GetWND() == hParentWnd ) {
                    bIsChildFocus = true;
                    break;
                }
                hWnd = hParentWnd;
            }
            if (!bIsChildFocus) {
                PostMessageW(this->GetWND(),WM_CLOSE,0,0);
                return 0;
            }
        }
    }
#else
    else if( uMsg == DUI_WM_MOUSEPRESS ) {
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
#endif

    long lRes = 0;
    if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
    return UIBaseWindow::HandleMessage(uMsg, wParam, lParam);
}

void UIComboWnd::EnsureVisible(int iIndex) {
    if( m_pOwner->GetCurSel() < 0 ) return;
    m_pLayout->FindSelectable(m_pOwner->GetCurSel(), false);
    RECT rcItem = m_pLayout->GetItemAt(iIndex)->GetPos();
    RECT rcList = m_pLayout->GetPos();
    UIScrollBar* pHorizontalScrollBar = m_pLayout->GetHorizontalScrollBar();
    if( pHorizontalScrollBar && pHorizontalScrollBar->IsVisible() ) rcList.bottom -= pHorizontalScrollBar->GetFixedHeight();
    int iPos = m_pLayout->GetScrollPos().cy;
    if( rcItem.top >= rcList.top && rcItem.bottom < rcList.bottom ) return;
    int dx = 0;
    if( rcItem.top < rcList.top ) dx = rcItem.top - rcList.top;
    if( rcItem.bottom > rcList.bottom ) dx = rcItem.bottom - rcList.bottom;
    Scroll(0, dx);
}

void UIComboWnd::Scroll(int dx, int dy) {
    if( dx == 0 && dy == 0 ) return;
    SIZE sz = m_pLayout->GetScrollPos();
    m_pLayout->SetScrollPos(SIZE{sz.cx + dx, sz.cy + dy});
}

UICombo::UICombo()
    :m_pWindow{nullptr},
    m_curSel{-1},
    m_buttonState{0}
{
    m_dropBox = SIZE{0,150};
    memset(&m_rcTextPadding,0,sizeof(m_rcTextPadding));
    m_listInfo.nColumns = 0;
    m_listInfo.uFixedHeight = 0;
    m_listInfo.nFont = -1;
    m_listInfo.textStyle = DT_VCENTER|DT_SINGLELINE;
    m_listInfo.dwTextColor = 0xFF000000;
    m_listInfo.dwBkColor = 0;
    m_listInfo.bAlternateBk = false;
    m_listInfo.dwSelectedTextColor = 0xFF000000;
    m_listInfo.dwSelectedBkColor = 0xFFC1E3FF;
    m_listInfo.dwHotTextColor = 0xFF000000;
    m_listInfo.dwHotBkColor = 0xFFE9F5FF;
    m_listInfo.dwDisabledTextColor = 0xFFCCCCCC;
    m_listInfo.dwDisabledBkColor = 0xFFFFFFFF;
    m_listInfo.iHLineSize = 0;
    m_listInfo.dwHLineColor = 0xFF3C3C3C;
    m_listInfo.iVLineSize = 0;
    m_listInfo.dwVLineColor = 0xFF3C3C3C;
    m_listInfo.bShowHtml = false;
    m_listInfo.bMultiExpandable = false;

    m_showText = true;
    m_selectCloseFlag = true;
    memset(&m_listInfo.rcTextPadding, 0, sizeof(m_listInfo.rcTextPadding));
    memset(&m_listInfo.rcColumn, 0, sizeof(m_listInfo.rcColumn));
}

UIString UICombo::GetClass() const {
    return UIString{DUI_CTR_COMBO};
}

LPVOID UICombo::GetInterface(const UIString &name) {
    if(name == DUI_CTR_ILISTOWNER){
        return static_cast<IListOwnerUI*>(this);
    }
    if(name == DUI_CTR_COMBO){
        return static_cast<UICombo*>(this);
    }
    return UIContainer::GetInterface(name);
}

void UICombo::DoInit() {
    //UIControl::DoInit();
}

uint32_t UICombo::GetControlFlags() const {
    return UIFLAG_TABSTOP;
}

UIString UICombo::GetText() const {
    if( m_curSel < 0 ) return UIString{""};
    auto* pControl = static_cast<UIControl*>(m_items[m_curSel]);
    return pControl->GetText();
}

void UICombo::SetEnabled(bool bEnabled) {
    UIContainer::SetEnabled(bEnabled);
    if( !IsEnabled() ) m_buttonState = 0;
}

UIString UICombo::GetDropBoxAttributeList() {
    return m_dropBoxAttributes;
}

void UICombo::SetDropBoxAttributeList(const char *pstrList) {
    m_dropBoxAttributes = pstrList;
}

SIZE UICombo::GetDropBoxSize() const {
    return m_dropBox;
}

void UICombo::SetDropBoxSize(SIZE szDropBox) {
    m_dropBox = szDropBox;
}

int UICombo::GetCurSel() const {
    return m_curSel;
}

bool UICombo::GetSelectCloseFlag() {
    return m_selectCloseFlag;
}

void UICombo::SetSelectCloseFlag(bool flag) {
    m_selectCloseFlag = flag;
}

bool UICombo::SelectItem(int iIndex, bool bTakeFocus, bool bTriggerEvent) {
    if( m_selectCloseFlag && m_pWindow != nullptr ) m_pWindow->Close();
    if( iIndex == m_curSel ) return true;
    int iOldSel = m_curSel;
    if( m_curSel >= 0 ) {
        auto* pControl = static_cast<UIControl*>(m_items[m_curSel]);
        if( !pControl ) return false;
        IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(UIString{DUI_CTR_ILISTITEM}));
        if( pListItem != nullptr ) pListItem->Select(false, bTriggerEvent);
        m_curSel = -1;
    }
    if( iIndex < 0 ) return false;
    if( m_items.GetSize() == 0 ) return false;
    if( iIndex >= m_items.GetSize() ) iIndex = m_items.GetSize() - 1;
    auto* pControl = static_cast<UIControl*>(m_items[iIndex]);
    if( !pControl || !pControl->IsVisible() || !pControl->IsEnabled() ) return false;
    IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(UIString{DUI_CTR_ILISTITEM}));
    if( pListItem == nullptr ) return false;
    m_curSel = iIndex;
    if( m_pWindow != nullptr || bTakeFocus ) pControl->SetFocus();
    pListItem->Select(true, bTriggerEvent);
    if( m_manager != nullptr && bTriggerEvent) m_manager->SendNotify(this, DUI_MSGTYPE_ITEMSELECT, (WPARAM)(long)m_curSel, (LPARAM)(long)iOldSel);
    Invalidate();

    return true;
}

bool UICombo::ExpandItem(int iIndex, bool bExpand) {
    return false;
}

int UICombo::GetExpandedItem() const {
    return -1;
}

bool UICombo::SetItemIndex(UIControl *control, int iNewIndex) {
    int iOrginIndex = GetItemIndex(control);
    if( iOrginIndex == -1 ) return false;
    if( iOrginIndex == iNewIndex ) return true;

    IListItemUI* pSelectedListItem = NULL;
    if( m_curSel >= 0 ) pSelectedListItem =
                                 static_cast<IListItemUI*>(GetItemAt(m_curSel)->GetInterface(UIString{DUI_CTR_ILISTITEM}));
    if( !UIContainer::SetItemIndex(control, iNewIndex) ) return false;
    int iMinIndex = min(iOrginIndex, iNewIndex);
    int iMaxIndex = max(iOrginIndex, iNewIndex);
    for(int i = iMinIndex; i < iMaxIndex + 1; ++i) {
        UIControl* p = GetItemAt(i);
        IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(UIString{DUI_CTR_ILISTITEM}));
        if( pListItem != NULL ) {
            pListItem->SetIndex(i);
        }
    }
    if( m_curSel >= 0 && pSelectedListItem != NULL ) m_curSel = pSelectedListItem->GetIndex();
    return true;
}

bool UICombo::SetMultiItemIndex(UIControl *startControl, int iCount, int iNewStartIndex) {
    if (startControl == nullptr || iCount < 0 || iNewStartIndex < 0) return false;
    int iStartIndex = GetItemIndex(startControl);
    if (iStartIndex == iNewStartIndex) return true;
    if (iStartIndex + iCount > GetCount()) return false;
    if (iNewStartIndex + iCount > GetCount()) return false;

    IListItemUI* pSelectedListItem = nullptr;
    if( m_curSel >= 0 ) pSelectedListItem =
                                 static_cast<IListItemUI*>(GetItemAt(m_curSel)->GetInterface(UIString{DUI_CTR_ILISTITEM}));
    if( !UIContainer::SetMultiItemIndex(startControl, iCount, iNewStartIndex) ) return false;
    int iMinIndex = min(iStartIndex, iNewStartIndex);
    int iMaxIndex = max(iStartIndex + iCount, iNewStartIndex + iCount);
    for(int i = iMinIndex; i < iMaxIndex + 1; ++i) {
        UIControl* p = GetItemAt(i);
        IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(UIString{DUI_CTR_ILISTITEM}));
        if( pListItem != nullptr ) {
            pListItem->SetIndex(i);
        }
    }
    if( m_curSel >= 0 && pSelectedListItem != nullptr ) m_curSel = pSelectedListItem->GetIndex();
    return true;
}

bool UICombo::Add(UIControl *control) {
    IListItemUI* pListItem = static_cast<IListItemUI*>(control->GetInterface(UIString{DUI_CTR_ILISTITEM}));
    if( pListItem != NULL )
    {
        pListItem->SetOwner(this);
        pListItem->SetIndex(m_items.GetSize());
    }
    return UIContainer::Add(control);
}

bool UICombo::AddAt(UIControl *control, int iIndex) {
    if (!UIContainer::AddAt(control, iIndex)) return false;

    // The list items should know about us
    IListItemUI* pListItem = static_cast<IListItemUI*>(control->GetInterface(UIString{DUI_CTR_ILISTITEM}));
    if( pListItem != NULL ) {
        pListItem->SetOwner(this);
        pListItem->SetIndex(iIndex);
    }

    for(int i = iIndex + 1; i < GetCount(); ++i) {
        UIControl* p = GetItemAt(i);
        pListItem = static_cast<IListItemUI*>(p->GetInterface(UIString{DUI_CTR_ILISTITEM}));
        if( pListItem != NULL ) {
            pListItem->SetIndex(i);
        }
    }
    if( m_curSel >= iIndex ) m_curSel += 1;
    return true;
}

bool UICombo::Remove(UIControl *control, bool bDoNotDestroy) {
    int iIndex = GetItemIndex(control);
    if (iIndex == -1) return false;

    if (!UIContainer::RemoveAt(iIndex, bDoNotDestroy)) return false;

    for(int i = iIndex; i < GetCount(); ++i) {
        UIControl* p = GetItemAt(i);
        IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(UIString{DUI_CTR_ILISTITEM}));
        if( pListItem != NULL ) {
            pListItem->SetIndex(i);
        }
    }

    if( iIndex == m_curSel && m_curSel >= 0 ) {
        int iSel = m_curSel;
        m_curSel = -1;
        SelectItem(FindSelectable(iSel, false));
    }
    else if( iIndex < m_curSel ) m_curSel -= 1;
    return true;
}

bool UICombo::RemoveAt(int iIndex, bool bDoNotDestroy) {
    if (!UIContainer::RemoveAt(iIndex, bDoNotDestroy)) return false;

    for(int i = iIndex; i < GetCount(); ++i) {
        UIControl* p = GetItemAt(i);
        IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(UIString{DUI_CTR_ILISTITEM}));
        if( pListItem != NULL ) pListItem->SetIndex(i);
    }

    if( iIndex == m_curSel && m_curSel >= 0 ) {
        int iSel = m_curSel;
        m_curSel = -1;
        SelectItem(FindSelectable(iSel, false));
    }
    else if( iIndex < m_curSel ) m_curSel -= 1;
    return true;
}

void UICombo::RemoveAll() {
    m_curSel = -1;
    UIContainer::RemoveAll();
}

bool UICombo::Activate() {
    if( !UIControl::Activate() ) return false;
    if( m_pWindow ) return true;
    m_pWindow = new UIComboWnd();
    assert(m_pWindow);
    m_pWindow->Init(this);
    if( m_manager != NULL ) m_manager->SendNotify(this, DUI_MSGTYPE_DROPDOWN);
    Invalidate();
    return true;
}

bool UICombo::GetShowText() const {
    return m_showText;
}

void UICombo::SetShowText(bool flag) {
    m_showText = flag;
    Invalidate();
}

RECT UICombo::GetTextPadding() const {
    return m_rcTextPadding;
}

void UICombo::SetTextPadding(RECT rc) {
    m_rcTextPadding = rc;
    Invalidate();
}

UIString UICombo::GetNormalImage() const {
    return m_diNormal.sDrawString;
}

void UICombo::SetNormalImage(const UIString &normalImage) {
    if( m_diNormal.sDrawString == normalImage && m_diNormal.pImageInfo != nullptr ) return;
    m_diNormal.Clear();
    m_diNormal.sDrawString = normalImage;
    Invalidate();
}

UIString UICombo::GetHotImage() const {
    return m_diHot.sDrawString;
}

void UICombo::SetHotImage(const UIString &hotImage) {
    if( m_diHot.sDrawString == hotImage && m_diHot.pImageInfo != nullptr ) return;
    m_diHot.Clear();
    m_diHot.sDrawString = hotImage;
    Invalidate();
}

UIString UICombo::GetPushedImage() const {
    return m_diPushed.sDrawString;
}

void UICombo::SetPushedImage(const UIString &pushedImage) {
    if( m_diPushed.sDrawString == pushedImage && m_diPushed.pImageInfo != nullptr ) return;
    m_diPushed.Clear();
    m_diPushed.sDrawString = pushedImage;
    Invalidate();
}

UIString UICombo::GetFocusedImage() const {
    return m_diFocused.sDrawString;
}

void UICombo::SetFocusedImage(const UIString &focusedImage) {
    if( m_diFocused.sDrawString == focusedImage && m_diFocused.pImageInfo != nullptr ) return;
    m_diFocused.Clear();
    m_diFocused.sDrawString = focusedImage;
    Invalidate();
}

UIString UICombo::GetDisabledImage() const {
    return m_diDisabled.sDrawString;
}

void UICombo::SetDisabledImage(const UIString &disabledImage) {
    if( m_diDisabled.sDrawString == disabledImage && m_diDisabled.pImageInfo != nullptr ) return;
    m_diDisabled.Clear();
    m_diDisabled.sDrawString = disabledImage;
    Invalidate();
}

TListInfoUI *UICombo::GetListInfo() {
    return &m_listInfo;
}

uint32_t UICombo::GetItemFixedHeight() {
    return m_listInfo.uFixedHeight;
}

void UICombo::SetItemFixedHeight(uint32_t nHeight) {
    m_listInfo.uFixedHeight = nHeight;
    Invalidate();
}

int UICombo::GetItemFont() {
    return m_listInfo.nFont;
}

void UICombo::SetItemFont(int index) {
    m_listInfo.nFont = index;
    Invalidate();
}

uint32_t UICombo::GetItemTextStyle() {
    return m_listInfo.textStyle;
}

void UICombo::SetItemTextStyle(uint32_t style) {
    m_listInfo.textStyle = style;
    Invalidate();
}

RECT UICombo::GetItemTextPadding() const {
    return m_listInfo.rcTextPadding;
}

void UICombo::SetItemTextPadding(RECT rc) {
    m_listInfo.rcTextPadding = rc;
    Invalidate();
}

uint32_t UICombo::GetItemTextColor() const {
    return m_listInfo.dwTextColor;
}

void UICombo::SetItemTextColor(uint32_t color) {
    m_listInfo.dwTextColor = color;
    Invalidate();
}

uint32_t UICombo::GetItemBkColor() const {
    return m_listInfo.dwBkColor;
}

void UICombo::SetItemBkColor(uint32_t color) {
    m_listInfo.dwBkColor = color;
    Invalidate();
}

UIString UICombo::GetItemBkImage() const {
    return m_listInfo.diBk.sDrawString;
}

void UICombo::SetItemBkImage(const UIString &bkImage) {
    if( m_listInfo.diBk.sDrawString == bkImage && m_listInfo.diBk.pImageInfo != nullptr ) return;
    m_listInfo.diBk.Clear();
    m_listInfo.diBk.sDrawString = bkImage;
}

bool UICombo::IsAlternateBk() const {
    return m_listInfo.bAlternateBk;
}

void UICombo::SetAlternateBk(bool bAlternateBk) {
    m_listInfo.bAlternateBk = bAlternateBk;
}

uint32_t UICombo::GetSelectedItemTextColor() const {
    return m_listInfo.dwSelectedTextColor;
}

void UICombo::SetSelectedItemTextColor(uint32_t textColor) {
    m_listInfo.dwSelectedTextColor = textColor;
}

uint32_t UICombo::GetSelectedItemBkColor() const {
    return m_listInfo.dwSelectedBkColor;
}

void UICombo::SetSelectedItemBkColor(uint32_t bkColor) {
    m_listInfo.dwSelectedBkColor = bkColor;
}

UIString UICombo::GetSelectedItemImage() const {
    return m_listInfo.diSelected.sDrawString;
}

void UICombo::SetSelectedItemImage(const UIString &selectedImage) {
    if( m_listInfo.diSelected.sDrawString == selectedImage && m_listInfo.diSelected.pImageInfo != NULL ) return;
    m_listInfo.diSelected.Clear();
    m_listInfo.diSelected.sDrawString = selectedImage;
}

uint32_t UICombo::GetHotItemTextColor() const {
    return m_listInfo.dwHotTextColor;
}

void UICombo::SetHotItemTextColor(uint32_t hotTextColor) {
    m_listInfo.dwHotTextColor = hotTextColor;
}

uint32_t UICombo::GetHotItemBkColor() const {
    return m_listInfo.dwHotBkColor;
}

void UICombo::SetHotItemBkColor(uint32_t bkColor) {
    m_listInfo.dwHotBkColor = bkColor;
}

UIString UICombo::GetHotItemImage() const {
    return m_listInfo.diHot.sDrawString;
}

void UICombo::SetHotItemImage(const UIString &hotImage) {
    if( m_listInfo.diHot.sDrawString == hotImage && m_listInfo.diHot.pImageInfo != NULL ) return;
    m_listInfo.diHot.Clear();
    m_listInfo.diHot.sDrawString = hotImage;
}

uint32_t UICombo::GetDisabledItemTextColor() const {
    return m_listInfo.dwDisabledTextColor;
}

void UICombo::SetDisabledItemTextColor(uint32_t textColor) {
    m_listInfo.dwDisabledTextColor = textColor;
}

uint32_t UICombo::GetDisabledItemBkColor() const {
    return m_listInfo.dwDisabledBkColor;
}

void UICombo::SetDisabledItemBkColor(uint32_t bkColor) {
    m_listInfo.dwDisabledBkColor = bkColor;
}

UIString UICombo::GetDisabledItemImage() const {
    return m_listInfo.diDisabled.sDrawString;
}

void UICombo::SetDisabledItemImage(const UIString &disabledImage) {
    if( m_listInfo.diDisabled.sDrawString == disabledImage && m_listInfo.diDisabled.pImageInfo != NULL ) return;
    m_listInfo.diDisabled.Clear();
    m_listInfo.diDisabled.sDrawString = disabledImage;
}

int UICombo::GetItemHLineSize() const {
    return m_listInfo.iHLineSize;
}

void UICombo::SetItemHLineSize(int iSize) {
    m_listInfo.iHLineSize = iSize;
}

uint32_t UICombo::GetItemHLineColor() const {
    return m_listInfo.dwHLineColor;
}

void UICombo::SetItemHLineColor(uint32_t lineColor) {
    m_listInfo.dwHLineColor = lineColor;
}

int UICombo::GetItemVLineSize() const {
    return m_listInfo.iVLineSize;
}

void UICombo::SetItemVLineSize(int iSize) {
    m_listInfo.iVLineSize = iSize;
}

uint32_t UICombo::GetItemVLineColor() const {
    return m_listInfo.dwVLineColor;
}

void UICombo::SetItemVLineColor(uint32_t lineColor) {
    m_listInfo.dwVLineColor = lineColor;
}

bool UICombo::IsItemShowHtml() {
    return m_listInfo.bShowHtml;
}

void UICombo::SetItemShowHtml(bool bShowHtml) {
    if( m_listInfo.bShowHtml == bShowHtml ) return;

    m_listInfo.bShowHtml = bShowHtml;
    Invalidate();
}

SIZE UICombo::EstimateSize(SIZE szAvailable) {
    uint32_t height = UIResourceMgr::GetInstance().GetDefaultFontHeight(m_manager->GetPaintDC());
    if( m_cxyFixed.cy == 0 ) return SIZE{m_cxyFixed.cx, (long)(height + 8)};
    return UIControl::EstimateSize(szAvailable);
}

void UICombo::SetPos(RECT rc, bool bNeedInvalidate) {
#ifdef __linux__
    if(m_pWindow != nullptr){ //linux系统下,当m_pWindow不为空，表示下拉框弹出，此时不能够设置
        //下拉元素大小为0,否则下拉显示空白。
        return;
    }
#endif
    // Put all elements out of sight
    RECT rcNull = { 0 };
    for( int i = 0; i < m_items.GetSize(); i++ ) static_cast<UIControl*>(m_items[i])->SetPos(rcNull, false);
    // Position this control
    UIControl::SetPos(rc, bNeedInvalidate);
}

void UICombo::Move(SIZE szOffset, bool bNeedInvalidate) {
    UIControl::Move(szOffset, bNeedInvalidate);
}

void UICombo::DoEvent(TEventUI &event) {
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_parent != NULL ) m_parent->DoEvent(event);
        else UIContainer::DoEvent(event);
        return;
    }

    if( event.Type == UIEVENT_SETFOCUS )
    {
        Invalidate();
    }
    if( event.Type == UIEVENT_KILLFOCUS )
    {
        Invalidate();
    }
    if( event.Type == UIEVENT_BUTTONDOWN )
    {
        if( IsEnabled() ) {
            Activate();
            m_buttonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
        }
        return;
    }
    if( event.Type == UIEVENT_BUTTONUP )
    {
        if( (m_buttonState & UISTATE_CAPTURED) != 0 ) {
            m_buttonState &= ~ UISTATE_CAPTURED;
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_MOUSEMOVE )
    {
        return;
    }
    if( event.Type == UIEVENT_KEYDOWN )
    {
        if (IsKeyboardEnabled() && IsEnabled()) {
            switch( event.chKey ) {
                case VK_F4:
                    Activate();
                    break;
                case VK_UP:
                    SetSelectCloseFlag(false);
                    SelectItem(FindSelectable(m_curSel - 1, false));
                    SetSelectCloseFlag(true);
                    break;
                case VK_DOWN:
                    SetSelectCloseFlag(false);
                    SelectItem(FindSelectable(m_curSel + 1, true));
                    SetSelectCloseFlag(true);
                    break;
                case VK_PRIOR:
                    SetSelectCloseFlag(false);
                    SelectItem(FindSelectable(m_curSel - 1, false));
                    SetSelectCloseFlag(true);
                    break;
                case VK_NEXT:
                    SetSelectCloseFlag(false);
                    SelectItem(FindSelectable(m_curSel + 1, true));
                    SetSelectCloseFlag(true);
                    break;
                case VK_HOME:
                    SetSelectCloseFlag(false);
                    SelectItem(FindSelectable(0, false));
                    SetSelectCloseFlag(true);
                    break;
                case VK_END:
                    SetSelectCloseFlag(false);
                    SelectItem(FindSelectable(GetCount() - 1, true));
                    SetSelectCloseFlag(true);
                    break;
            }
            return;
        }
    }
    if( event.Type == UIEVENT_SCROLLWHEEL )
    {
        if (IsEnabled()) {
            bool bDownward = LOWORD(event.wParam) == SB_LINEDOWN;
            SetSelectCloseFlag(false);
            SelectItem(FindSelectable(m_curSel + (bDownward ? 1 : -1), bDownward));
            SetSelectCloseFlag(true);
            return;
        }
    }
    if( event.Type == UIEVENT_CONTEXTMENU )
    {
        return;
    }
    if( event.Type == UIEVENT_MOUSEENTER )
    {
        UIRect rcItem{m_rcItem};
        if( rcItem.IsPtIn(event.ptMouse ) ) {
            if( IsEnabled() ) {
                if( (m_buttonState & UISTATE_HOT) == 0  ) {
                    m_buttonState |= UISTATE_HOT;
                    Invalidate();
                }
            }
        }
    }
    if( event.Type == UIEVENT_MOUSELEAVE )
    {
        UIRect rcItem{m_rcItem};
        if( !rcItem.IsPtIn( event.ptMouse ) ) {
            if( IsEnabled() ) {
                if( (m_buttonState & UISTATE_HOT) != 0  ) {
                    m_buttonState &= ~UISTATE_HOT;
                    Invalidate();
                }
            }
            if (m_manager) m_manager->RemoveMouseLeaveNeeded(this);
        }
        else {
            if (m_manager) m_manager->AddMouseLeaveNeeded(this);
            return;
        }
    }
    UIControl::DoEvent(event);
}

void UICombo::SetAttribute(const char *pstrName, const char *pstrValue) {
    if( strcasecmp(pstrName, "textpadding") == 0 ) {
        RECT rcTextPadding = { 0 };
        char *pstr = nullptr;
        rcTextPadding.left = strtol(pstrValue, &pstr, 10);  assert(pstr);
        rcTextPadding.top = strtol(pstr + 1, &pstr, 10);    assert(pstr);
        rcTextPadding.right = strtol(pstr + 1, &pstr, 10);  assert(pstr);
        rcTextPadding.bottom = strtol(pstr + 1, &pstr, 10); assert(pstr);
        SetTextPadding(rcTextPadding);
    }
    else if( strcasecmp(pstrName, "showtext") == 0 ) SetShowText(strcasecmp(pstrValue, "true") == 0);
    else if( strcasecmp(pstrName, "normalimage") == 0 ) SetNormalImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "hotimage") == 0 ) SetHotImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "pushedimage") == 0 ) SetPushedImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "focusedimage") == 0 ) SetFocusedImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "disabledimage") == 0 ) SetDisabledImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "dropbox") == 0 ) SetDropBoxAttributeList(pstrValue);
    else if( strcasecmp(pstrName, "dropboxsize") == 0)
    {
        SIZE szDropBoxSize = { 0 };
        char *pstr = nullptr;
        szDropBoxSize.cx = strtol(pstrValue, &pstr, 10);  assert(pstr);
        szDropBoxSize.cy = strtol(pstr + 1, &pstr, 10);    assert(pstr);
        SetDropBoxSize(szDropBoxSize);
    }
    else if( strcasecmp(pstrName, "itemheight") == 0 ) m_listInfo.uFixedHeight = atoi(pstrValue);
    else if( strcasecmp(pstrName, "itemfont") == 0 ) m_listInfo.nFont = atoi(pstrValue);
    else if( strcasecmp(pstrName, "itemalign") == 0 ) {
        if( strstr(pstrValue, "left") != NULL ) {
            m_listInfo.textStyle &= ~(DT_CENTER | DT_RIGHT);
            m_listInfo.textStyle |= DT_LEFT;
        }
        if( strstr(pstrValue, "center") != NULL ) {
            m_listInfo.textStyle &= ~(DT_LEFT | DT_RIGHT);
            m_listInfo.textStyle |= DT_CENTER;
        }
        if( strstr(pstrValue, "right") != NULL ) {
            m_listInfo.textStyle &= ~(DT_LEFT | DT_CENTER);
            m_listInfo.textStyle |= DT_RIGHT;
        }
    }
    if( strcasecmp(pstrName, "itemtextpadding") == 0 ) {
        RECT rcTextPadding = { 0 };
        char *pstr = nullptr;
        rcTextPadding.left = strtol(pstrValue, &pstr, 10);  assert(pstr);
        rcTextPadding.top = strtol(pstr + 1, &pstr, 10);    assert(pstr);
        rcTextPadding.right = strtol(pstr + 1, &pstr, 10);  assert(pstr);
        rcTextPadding.bottom = strtol(pstr + 1, &pstr, 10); assert(pstr);
        SetItemTextPadding(rcTextPadding);
    }
    else if( strcasecmp(pstrName, "itemtextcolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetItemTextColor(clrColor);
    }
    else if( strcasecmp(pstrName, "itembkcolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetItemBkColor(clrColor);
    }
    else if( strcasecmp(pstrName, "itembkimage") == 0 ) SetItemBkImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "itemaltbk") == 0 ) SetAlternateBk(strcasecmp(pstrValue, "true") == 0);
    else if( strcasecmp(pstrName, "itemselectedtextcolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetSelectedItemTextColor(clrColor);
    }
    else if( strcasecmp(pstrName, "itemselectedbkcolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetSelectedItemBkColor(clrColor);
    }
    else if( strcasecmp(pstrName, "itemselectedimage") == 0 ) SetSelectedItemImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "itemhottextcolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetHotItemTextColor(clrColor);
    }
    else if( strcasecmp(pstrName, "itemhotbkcolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetHotItemBkColor(clrColor);
    }
    else if( strcasecmp(pstrName, "itemhotimage") == 0 ) SetHotItemImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "itemdisabledtextcolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetDisabledItemTextColor(clrColor);
    }
    else if( strcasecmp(pstrName, "itemdisabledbkcolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetDisabledItemBkColor(clrColor);
    }
    else if( strcasecmp(pstrName, "itemdisabledimage") == 0 ) SetDisabledItemImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "itemvlinesize") == 0 ) {
        SetItemVLineSize(atoi(pstrValue));
    }
    else if( strcasecmp(pstrName, "itemvlinecolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetItemVLineColor(clrColor);
    }
    else if( strcasecmp(pstrName, "itemhlinesize") == 0 ) {
        SetItemHLineSize(atoi(pstrValue));
    }
    else if( strcasecmp(pstrName, "itemhlinecolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetItemHLineColor(clrColor);
    }
    else if( strcasecmp(pstrName, "itemshowhtml") == 0 ) SetItemShowHtml(strcasecmp(pstrValue, "true") == 0);
    else UIContainer::SetAttribute(pstrName, pstrValue);
}

bool UICombo::DoPaint(HANDLE_DC hDC, const RECT &rcPaint, UIControl *stopControl) {
    return UIControl::DoPaint(hDC, rcPaint, stopControl);
}

void UICombo::PaintText(HANDLE_DC hDC) {
    if (!m_showText) return;

    RECT rcText = m_rcItem;
    rcText.left += m_rcTextPadding.left;
    rcText.right -= m_rcTextPadding.right;
    rcText.top += m_rcTextPadding.top;
    rcText.bottom -= m_rcTextPadding.bottom;

    if( m_curSel >= 0 ) {
        auto* pControl = static_cast<UIControl*>(m_items[m_curSel]);
        IListItemUI* pElement = static_cast<IListItemUI*>(pControl->GetInterface(UIString{DUI_CTR_ILISTITEM}));
        if( pElement != nullptr ) {
            pElement->DrawItemText(hDC, rcText);
        }
        else {
            RECT rcOldPos = pControl->GetPos();
            pControl->SetPos(rcText, false);
            pControl->Paint(hDC, rcText, nullptr);
            pControl->SetPos(rcOldPos, false);
        }
    }
}

void UICombo::PaintStatusImage(HANDLE_DC hDC) {
    if( IsFocused() ) m_buttonState |= UISTATE_FOCUSED;
    else m_buttonState &= ~ UISTATE_FOCUSED;
    if( !IsEnabled() ) m_buttonState |= UISTATE_DISABLED;
    else m_buttonState &= ~ UISTATE_DISABLED;

    if( (m_buttonState & UISTATE_DISABLED) != 0 ) {
        if (DrawImage(hDC, m_diDisabled)) return;
    }
    else if( (m_buttonState & UISTATE_PUSHED) != 0 ) {
        if (DrawImage(hDC, m_diPushed)) return;
    }
    else if( (m_buttonState & UISTATE_HOT) != 0 ) {
        if (DrawImage(hDC, m_diHot)) return;
    }
    else if( (m_buttonState & UISTATE_FOCUSED) != 0 ) {
        if (DrawImage(hDC, m_diFocused)) return;
    }

    DrawImage(hDC, m_diNormal);
}
