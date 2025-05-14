#include "../../include/UIBaseWindow.h"
#include "../../include/UIRect.h"
#include <windowsx.h>
#include "EncodingTransform.h"
#include <cassert>
#include <UIBaseWindowObjects.h>

#include <iostream>
using namespace std;

class UIBaseWindowPrivate
{
public:

    //HANDLE_WND Create(HANDLE_WND parent, const UIString &className, int x, int y, int nWidth, int nHeight, LPVOID param);
    HANDLE_WND Create(HANDLE_WND parent, const UIString &className, uint32_t style, uint32_t exStyle,int x,int y,int nWidth, int nHeight,LPVOID param);
    static LRESULT CALLBACK __WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
    bool RegisterWindowClass(const wchar_t *className);
public:
    HANDLE_WND  m_hWnd;
    HANDLE_WND  m_parent;
    UIString    m_className;
};

HANDLE_WND
UIBaseWindowPrivate::Create(HANDLE_WND parent, const UIString &className, uint32_t style, uint32_t exStyle, int x,
                            int y, int nWidth, int nHeight, LPVOID param) {
    wchar_t *wideClassName = Utf8ToUcs2(className.GetData(),-1);
    if(wideClassName == nullptr){
        return nullptr;
    }
    if(!this->RegisterWindowClass(wideClassName)){
        delete []wideClassName;
        return nullptr;
    }
    m_hWnd = ::CreateWindowExW(exStyle, wideClassName,
                               wideClassName, style, x, y, nWidth, nHeight,
                               parent,
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
    ATOM ret = RegisterClassExW(&wndClass);
    return ret != 0 || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

LRESULT CALLBACK UIBaseWindowPrivate::__WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    UIBaseWindow* pThis = nullptr;
    if( uMsg == WM_NCCREATE ) {
        auto lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pThis = static_cast<UIBaseWindow*>(lpcs->lpCreateParams);
        pThis->SetWND(hWnd);
        UIBaseWindowObjects::GetInstance().AddObject(hWnd,pThis);
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
            UIBaseWindowObjects::GetInstance().RemoveObject(hWnd);
            if (UIBaseWindowObjects::GetInstance().GetWindowCount()==0 && UIPaintManager::GetQuitOnLastWindowDestroy()) {
                UI_APP_QUIT();
            }
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

HANDLE_WND
UIBaseWindow::Create(HANDLE_WND parent, const UIString &className, uint32_t style, uint32_t exStyle, RECT rc) {
    return m_data->Create(parent,className,style, exStyle,rc.left,rc.top,
                          rc.right-rc.left, rc.bottom-rc.top,this);
}

HANDLE_WND
UIBaseWindow::Create(HANDLE_WND parent, const UIString &className, uint32_t style, uint32_t exStyle, int x, int y,
                     int cx, int cy) {
    return m_data->Create(parent, className, style, exStyle,x,y,cx,cy,this);
}

void UIBaseWindow::ShowWindow(bool bShow)const {
    if( !::IsWindow(this->GetWND()) ) return;
    ::ShowWindow(this->GetWND(), bShow ? SW_SHOWNORMAL : SW_HIDE);
}

DuiResponseVal UIBaseWindow::ShowModal() {
    assert(::IsWindow(this->GetWND()));
    UINT nRet = 0;
    HWND hWndParent = GetWindowOwner(this->GetWND());
    ::ShowWindow(this->GetWND(), SW_SHOWNORMAL);
    ::EnableWindow(hWndParent, FALSE);
    MSG msg = { 0 };
    while( ::IsWindow(this->GetWND()) && ::GetMessageW(&msg, nullptr, 0, 0) ) {
        if( msg.message == WM_CLOSE && msg.hwnd == this->GetWND() ) {
            nRet = msg.wParam;
            ::EnableWindow(hWndParent, TRUE);
            ::SetFocus(hWndParent);
        }
        //if( !CPaintManagerUI::TranslateMessage(&msg) ) {
        ::TranslateMessage(&msg);
        ::DispatchMessageW(&msg);
        //}
        if( msg.message == WM_QUIT ) break;
    }
    ::EnableWindow(hWndParent, TRUE);
    ::SetFocus(hWndParent);
    if( msg.message == WM_QUIT ) ::PostQuitMessage(msg.wParam);
    return (DuiResponseVal)(nRet);
}

void UIBaseWindow::Close(DuiResponseVal val) const {
    ::PostMessageW(this->GetWND(), WM_CLOSE, WPARAM(val),0);
}

void UIBaseWindow::Maximize()const {
    ::SendMessageW(this->GetWND(), WM_SYSCOMMAND, SC_MAXIMIZE,0);
}

void UIBaseWindow::Restore()const {
    ::SendMessageW(this->GetWND(), WM_SYSCOMMAND, SC_RESTORE,0);
}

void UIBaseWindow::Minimize()const {
    ::SendMessageW(this->GetWND(), WM_SYSCOMMAND, SC_MINIMIZE,0);
}

HANDLE_WND UIBaseWindow::GetWND()const {
    return m_data->m_hWnd;
}

void UIBaseWindow::SetWND(HANDLE_WND wndHandle) const{
    m_data->m_hWnd = wndHandle;
}

void UIBaseWindow::OnFinalMessage(HANDLE_WND hWnd) {
    cout<<"On Final Message:"<<endl;
}

void UIBaseWindow::CenterWindow() const {
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
    return (long)::DefWindowProcW(this->GetWND(), uMsg, wParam, lParam);
}
