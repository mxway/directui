#include "UICombo.h"
#include <windowsx.h>
#include <UIResourceMgr.h>
#include "../../src/UIComboWnd.h"
#include <UIRect.h>

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

long UIComboWnd::HandleMessage_Internal(uint32_t uMsg, WPARAM wParam, LPARAM lParam) {
    if( uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONDBLCLK ) {
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
}