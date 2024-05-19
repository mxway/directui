#include "UIBaseWindow.h"
#include "UIRect.h"
#include <windowsx.h>
#include "EncodingTransform.h"
#include <UIResourceMgr.h>
#include <cassert>

#include <iostream>
using namespace std;

class UIBaseWindowPrivate
{
public:

    HANDLE_WND Create(HANDLE_WND parent, const UIString &className, int x, int y, int nWidth, int nHeight, LPVOID param);
    static LRESULT CALLBACK __WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
    bool RegisterWindowClass(const wchar_t *className);
public:
    HANDLE_WND  m_hWnd;
    HANDLE_WND  m_parent;
    UIString    m_className;
};

HANDLE_WND
UIBaseWindowPrivate::Create(HANDLE_WND parent, const UIString &className, int x, int y, int nWidth, int nHeight, LPVOID param) {
    wchar_t *wideClassName = Utf8ToUcs2(className.GetData(),-1);
    if(wideClassName == nullptr){
        return nullptr;
    }
    if(!this->RegisterWindowClass(wideClassName)){
        delete []wideClassName;
        return nullptr;
    }
    m_hWnd = ::CreateWindowExW(WS_EX_APPWINDOW, wideClassName,
                               wideClassName, WS_VISIBLE|WS_OVERLAPPEDWINDOW, x, y, nWidth, nHeight,
                               nullptr,
                               nullptr,
                               GetModuleHandleW(nullptr), param);
    delete []wideClassName;
    return m_hWnd;
}

bool UIBaseWindowPrivate::RegisterWindowClass(const wchar_t *className) {
    WNDCLASSEXW  wndClass{0};
    wndClass.cbSize = sizeof(WNDCLASSEXW);
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.style = (CS_DBLCLKS);
    wndClass.lpfnWndProc = UIBaseWindowPrivate::__WndProc;
    wndClass.hInstance = GetModuleHandleW(nullptr);
    wndClass.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    wndClass.lpszClassName = className;
    return RegisterClassExW(&wndClass);
}

LRESULT CALLBACK UIBaseWindowPrivate::__WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    UIBaseWindow* pThis = NULL;
    if( uMsg == WM_NCCREATE ) {
        LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pThis = static_cast<UIBaseWindow*>(lpcs->lpCreateParams);
        pThis->SetWND(hWnd);
        ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(pThis));
    }
    else {
        pThis = reinterpret_cast<UIBaseWindow*>(::GetWindowLongPtrW(hWnd, GWLP_USERDATA));
        if( uMsg == WM_NCDESTROY && pThis != NULL ) {
            //LRESULT lRes = ::CallWindowProc(pThis->m_OldWndProc, hWnd, uMsg, wParam, lParam);
            ::SetWindowLongPtrW(pThis->GetWND(), GWLP_USERDATA, 0L);
            //if( pThis->m_bSubclassed ) pThis->Unsubclass();
            //pThis->m_hWnd = NULL;
            pThis->OnFinalMessage(hWnd);
            return 0;
        }
    }
    if( pThis != NULL ) {
        return pThis->HandleMessage(uMsg, wParam, lParam);
    }
    else {
        return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }
}

UIBaseWindow::UIBaseWindow()
    :m_data {make_shared<UIBaseWindowPrivate>()}
{

}

HANDLE_WND UIBaseWindow::Create(HANDLE_WND parent, const UIString &className, int x, int y, int nWidth, int nHeight) {
    return m_data->Create(parent, className, x, y, nWidth, nHeight, this);
}

void UIBaseWindow::ShowWindow(bool bShow) {
    if( !::IsWindow(this->GetWND()) ) return;
    ::ShowWindow(this->GetWND(), bShow ? SW_SHOWNORMAL : SW_HIDE);
}

void UIBaseWindow::Maximize() {
    ::SendMessageW(this->GetWND(), WM_SYSCOMMAND, SC_MAXIMIZE,0);
}

void UIBaseWindow::Restore() {
    ::SendMessageW(this->GetWND(), WM_SYSCOMMAND, SC_RESTORE,0);
}

