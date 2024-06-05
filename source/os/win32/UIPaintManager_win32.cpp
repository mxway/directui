#include "UIPaintManager.h"
#include <windows.h>
#include <UIBaseWindow.h>
#include <UIRenderEngine.h>
#include <iostream>
#include <cassert>
#include <windowsx.h>
#include <commctrl.h>
#include <chrono>

using namespace std;
using namespace chrono;

typedef struct tagTIMERINFO
{
    UIControl   *pSender;
    uint32_t    uTimerId;
    HANDLE_WND  hWnd;
    bool        killed;
}TIMERINFO;

class UIPaintManagerInternalImp
{
public:
    HDC m_hDcPaint;
    HDC m_hDcOffscreen;
    HDC m_hDcBackground;
    HBITMAP m_hbmpOffscreen;
    COLORREF* m_pOffscreenBits;
    HBITMAP m_hbmpBackground;
    COLORREF* m_pBackgroundBits;
};

static UINT MapKeyState()
{
    UINT uState = 0;
    if( ::GetKeyState(VK_CONTROL) < 0 ) uState |= MK_CONTROL;
    if( ::GetKeyState(VK_RBUTTON) < 0 ) uState |= MK_RBUTTON;
    if( ::GetKeyState(VK_LBUTTON) < 0 ) uState |= MK_LBUTTON;
    if( ::GetKeyState(VK_SHIFT) < 0 ) uState |= MK_SHIFT;
    if( ::GetKeyState(VK_MENU) < 0 ) uState |= MK_ALT;
    return uState;
}

UIPaintManager::UIPaintManager()
    :m_impl {make_shared<UIPaintManagerInternalImp>()},
    m_sName{},
    m_paintWnd{nullptr},
    m_bUpdateNeeded{false},
    m_bNoActivate{false},
    m_iHoverTime {1000},
     m_ptLastMousePos{-1, -1},
     m_szMinWindow{0,0},
     m_szMaxWindow{0,0},
     m_szInitWindowSize{0,0},
     m_rcSizeBox{0,0,0,0},
     m_roundCorner{0,0},
     m_rcCaption {0,0,0,30},
     m_pRoot {nullptr},
     m_pFocus {nullptr},
     m_pEventHover {nullptr},
     m_pEventClick {nullptr},
     m_bMouseTracking {false},
     m_bMouseCapture {false},
     m_defaultDisabledColor{0xFFA7A6AA},
     m_defaultFontColor{0xFF000000},
     m_defaultLinkFontColor{0xFF0000FF},
     m_defaultLinkHoverFontColor{0xFFD3215F},
     m_defaultSelectedBkColor{0xFFBAE4FF}
{

}

void UIPaintManager::Init(HANDLE_WND hWnd, const UIString &name) {
    m_paintWnd = hWnd;
    m_impl->m_hDcPaint = ::GetDC(m_paintWnd);
    m_sName = name;
}

HANDLE_DC UIPaintManager::GetPaintDC() {
    return m_impl->m_hDcPaint;
}

void UIPaintManager::MessageLoop() {
    MSG msg = { nullptr };
    while( ::GetMessageW(&msg, nullptr, 0, 0) ) {
        ::TranslateMessage(&msg);
        ::DispatchMessageW(&msg);
    }
}

void UIPaintManager::SetCapture() {
    ::SetCapture(m_paintWnd);
    m_bMouseCapture = true;
}

void UIPaintManager::ReleaseCapture() {
    ::ReleaseCapture();
    m_bMouseCapture = false;
}

void UIPaintManager::Invalidate(RECT &rcItem) {
    if( rcItem.left < 0 ) rcItem.left = 0;
    if( rcItem .top < 0 ) rcItem.top = 0;
    if( rcItem.right < rcItem.left ) rcItem.right = rcItem.left;
    if( rcItem.bottom < rcItem.top ) rcItem.bottom = rcItem.top;
    ::InvalidateRect(m_paintWnd, &rcItem, FALSE);
}

