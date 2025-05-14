#include <cassert>
#include <UIPaintManager.h>
#include <UIRect.h>
#include "TimerContext.h"
#include "DispatchMessage.h"
#include "DisplayInstance.h"
#include <UIBaseWindowObjects.h>
#include "X11HDC.h"
#include <poll.h>

#include "X11Window.h"

struct UIPaintManagerInternalImp
{

};

static uint32_t MapKeyState(uint32_t state)
{
    uint32_t retState = 0;
    if(state & ControlMask){
        retState |= MK_CONTROL;
    }
    if(state & Button1Mask){
        retState |= MK_LBUTTON;
    }
    if(state & Button2Mask){
        retState |= MK_MBUTTON;
    }
    if(state & Button3Mask){
        retState |= MK_RBUTTON;
    }
    if(state & ShiftMask){
        retState |= MK_SHIFT;
    }
    return retState;
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
}

bool glbContinueRunning = true;

void UIPaintManager::MessageLoop() {

    XEvent event;
    Display *display = DisplayInstance::GetInstance().GetDisplay();
    if (display == nullptr) {
        fprintf(stderr,"Can't Open Display");
        return;
    }
    struct pollfd fd = {
        .fd = ConnectionNumber(display),
        .events =  POLLIN
    };
    while (glbContinueRunning) {
        bool ret = XPending(display)> 0 || poll(&fd,1,TimerContext::GetInstance().GetMinimumTimeout())>0;
        if (!ret) {
            //timeout
            TimerContext::GetInstance().ProcessTimeout();
            continue;
        }
        //有XEvent事件的时候，也有可能正好超时事件发生。
        TimerContext::GetInstance().ProcessTimeout();
        XNextEvent(display, &event);
        if(XFilterEvent(&event,None)){
            continue;
        }
        DispatchMessage(event);
        if (event.type == DestroyNotify) {
            if(UIBaseWindowObjects::GetInstance().GetWindowCount()==0 && m_quitOnLastWindowDestroy){
                glbContinueRunning = false;
            }
        }
    }
}

void    UIAppQuitX11() {
    glbContinueRunning = false;
}

void UIPaintManager::Invalidate(RECT &rcItem) {
    UIRect uiRect{rcItem};
    if(uiRect.IsEmpty() || m_paintWnd == nullptr){
        return;
    }
    X11Window  *window = m_paintWnd;
    if (window->window==0) {
        return;
    }
    XEvent event;
    event.type = Expose;
    event.xexpose.window = window->window;
    event.xexpose.x = rcItem.left;
    event.xexpose.y = rcItem.top;
    event.xexpose.width = rcItem.right-rcItem.left;
    event.xexpose.height = rcItem.bottom-rcItem.top;
    event.xexpose.count = 0;

    XSendEvent(window->display, window->window, False, ExposureMask, &event);
}

HANDLE_DC UIPaintManager::GetPaintDC() {
    return m_paintWnd->hdc;
}

static unsigned long LastClickTime = 0;
static const int DOUBLE_CLICK_INTERVAL = 300;

