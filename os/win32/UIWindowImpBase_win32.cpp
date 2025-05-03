#include <UIWindowImpBase.h>
#include <UIRect.h>
#include <windowsx.h>

static long OnNcHitTest(HANDLE_WND hWnd, UIPaintManager *paintManager, uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled)
{
    POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
    ::ScreenToClient(hWnd, &pt);

    RECT rcClient;
    ::GetClientRect(hWnd, &rcClient);

    RECT rcCaption = paintManager->GetCaptionRect();
    if( pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
			&& pt.y >= rcCaption.top && pt.y < rcCaption.bottom ) {
        auto* pControl = static_cast<UIControl*>(paintManager->FindControl(pt));
        if( pControl && (pControl->GetClass() != DUI_CTR_BUTTON) &&
            (pControl->GetClass() != DUI_CTR_OPTION) &&
            (pControl->GetClass() != DUI_CTR_TEXT) )
            return HTCAPTION;
    }

    return HTCLIENT;
}

static long OnGetMinMaxInfo(HANDLE_WND wndHandle, uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool bHandled)
{
    MONITORINFO  monitorinfo = {0};
    monitorinfo.cbSize = sizeof(MONITORINFO);
    ::GetMonitorInfoW(::MonitorFromWindow(wndHandle,MONITOR_DEFAULTTOPRIMARY), &monitorinfo);
    UIRect  rcWork{monitorinfo.rcWork};
    rcWork.Offset(-monitorinfo.rcMonitor.left, -monitorinfo.rcMonitor.top);
    auto  lpMMI = (LPMINMAXINFO)lParam;
    lpMMI->ptMaxPosition.x = rcWork.left;
    lpMMI->ptMaxPosition.y = rcWork.top;
    lpMMI->ptMaxSize.x     = rcWork.right;
    lpMMI->ptMaxSize.y     = rcWork.bottom;
    bHandled = false;
    return 0;
}

long UIWindowImpBase::HandleMessage(uint32_t uMsg, WPARAM wParam, LPARAM lParam) {
    long lRes = 0;

    bool bHandle = true;
    switch(uMsg){
        case DUI_WM_CREATE:
            lRes = OnCreate(uMsg, wParam, lParam, bHandle);
            break;
        case DUI_WM_CLOSE:
            lRes = OnClose(uMsg, wParam, lParam, bHandle);
            break;
        case DUI_WM_DESTROY:
            lRes = OnDestroy(uMsg, wParam, lParam, bHandle);
            break;
        case DUI_WM_SIZE:
            lRes = OnSize(uMsg, wParam, lParam, bHandle);
            break;
        case WM_GETMINMAXINFO:
            lRes = OnGetMinMaxInfo(this->GetWND(), uMsg, wParam, lParam, bHandle);
            break;
        case WM_NCACTIVATE:
            if( ::IsIconic(this->GetWND()) ) bHandle = FALSE;
            return (wParam == 0) ? TRUE : FALSE;
        case WM_NCCALCSIZE:
            return 0;
        case WM_NCPAINT:
            return 0;
        case WM_ERASEBKGND:
            return 1;
        case WM_KEYDOWN:
            bHandle = false;
            if(wParam == VK_ESCAPE || wParam==VK_RETURN){
                UI_DESTROY_WINDOW(this->GetWND());
            }
            break;
        case WM_NCHITTEST:
            lRes = OnNcHitTest(this->GetWND(), &m_pm,uMsg, wParam, lParam,bHandle);
            break;
        case DUI_WM_KILLFOCUS:
            lRes = OnKillFocus(uMsg, wParam,lParam, bHandle);
            break;
        default:
            bHandle = false;
    }
    if(bHandle)return lRes;
    if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
    return UIBaseWindow::HandleMessage(uMsg, wParam, lParam);
}

long UIWindowImpBase::OnCreate(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) {
    long styleValue = ::GetWindowLong(this->GetWND(), GWL_STYLE);
    styleValue &= ~WS_CAPTION;
    styleValue &= ~WS_THICKFRAME;
    ::SetWindowLong(this->GetWND(), GWL_STYLE, styleValue);
    return 0;
}

long UIWindowImpBase::OnClose(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) {
    bHandled = false;
    return 0;
}

long UIWindowImpBase::OnDestroy(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) {
    bHandled = true;
    return 0;
}

long UIWindowImpBase::OnSize(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) {
    SIZE szRoundCorner = m_pm.GetRoundCorner();
    if( !::IsIconic(this->GetWND()) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0) ) {
        UIRect rcWnd;
        ::GetWindowRect(this->GetWND(), &rcWnd);
        rcWnd.Offset(-rcWnd.left, -rcWnd.top);
        rcWnd.right++; rcWnd.bottom++;
        HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szRoundCorner.cx, szRoundCorner.cy);
        ::SetWindowRgn(this->GetWND(), hRgn, TRUE);
        ::DeleteObject(hRgn);
    }

    bHandled = FALSE;
    return 0;
}