bool UIPaintManager::MessageHandler(uint32_t uMsg, WPARAM wParam, LPARAM lParam, long &lRes) {
    if( m_paintWnd == nullptr ) return false;
    switch (uMsg) {
        case WM_ERASEBKGND:
        {
            // We'll do the painting here...
            lRes = 1;
            return true;
        }
        case DUI_WM_PAINT:
        {
            static bool bInit = false;
            if(!bInit){
                bInit = true;
                SendNotify(m_pRoot, DUI_MSGTYPE_WINDOWINIT, 0,0, false);
            }
            if( m_pRoot == nullptr ) {
                PAINTSTRUCT ps = { nullptr };
                ::BeginPaint(m_paintWnd, &ps);
                UIRenderEngine::DrawColor(m_impl->m_hDcPaint, ps.rcPaint, 0xFF000000);
                ::EndPaint(m_paintWnd, &ps);
                return true;
            }

            RECT rcClient = { 0 };
            ::GetClientRect(m_paintWnd, &rcClient);

            RECT rcPaint = { 0 };

            if( !::GetUpdateRect(m_paintWnd, &rcPaint, FALSE) ) return true;
            PAINTSTRUCT ps = {nullptr};
            if( m_bUpdateNeeded ) {
                m_bUpdateNeeded = false;
                if( !::IsRectEmpty(&rcClient) ) {
                    if( m_pRoot->IsUpdateNeeded() ) {
                        RECT rcRoot = rcClient;
                        m_pRoot->SetPos(rcRoot, true);
                    }else{
                        UIControl* pControl = NULL;
                        m_aFoundControls.Empty();
                        m_pRoot->FindControl(__FindControlsFromUpdate, NULL, UIFIND_VISIBLE | UIFIND_ME_FIRST | UIFIND_UPDATETEST);
                        cout<<"Update Control Count:"<<m_aFoundControls.GetSize()<<endl;
                        for( int it = 0; it < m_aFoundControls.GetSize(); it++ ) {
                            pControl = static_cast<UIControl*>(m_aFoundControls[it]);
                            if( !pControl->IsFloat() ) pControl->SetPos(pControl->GetPos(), true);
                            else pControl->SetPos(pControl->GetRelativePos(), true);
                        }
                    }
                }
            }

            HDC hDcOffscreen = ::CreateCompatibleDC(m_impl->m_hDcPaint);
            HBITMAP hbmpOffscreen = ::CreateCompatibleBitmap(m_impl->m_hDcPaint, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);
            ::BeginPaint(m_paintWnd,&ps);
            //m_pRoot->Paint(m_impl->m_hDcPaint,rcPaint,nullptr);
            auto hOldBitmap = (HBITMAP)::SelectObject(hDcOffscreen, hbmpOffscreen);
            //int iSaveDc = ::SaveDC(hDcOffscreen);
            auto start = chrono::high_resolution_clock::now();
            m_pRoot->Paint(hDcOffscreen,rcPaint,nullptr);
            //::RestoreDC(hDcOffscreen, iSaveDc);
            ::BitBlt(m_impl->m_hDcPaint, rcPaint.left, rcPaint.top, rcPaint.right - rcPaint.left,
                     rcPaint.bottom - rcPaint.top, hDcOffscreen, rcPaint.left, rcPaint.top, SRCCOPY);
            //::BitBlt(m_hDcPaint, rcPaint.left, rcPaint.top, rcPaint.right - rcPaint.left,
            //         rcPaint.bottom - rcPaint.top, m_hDcOffscreen, rcPaint.left, rcPaint.top, SRCCOPY);
            ::SelectObject(hDcOffscreen, hOldBitmap);
            ::EndPaint(m_paintWnd, &ps);
            auto end = chrono::high_resolution_clock::now();
            cout<<"Draw Duration:"<<duration_cast<milliseconds>(end-start).count()<<"(ms)"<<endl;
            return true;
        }
        case WM_MOUSEACTIVATE:
        {
            if(m_bNoActivate){
                lRes = MA_NOACTIVATE;
                return true;
            }
            break;
        }
        case WM_MOUSEHOVER:
        {
            if (m_pRoot == nullptr) break;
            m_bMouseTracking = false;
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            UIControl* pHover = FindControl(pt);
            if (pHover == nullptr) break;
            // Generate mouse hover event
            if (m_pEventHover != nullptr) {
                TEventUI event = { 0 };
                event.Type = UIEVENT_MOUSEHOVER;
                event.pSender = m_pEventHover;
                event.wParam = wParam;
                event.lParam = lParam;
                event.dwTimestamp = ::GetTickCount();
                event.ptMouse = pt;
                event.wKeyState = MapKeyState();
                m_pEventHover->Event(event);
            }
            break;
        }
        case WM_MOUSELEAVE:
        {
            if( m_pRoot == nullptr ) break;
            if( m_bMouseTracking ) {
                POINT pt = { 0 };
                RECT rcWnd = { 0 };
                ::GetCursorPos(&pt);
                ::GetWindowRect(m_paintWnd, &rcWnd);
                if( !::IsIconic(m_paintWnd) && ::GetActiveWindow() == m_paintWnd && ::PtInRect(&rcWnd, pt) ) {
                    if( ::SendMessage(m_paintWnd, WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y)) == HTCLIENT ) {
                        ::ScreenToClient(m_paintWnd, &pt);
                        ::SendMessage(m_paintWnd, WM_MOUSEMOVE, 0, MAKELPARAM(pt.x, pt.y));
                    }
                    else
                        ::SendMessage(m_paintWnd, WM_MOUSEMOVE, 0, (LPARAM)-1);
                }
                else
                    ::SendMessage(m_paintWnd, WM_MOUSEMOVE, 0, (LPARAM)-1);
            }
            m_bMouseTracking = false;
            break;
        }
        case WM_MOUSEMOVE:
        {
            if( m_pRoot == nullptr ) break;
            // Start tracking this entire window again...
            if( !m_bMouseTracking ) {
                TRACKMOUSEEVENT tme = { 0 };
                tme.cbSize = sizeof(TRACKMOUSEEVENT);
                tme.dwFlags = TME_HOVER | TME_LEAVE;
                tme.hwndTrack = m_paintWnd;
                tme.dwHoverTime = m_iHoverTime;
                //tme.dwHoverTime = m_hwndTooltip == NULL ? m_iHoverTime : (DWORD) ::SendMessage(m_hwndTooltip, TTM_GETDELAYTIME, TTDT_INITIAL, 0L);
                _TrackMouseEvent(&tme);
                m_bMouseTracking = true;
            }
            // Generate the appropriate mouse messages
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            m_ptLastMousePos = pt;
            UIControl* pNewHover = FindControl(pt);
            if( pNewHover != nullptr && pNewHover->GetManager() != this ) break;
            TEventUI event = { 0 };
            event.ptMouse = pt;
            event.wParam = wParam;
            event.lParam = lParam;
            event.dwTimestamp = ::GetTickCount();
            event.wKeyState = MapKeyState();
            if( !IsCaptured() ) {
                pNewHover = FindControl(pt);
                if( pNewHover != nullptr && pNewHover->GetManager() != this ) break;
                if( pNewHover != m_pEventHover && m_pEventHover != nullptr ) {
                    event.Type = UIEVENT_MOUSELEAVE;
                    event.pSender = m_pEventHover;

                    UIPtrArray aNeedMouseLeaveNeeded(m_aNeedMouseLeaveNeeded.GetSize());
                    aNeedMouseLeaveNeeded.Resize(m_aNeedMouseLeaveNeeded.GetSize());
                    ::CopyMemory(aNeedMouseLeaveNeeded.GetData(), m_aNeedMouseLeaveNeeded.GetData(), m_aNeedMouseLeaveNeeded.GetSize() * sizeof(LPVOID));
                    for( int i = 0; i < aNeedMouseLeaveNeeded.GetSize(); i++ ) {
                        static_cast<UIControl*>(aNeedMouseLeaveNeeded[i])->Event(event);
                    }

                    m_pEventHover->Event(event);
                    m_pEventHover = nullptr;
                }
                if( pNewHover != m_pEventHover && pNewHover != nullptr ) {
                    event.Type = UIEVENT_MOUSEENTER;
                    event.pSender = pNewHover;
                    pNewHover->Event(event);
                    m_pEventHover = pNewHover;
                }
            }
            if( m_pEventClick != nullptr ) {
                event.Type = UIEVENT_MOUSEMOVE;
                event.pSender = m_pEventClick;
                m_pEventClick->Event(event);
            }
            else if( pNewHover != nullptr ) {
                event.Type = UIEVENT_MOUSEMOVE;
                event.pSender = pNewHover;
                pNewHover->Event(event);
            }
            break;
        }
        case WM_LBUTTONDOWN:
        {
// We alway set focus back to our app (this helps
            // when Win32 child windows are placed on the dialog
            // and we need to remove them on focus change).
            if (!m_bNoActivate) ::SetFocus(m_paintWnd);
            if( m_pRoot == nullptr ) break;
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            m_ptLastMousePos = pt;
            UIControl* pControl = FindControl(pt);
            if( pControl == nullptr ) break;
            if( pControl->GetManager() != this ) break;
            m_pEventClick = pControl;
            pControl->SetFocus();
            SetCapture();
            TEventUI event = { 0 };
            event.Type = UIEVENT_BUTTONDOWN;
            event.pSender = pControl;
            event.wParam = wParam;
            event.lParam = lParam;
            event.ptMouse = pt;
            event.wKeyState = (WORD)wParam;
            event.dwTimestamp = ::GetTickCount();
            pControl->Event(event);
            break;
        }
        case WM_LBUTTONDBLCLK:
        {
            if(!m_bNoActivate)::SetFocus(m_paintWnd);
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            m_ptLastMousePos = pt;
            UIControl* pControl = FindControl(pt);
            if( pControl == nullptr ) break;
            if( pControl->GetManager() != this ) break;
            SetCapture();
            TEventUI event = { 0 };
            event.Type = UIEVENT_DBLCLICK;
            event.pSender = pControl;
            event.ptMouse = pt;
            event.wKeyState = (WORD)wParam;
            event.dwTimestamp = ::GetTickCount();
            pControl->Event(event);
            m_pEventClick = pControl;
            break;
        }
        case WM_LBUTTONUP:
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            m_ptLastMousePos = pt;
            if( m_pEventClick == nullptr ) break;
            ReleaseCapture();
            TEventUI event = { 0 };
            event.Type = UIEVENT_BUTTONUP;
            event.pSender = m_pEventClick;
            event.wParam = wParam;
            event.lParam = lParam;
            event.ptMouse = pt;
            event.wKeyState = (WORD)wParam;
            event.dwTimestamp = ::GetTickCount();
            UIControl* pClick = m_pEventClick;
            m_pEventClick = nullptr;
            pClick->Event(event);
            break;
        }
        case WM_RBUTTONDOWN:
        {
            if (!m_bNoActivate) ::SetFocus(m_paintWnd);
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            m_ptLastMousePos = pt;
            UIControl* pControl = FindControl(pt);
            if( pControl == nullptr ) break;
            if( pControl->GetManager() != this ) break;
            pControl->SetFocus();
            TEventUI event = { 0 };
            event.Type = UIEVENT_RBUTTONDOWN;
            event.pSender = pControl;
            event.wParam = wParam;
            event.lParam = lParam;
            event.ptMouse = pt;
            event.wKeyState = (WORD)wParam;
            event.dwTimestamp = ::GetTickCount();
            pControl->Event(event);
            m_pEventClick = pControl;
            break;
        }
        case WM_MOUSEWHEEL:
        {
            if( m_pRoot == nullptr ) break;
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            ::ScreenToClient(m_paintWnd, &pt);
            m_ptLastMousePos = pt;
            UIControl* pControl = FindControl(pt);
            if( pControl == nullptr ) break;
            if( pControl->GetManager() != this ) break;
            int zDelta = (int) (short) HIWORD(wParam);
            TEventUI event = { 0 };
            event.Type = UIEVENT_SCROLLWHEEL;
            event.pSender = pControl;
            event.wParam = MAKELPARAM(zDelta < 0 ? SB_LINEDOWN : SB_LINEUP, 0);
            event.lParam = lParam;
            event.ptMouse = m_ptLastMousePos;
            event.wKeyState = MapKeyState();
            event.dwTimestamp = ::GetTickCount();
            pControl->Event(event);

            // Let's make sure that the scroll item below the cursor is the same as before...
            ::SendMessage(m_paintWnd, WM_MOUSEMOVE, 0, (LPARAM) MAKELPARAM(m_ptLastMousePos.x, m_ptLastMousePos.y));
            break;
        }
        case WM_TIMER:
        {
            for( int i = 0; i < m_aTimers.GetSize(); i++ ) {
                const TIMERINFO* pTimer = static_cast<TIMERINFO*>(m_aTimers[i]);
                if( pTimer->hWnd == m_paintWnd && pTimer->uTimerId == wParam && (!pTimer->killed) ) {
                    TEventUI event = { 0 };
                    event.Type = UIEVENT_TIMER;
                    event.pSender = pTimer->pSender;
                    event.dwTimestamp = ::GetTickCount();
                    event.ptMouse = m_ptLastMousePos;
                    event.wKeyState = MapKeyState();
                    event.wParam = pTimer->uTimerId;
                    event.lParam = lParam;
                    pTimer->pSender->Event(event);
                    break;
                }
            }
            break;
        }
        case WM_SETCURSOR:
        {
            if( m_pRoot == NULL ) break;
            if( LOWORD(lParam) != HTCLIENT ) break;
            if( m_bMouseCapture ) return true;

            POINT pt = { 0 };
            ::GetCursorPos(&pt);
            ::ScreenToClient(m_paintWnd, &pt);
            UIControl* pControl = FindControl(pt);
            if( pControl == NULL ) break;
            if( (pControl->GetControlFlags() & UIFLAG_SETCURSOR) == 0 ) break;
            TEventUI event = { 0 };
            event.Type = UIEVENT_SETCURSOR;
            event.pSender = pControl;
            event.wParam = wParam;
            event.lParam = lParam;
            event.ptMouse = pt;
            event.wKeyState = MapKeyState();
            event.dwTimestamp = ::GetTickCount();
            pControl->Event(event);
            return true;
        }
        case WM_CONTEXTMENU:
        {
            if( m_pRoot == nullptr ) break;
            if( IsCaptured() ) break;
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            ::ScreenToClient(m_paintWnd, &pt);
            m_ptLastMousePos = pt;
            if( m_pEventClick == nullptr ) break;
            TEventUI event = { 0 };
            event.Type = UIEVENT_CONTEXTMENU;
            event.pSender = m_pEventClick;
            event.ptMouse = pt;
            event.wKeyState = (WORD)wParam;
            event.lParam = (LPARAM)m_pEventClick;
            event.dwTimestamp = ::GetTickCount();
            m_pEventClick->Event(event);
            m_pEventClick = nullptr;
            break;
        }
        case WM_SIZE:
        {
            if(m_pRoot!=nullptr){
                m_pRoot->NeedUpdate();
            }
            return false;
        }
        case DUI_WM_KEYPRESS:
        {
            if(m_pFocus == nullptr){
                return false;
            }
            if(wParam == VK_ESCAPE ){
                TEventUI  event;
                event.pSender = m_pFocus;
                event.Type = UIEVENT_KILLFOCUS;
                event.dwTimestamp = GetTickCount();
                event.wKeyState = MapKeyState();
                event.wParam = wParam;
                event.lParam = lParam;
                m_pFocus->DoEvent(event);
                m_pFocus = nullptr;
                return true;
            }
            TEventUI    event;
            event.pSender = m_pFocus;
            event.Type = UIEVENT_KEYDOWN;
            event.dwTimestamp = GetTickCount();
            event.wKeyState = MapKeyState();
            event.wParam = wParam;
            event.lParam = lParam;
            m_pFocus->DoEvent(event);
            return true;
        }
        case WM_CHAR:
        {
            if(m_pFocus == nullptr){
                return false;
            }
            TEventUI    event;
            event.pSender = m_pFocus;
            event.Type = UIEVENT_CHAR;
            event.dwTimestamp = GetTickCount();
            event.wKeyState = MapKeyState();
            event.wParam = wParam;
            event.lParam = lParam;
            m_pFocus->DoEvent(event);
            return true;
        }
        default:
            break;
    }
    return false;
}