void UIBaseWindow::Minimize() {
    ::SendMessageW(this->GetWND(), WM_SYSCOMMAND, SC_MINIMIZE,0);
}

HANDLE_WND UIBaseWindow::GetWND() {
    return m_data->m_hWnd;
}

void UIBaseWindow::SetWND(HANDLE_WND wndHandle) {
    m_data->m_hWnd = wndHandle;
}

void UIBaseWindow::OnFinalMessage(HANDLE_WND hWnd) {
    cout<<"On Final Message:"<<endl;
}

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
    LPMINMAXINFO  lpMMI = (LPMINMAXINFO)lParam;
    lpMMI->ptMaxPosition.x = rcWork.left;
    lpMMI->ptMaxPosition.y = rcWork.top;
    lpMMI->ptMaxSize.x     = rcWork.right;
    lpMMI->ptMaxSize.y     = rcWork.bottom;
    bHandled = false;
    return 0;
}

void UIBaseWindow::CenterWindow() {
    assert(::IsWindow(this->GetWND()));
    assert((GetWindowStyle(this->GetWND())&WS_CHILD)==0);
    RECT rcDlg = { 0 };
    ::GetWindowRect(this->GetWND(), &rcDlg);
    RECT rcArea = { 0 };
    RECT rcCenter = { 0 };
    HWND hWnd=this->GetWND();
    HWND hWndParent = ::GetParent(this->GetWND());
    HWND hWndCenter = ::GetWindowOwner(this->GetWND());
    if (hWndCenter!=nullptr)
        hWnd=hWndCenter;

    // 处理多显示器模式下屏幕居中
    MONITORINFO oMonitor = {};
    oMonitor.cbSize = sizeof(oMonitor);
    ::GetMonitorInfo(::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST), &oMonitor);
    rcArea = oMonitor.rcWork;

    if( hWndCenter == nullptr || IsIconic(hWndCenter))
        rcCenter = rcArea;
    else
        ::GetWindowRect(hWndCenter, &rcCenter);

    int DlgWidth = rcDlg.right - rcDlg.left;
    int DlgHeight = rcDlg.bottom - rcDlg.top;

    // Find dialog's upper left based on rcCenter
    int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
    int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

    // The dialog is outside the screen, move it inside
    if( xLeft < rcArea.left ) xLeft = rcArea.left;
    else if( xLeft + DlgWidth > rcArea.right ) xLeft = rcArea.right - DlgWidth;
    if( yTop < rcArea.top ) yTop = rcArea.top;
    else if( yTop + DlgHeight > rcArea.bottom ) yTop = rcArea.bottom - DlgHeight;
    ::SetWindowPos(this->GetWND(), nullptr, xLeft, yTop, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

long UIBaseWindow::HandleMessage(uint32_t uMsg, WPARAM wParam, LPARAM lParam) {

    long lRes = 0;
    bool bHandle = true;
    HDC hdc = nullptr;
    PAINTSTRUCT ps = { 0 };
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
            if(wParam == VK_ESCAPE || wParam==VK_RETURN){
                UI_DESTROY_WINDOW(this->GetWND());
            }
            break;
        case WM_NCHITTEST:
            lRes = OnNcHitTest(this->GetWND(), &m_pm,uMsg, wParam, lParam,bHandle);
            break;
        default:
            bHandle = false;
    }
    if(bHandle)return lRes;
    return ::DefWindowProcW(this->GetWND(),uMsg, wParam, lParam);
}

long UIBaseWindow::OnCreate(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) {
    long styleValue = ::GetWindowLong(this->GetWND(), GWL_STYLE);
    styleValue &= ~WS_CAPTION;
    styleValue &= ~WS_THICKFRAME;
    ::SetWindowLong(this->GetWND(), GWL_STYLE, styleValue);
    return 0;
}

long UIBaseWindow::OnClose(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) {
    return 0;
}

long UIBaseWindow::OnDestroy(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) {
    bHandled = true;
    return 0;
}

long UIBaseWindow::OnSize(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) {
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