bool UIPaintManager::MessageHandler(uint32_t uMsg, WPARAM wParam, LPARAM lParam, long &lRes) {
    if( m_paintWnd == nullptr ) return false;
    switch(uMsg){
        case DUI_WM_PAINT:
        {
            if (m_paintWnd->window==0) {
                return true;
            }
            if(m_bUpdateNeeded)
            {
                m_bUpdateNeeded = false;
                if(m_pRoot->IsUpdateNeeded()){
                    UIRect clientRect {0,0,m_paintWnd->width,m_paintWnd->height};
                    m_pRoot->SetPos(clientRect);
                }else{
                    UIControl* pControl = nullptr;
                    m_aFoundControls.Empty();
                    m_pRoot->FindControl(__FindControlsFromUpdate, nullptr, UIFIND_VISIBLE | UIFIND_ME_FIRST | UIFIND_UPDATETEST);
                    for( int it = 0; it < m_aFoundControls.GetSize(); it++ ) {
                        pControl = static_cast<UIControl*>(m_aFoundControls[it]);
                        if( !pControl->IsFloat() ) pControl->SetPos(pControl->GetPos(), true);
                        else pControl->SetPos(pControl->GetRelativePos(), true);
                    }
                }
            }

            static bool bInit = false;
            if(!bInit){
                bInit = true;
                SendNotify(m_pRoot, DUI_MSGTYPE_WINDOWINIT, 0,0, false);
            }

            //
            // Is here has a rect for draw.
            XExposeEvent *event = static_cast<XExposeEvent *>(wParam);
            if(event != nullptr){
                UIRect rect {event->x,event->y,event->x + event->width, event->y + event->height};
                if(!rect.IsEmpty()){
                    HANDLE_DC hdc = CreateHDC(m_paintWnd,m_paintWnd->window,m_paintWnd->width,m_paintWnd->height);
                    m_pRoot->Paint(hdc,rect);
                    XCopyArea(m_paintWnd->display,hdc->drawablePixmap,m_paintWnd->window,m_paintWnd->hdc->gc,rect.left,rect.top,
                              rect.right-rect.left,rect.bottom-rect.top,rect.left,rect.top);
                    ::ReleaseHDC(hdc);
                }
            }
            lRes = 1;
            return true;

        }
        case DUI_WM_SIZE:
        {
            XConfigureEvent *configureEvent = (XConfigureEvent*)wParam;
            if(m_pRoot!=nullptr){
                m_pRoot->NeedUpdate();
            }
            m_paintWnd->x = configureEvent->x;
            m_paintWnd->y = configureEvent->y;
            m_paintWnd->width = configureEvent->width;
            m_paintWnd->height = configureEvent->height;
            return false;
        }
        case DUI_WM_KEYPRESS:
        {
            auto *eventKey = (XKeyEvent *)(wParam);
            if(m_pFocus == nullptr || eventKey== nullptr){
                return false;
            }
            KeySym keysym = XLookupKeysym(eventKey,0);
            if(keysym == VK_ESCAPE ){
                TEventUI  event;
                event.pSender = m_pFocus;
                event.Type = UIEVENT_KILLFOCUS;
                event.dwTimestamp = UIGetTickCount();
                event.wKeyState = MapKeyState(eventKey->state);
                event.wParam = wParam;
                event.lParam = lParam;
                m_pFocus->DoEvent(event);
                m_pFocus = nullptr;
                return true;
            }
            TEventUI    event;
            event.pSender = m_pFocus;
            event.Type = UIEVENT_KEYDOWN;
            event.dwTimestamp = UIGetTickCount();
            event.wKeyState = MapKeyState(eventKey->state);
            event.wParam = wParam;
            event.lParam = lParam;
            m_pFocus->DoEvent(event);
            return true;
        }
        case DUI_WM_MOUSEPRESS:
        {
            auto *event = (XButtonEvent *)wParam;
            int Type;
            if(event->button == Button1){
                Type = UIEVENT_BUTTONDOWN;
                if (event->time - LastClickTime < DOUBLE_CLICK_INTERVAL) {
                    LastClickTime = 0;
                    Type = UIEVENT_DBLCLICK;
                }else {
                    LastClickTime = event->time;
                }
            }else if(event->button == Button3){
                Type = UIEVENT_RBUTTONDOWN;
            }else{
                break;
            }

            POINT pt = {(long)event->x, (long)event->y};
            m_ptLastMousePos = pt;
            UIControl *pControl = FindControl(pt);
            if (pControl){
                m_pEventClick = pControl;

                pControl->SetFocus();
                TEventUI SendEvent;
                memset(&SendEvent, 0, sizeof(TEventUI));

                SendEvent.Type = Type;
                SendEvent.pSender = pControl;
                SendEvent.wParam = wParam;
                SendEvent.lParam = lParam;
                SendEvent.ptMouse = pt;
                SendEvent.dwTimestamp = event->time;
                pControl->DoEvent(SendEvent);
            }
            return true;
        }
        case DUI_WM_MOUSERELEASE:
        {
            auto *buttonEvent = (XButtonEvent *)wParam;
            POINT pt = {(long)buttonEvent->x, (long)buttonEvent->y};

            //
            // set last mouse position
            //

            m_ptLastMousePos = pt;

            if (buttonEvent->type == ButtonRelease && buttonEvent->button == Button1){
                if( m_pEventClick == nullptr ) break;
                //ReleaseCapture();
                TEventUI event = { 0 };
                event.Type = UIEVENT_BUTTONUP;
                event.pSender = m_pEventClick;
                event.wParam = wParam;
                event.lParam = lParam;
                event.ptMouse = pt;
                event.dwTimestamp = buttonEvent->time;
                UIControl* pClick = m_pEventClick;
                m_pEventClick = nullptr;
                pClick->Event(event);
            }
            return true;
        }
        case DUI_WM_MOUSEMOVE:
        {
            auto *motionEvent = (XMotionEvent *)wParam;
            POINT pt = {(long)motionEvent->x, (long)motionEvent->y};

            m_ptLastMousePos = pt;
            UIControl* pNewHover = FindControl(pt);
            if(pNewHover == nullptr)
                break;

            TEventUI SendEvent;
            memset(&SendEvent, 0, sizeof(TEventUI));
            SendEvent.ptMouse = pt;
            SendEvent.dwTimestamp = motionEvent->time;
            SendEvent.Type = UIEVENT_SETCURSOR;
            SendEvent.pSender = pNewHover;
            pNewHover->DoEvent(SendEvent);

            if(pNewHover != m_pEventHover && m_pEventHover != nullptr){
                SendEvent.Type = UIEVENT_MOUSELEAVE;
                SendEvent.pSender = m_pEventHover;
                m_pEventHover->DoEvent(SendEvent);
                m_pEventHover = nullptr;
            }
            if(pNewHover != m_pEventHover){
                SendEvent.Type = UIEVENT_MOUSEENTER;
                SendEvent.pSender = pNewHover;
                pNewHover->DoEvent(SendEvent);
                m_pEventHover = pNewHover;
            }
            if(m_pEventClick != nullptr){
                SendEvent.Type = UIEVENT_MOUSEMOVE;
                SendEvent.pSender = m_pEventClick;
                m_pEventClick->DoEvent(SendEvent);
            }else{
                SendEvent.Type = UIEVENT_MOUSEMOVE;
                SendEvent.pSender = pNewHover;
                pNewHover->DoEvent(SendEvent);
            }
            return true;
        }
        case DUI_WM_MOUSELEAVE:
        {
            if(m_pEventHover){
                auto *Event = (XLeaveWindowEvent *)wParam;
                TEventUI SendEvent;
                memset(&SendEvent, 0, sizeof(TEventUI));

                POINT pt = {(long)Event->x, (long)Event->y};
                SendEvent.ptMouse = pt;
                SendEvent.dwTimestamp = Event->time;
                SendEvent.Type = UIEVENT_MOUSELEAVE;
                SendEvent.pSender = m_pEventHover;
                m_pEventHover->DoEvent(SendEvent);
                m_pEventHover = nullptr;
            }
            return true;
        }
        case DUI_WM_MOUSEWHEEL:
        {
            auto *Event = (XButtonEvent *)wParam;
            POINT pt = {(long)Event->x, (long)Event->y};
            m_ptLastMousePos = pt;
            UIControl* pControl = FindControl(pt);
            if(pControl){
                TEventUI SendEvent;
                memset(&SendEvent, 0, sizeof(TEventUI));

                SendEvent.Type = UIEVENT_SCROLLWHEEL;
                SendEvent.pSender = pControl;
                if(Event->button == Button4){
                    SendEvent.wParam = (WPARAM)SB_LINEUP;
                }else if(Event->button == Button5){
                    SendEvent.wParam = (WPARAM)SB_LINEDOWN;
                }else{
                    return true;
                }
                SendEvent.lParam = lParam;
                SendEvent.wKeyState = MapKeyState(Event->state);
                SendEvent.dwTimestamp = Event->time;
                pControl->DoEvent(SendEvent);
            }
            return true;
        }
        case DUI_WM_TIMER:
        {
            TEventUI sendEvent;
            memset(&sendEvent, 0, sizeof(TEventUI));
            sendEvent.Type = UIEVENT_TIMER;
            sendEvent.pSender = (UIControl*)wParam;
            sendEvent.wParam = (LPARAM)lParam;
            sendEvent.pSender->DoEvent(sendEvent);
            return false;
        }
        default:
            break;
    }
    return false;
}