static uint32_t  glbTimerId = 0x100;

uint32_t UIPaintManager::SetTimer(UIControl *pControl, uint32_t uElapse) {
    assert(pControl!=nullptr);
    assert(uElapse>0);
    /*for( int i = 0; i< m_aTimers.GetSize(); i++ ) {
        auto* pTimer = static_cast<TIMERINFO*>(m_aTimers[i]);
        if( pTimer->pSender == pControl
            && pTimer->hWnd == m_paintWnd) {
            if(pTimer->killed) {
                if( ::SetTimer(m_paintWnd, pTimer->uTimerId, uElapse, nullptr) ) {
                    pTimer->killed = false;
                    return pTimer->uTimerId;
                }
            }
        }
    }*/
    uint32_t nTimerID = glbTimerId++;
    if( !::SetTimer(m_paintWnd, nTimerID, uElapse, nullptr) ) return false;
    auto* pTimer = new TIMERINFO;
    pTimer->hWnd = m_paintWnd;
    pTimer->pSender = pControl;
    pTimer->uTimerId = nTimerID;
    pTimer->killed = false;
    m_aTimers.Add(pTimer);
    return pTimer->uTimerId;
}

bool UIPaintManager::KillTimer(UIControl *pControl, uint32_t nTimerID) {
    assert(pControl!=nullptr);
    for( int i = 0; i< m_aTimers.GetSize(); i++ ) {
        auto* pTimer = static_cast<TIMERINFO*>(m_aTimers[i]);
        if( pTimer->pSender == pControl
            && pTimer->hWnd == m_paintWnd
            && pTimer->uTimerId == nTimerID )
        {
            ::KillTimer(pTimer->hWnd, pTimer->uTimerId);
            delete pTimer;
            m_aTimers.Remove(i);
            break;
            /*if(!pTimer->killed) {
                if( ::IsWindow(m_paintWnd) ) ::KillTimer(pTimer->hWnd, pTimer->uTimerId);
                pTimer->killed = true;
                return true;
            }*/
        }
    }
    return false;
}

void UIPaintManager::KillTimer(UIControl *pControl) {
    assert(pControl!=nullptr);
    int count = m_aTimers.GetSize();
    for( int i = 0, j = 0; i < count; i++ ) {
        auto* pTimer = static_cast<TIMERINFO*>(m_aTimers[i - j]);
        if( pTimer->pSender == pControl && pTimer->hWnd == m_paintWnd ) {
            if(!pTimer->killed) ::KillTimer(pTimer->hWnd, pTimer->uTimerId);
            delete pTimer;
            m_aTimers.Remove(i - j);
            j++;
        }
    }
}

void UIPaintManager::RemoveAllTimers() {
    for( int i = 0; i < m_aTimers.GetSize(); i++ ) {
        auto* pTimer = static_cast<TIMERINFO*>(m_aTimers[i]);
        if( pTimer->hWnd == m_paintWnd ) {
            if(!pTimer->killed) {
                if( ::IsWindow(m_paintWnd) ) ::KillTimer(m_paintWnd, pTimer->uTimerId);
            }
            delete pTimer;
        }
    }
    m_aTimers.Empty();
}

void UIPaintManager::SetInitSize(int cx, int cy) {
    m_szInitWindowSize.cx = cx;
    m_szInitWindowSize.cy = cy;
    if( m_pRoot == nullptr && m_paintWnd != nullptr ) {
        ::SetWindowPos(m_paintWnd, nullptr, 0, 0, cx, cy, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
    }
}