uint32_t UIPaintManager::SetTimer(UIControl *pControl, uint32_t uElapse) {
    assert(pControl != nullptr);
    assert(uElapse > 0);

    TIMERINFO *timerinfo = TimerContext::GetInstance().AddTimer(pControl,this,uElapse);
    if (timerinfo == nullptr) {
        return 0;
    }
    m_aTimers.Add(timerinfo);
    return timerinfo->uTimerId;
}

bool UIPaintManager::KillTimer(UIControl *pControl, uint32_t nTimerID) {
    assert(pControl != nullptr);
    for(int i = 0; i< m_aTimers.GetSize(); i++){
        auto* pTimer = static_cast<TIMERINFO*>(m_aTimers[i]);
        if(pTimer->pSender == pControl && pTimer->uTimerId == nTimerID){
            TimerContext::GetInstance().Remove(pTimer->uTimerId);
            pTimer->uTimerId = 0;
            pTimer->killed = true;
            delete pTimer;
            m_aTimers.Remove(i);
            break;
        }
    }
    return true;
}

void UIPaintManager::KillTimer(UIControl *pControl) {
    assert(pControl != nullptr);
    int count = m_aTimers.GetSize();
    for(int i = 0, j = 0; i < count; i++){
        auto* pTimer = static_cast<TIMERINFO*>(m_aTimers[i - j]);
        if(pTimer->pSender == pControl){
            if(!pTimer->killed)
                TimerContext::GetInstance().Remove(pTimer->uTimerId);
            delete pTimer;
            m_aTimers.Remove(i - j);
            j++;
        }
    }
}

void UIPaintManager::RemoveAllTimers() {
    TimerContext::GetInstance().RemoveAllTimers();
    for(int i = 0; i < m_aTimers.GetSize(); i++){
        auto* pTimer = static_cast<TIMERINFO*>(m_aTimers[i]);
        delete pTimer;
    }
    m_aTimers.Empty();
}

void UIPaintManager::SetCapture() {
}

void UIPaintManager::ReleaseCapture() {
}

void UIPaintManager::SetInitSize(int cx, int cy) {
    m_szInitWindowSize.cx = cx;
    m_szInitWindowSize.cy = cy;
    m_paintWnd->width = cx;
    m_paintWnd->height = cy;
    XResizeWindow(m_paintWnd->display,m_paintWnd->window,cx,cy);
}