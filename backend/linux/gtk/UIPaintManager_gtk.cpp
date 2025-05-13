#include "UIPaintManager.h"
#include <gtk/gtk.h>
#include "UIBaseWindow.h"
#include "UIRect.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <chrono>

using namespace std;
using namespace chrono;

typedef struct tagTIMERINFO
{
    UIControl* pSender;
    uint32_t   uTimerId;
    bool        killed;
    UIPaintManager* paintManager;
} TIMERINFO;

static gboolean wrap_timer_event(gpointer userdata)
{
    auto  *pTimer = (TIMERINFO *)userdata;
    g_print("timer event\n");
    long lRes = 0;
    pTimer->paintManager->MessageHandler(DUI_WM_TIMER, (WPARAM)pTimer->pSender,
                                         (LPARAM)(long)pTimer->uTimerId, lRes);
    return true;
}

struct UIPaintManagerInternalImp
{
    cairo_t     *m_paintDC;
};


static uint32_t MapKeyState(uint32_t state)
{
    uint32_t retState = 0;
    if(state & GDK_CONTROL_MASK){
        retState |= MK_CONTROL;
    }
    if(state & GDK_BUTTON1_MASK){
        retState |= MK_LBUTTON;
    }
    if(state & GDK_BUTTON2_MASK){
        retState |= MK_MBUTTON;
    }
    if(state & GDK_BUTTON3_MASK){
        retState |= MK_RBUTTON;
    }
    if(state & GDK_SHIFT_MASK){
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

void UIPaintManager::MessageLoop() {
    gtk_main();
}

void UIPaintManager::Invalidate(RECT &rcItem) {
    if(m_paintWnd==nullptr || (!GDK_IS_WINDOW(gtk_widget_get_window(m_paintWnd)))){
        return;
    }
    GdkWindow *gdkWindow;
    GdkRectangle gdkRect;
    gdkWindow = gtk_widget_get_window(m_paintWnd);
    gdkRect.x = (int)rcItem.left;
    gdkRect.y = (int)rcItem.top;
    gdkRect.width = (int)rcItem.right - (int)rcItem.left;
    gdkRect.height = (int)rcItem.bottom - (int)rcItem.top;
    gdk_window_invalidate_rect(gdkWindow, &gdkRect, FALSE);
}

HANDLE_DC UIPaintManager::GetPaintDC() {
    return m_impl->m_paintDC;
}

bool UIPaintManager::MessageHandler(uint32_t uMsg, WPARAM wParam, LPARAM lParam, long &lRes) {
    if( m_paintWnd == nullptr ) return false;
    switch(uMsg){
        case DUI_WM_PAINT:
        {
            auto *cr = (cairo_t*)wParam;
            m_impl->m_paintDC = cr;
            if(m_bUpdateNeeded)
            {
                m_bUpdateNeeded = false;
                GtkAllocation Allocation;
                gtk_widget_get_allocation(this->GetPaintWindow(), &Allocation);
                if (Allocation.width <= 0 || Allocation.height <= 0) {
                    g_critical("allocation is empty");
                } else {

                    //
                    // check is root control need update pos
                    //

                    if(m_pRoot->IsUpdateNeeded()){
                        UIRect clientRect(Allocation.x, Allocation.y, Allocation.x + Allocation.width,
                                          Allocation.y + Allocation.height);
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

                    //
                    // generate windows init notify
                    //

                }
            }

            static bool bInit = false;
            if(!bInit){
                bInit = true;
                SendNotify(m_pRoot, DUI_MSGTYPE_WINDOWINIT, 0,0, false);
            }

            //
            // Is here has a rect for draw.
            //
            GdkRectangle Rect;

            if(gdk_cairo_get_clip_rectangle(cr, &Rect)){

                //
                // render control
                //

                RECT paintRect{0};
                paintRect.left = Rect.x;
                paintRect.top = Rect.y;
                paintRect.right = Rect.x + Rect.width;
                paintRect.bottom = Rect.y + Rect.height;
                m_pRoot->Paint(cr, paintRect);
            }
            lRes = 1;
            return true;

        }
        case DUI_WM_SIZE:
            if(m_pRoot!=nullptr){
                m_pRoot->NeedUpdate();
            }
            return false;
        case DUI_WM_KEYPRESS:
        {
            auto *eventKey = (GdkEventKey*)(wParam);
            if(m_pFocus == nullptr){
                return false;
            }
            if(eventKey->keyval == VK_ESCAPE ){
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
            auto *Event = (GdkEventButton*)wParam;
            int Type;

            if (Event->type == GDK_BUTTON_PRESS){
                if (Event->button == 1){
                    Type = UIEVENT_BUTTONDOWN;
                }else if (Event->button == 3){
                    Type = UIEVENT_RBUTTONDOWN;
                }
            }else if (Event->type == GDK_2BUTTON_PRESS && Event->button == 1){
                Type = UIEVENT_DBLCLICK;
            }else{
                break;
            }

            POINT pt = {(long)Event->x, (long)Event->y};
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
                SendEvent.dwTimestamp = Event->time;
                pControl->DoEvent(SendEvent);
            }
            return true;
        }
        case DUI_WM_MOUSERELEASE:
        {
            auto *Event = (GdkEventButton*)wParam;
            POINT pt = {(long)Event->x, (long)Event->y};

            //
            // set last mouse position
            //

            m_ptLastMousePos = pt;

            if (Event->type == GDK_BUTTON_RELEASE && Event->button == 1){
                if( m_pEventClick == nullptr ) break;
                //ReleaseCapture();
                TEventUI event = { 0 };
                event.Type = UIEVENT_BUTTONUP;
                event.pSender = m_pEventClick;
                event.wParam = wParam;
                event.lParam = lParam;
                event.ptMouse = pt;
                event.dwTimestamp = Event->time;
                UIControl* pClick = m_pEventClick;
                m_pEventClick = nullptr;
                pClick->Event(event);
            }
            return true;
        }
        case DUI_WM_MOUSEMOVE:
        {
            auto *Event = (GdkEventMotion *)wParam;
            POINT pt = {(long)Event->x, (long)Event->y};

            m_ptLastMousePos = pt;
            UIControl* pNewHover = FindControl(pt);
            if(pNewHover == nullptr)
                break;

            TEventUI SendEvent;
            memset(&SendEvent, 0, sizeof(TEventUI));
            SendEvent.ptMouse = pt;
            SendEvent.dwTimestamp = Event->time;
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
                auto *Event = (GdkEventCrossing *)wParam;
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
            auto *Event = (GdkEventScroll *)wParam;
            POINT pt = {(long)Event->x, (long)Event->y};
            m_ptLastMousePos = pt;
            UIControl* pControl = FindControl(pt);
            if(pControl){
                TEventUI SendEvent;
                memset(&SendEvent, 0, sizeof(TEventUI));

                SendEvent.Type = UIEVENT_SCROLLWHEEL;
                SendEvent.pSender = pControl;
                //SendEvent.wParam = MAKELPARAM(Event->direction == GDK_SCROLL_DOWN ? SB_LINEDOWN : SB_LINEUP, 0);
                if(Event->direction == GDK_SCROLL_DOWN){
                    SendEvent.wParam = (WPARAM)SB_LINEDOWN;
                }else if(Event->direction == GDK_SCROLL_UP){
                    SendEvent.wParam = (WPARAM)SB_LINEUP;
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

    auto* pTimer = new TIMERINFO;
    memset(pTimer, 0, sizeof(*pTimer));

    pTimer->paintManager = this;
    pTimer->killed = false;
    pTimer->pSender = pControl;
    pTimer->uTimerId = g_timeout_add((guint)uElapse, wrap_timer_event, pTimer);

    m_aTimers.Add(pTimer);
    return pTimer->uTimerId;
}

bool UIPaintManager::KillTimer(UIControl *pControl, uint32_t nTimerID) {
    assert(pControl != nullptr);
    for(int i = 0; i< m_aTimers.GetSize(); i++){
        auto* pTimer = static_cast<TIMERINFO*>(m_aTimers[i]);
        if(pTimer->pSender == pControl && pTimer->uTimerId == nTimerID){
            g_source_remove(pTimer->uTimerId);
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
                g_source_remove(pTimer->uTimerId);
            delete pTimer;
            m_aTimers.Remove(i - j);
            j++;
        }
    }
}

void UIPaintManager::RemoveAllTimers() {
    for(int i = 0; i < m_aTimers.GetSize(); i++){
        auto* pTimer = static_cast<TIMERINFO*>(m_aTimers[i]);
        if(!pTimer->killed) {
            g_source_remove(pTimer->uTimerId);
        }
        delete pTimer;
    }
    m_aTimers.Empty();
}

void UIPaintManager::SetInitSize(int cx, int cy) {
    m_szInitWindowSize.cx = cx;
    m_szInitWindowSize.cy = cy;
    gtk_window_set_default_size(GTK_WINDOW(m_paintWnd),cx,cy);
    //gtk_window_set_default_size(GTK_WINDOW(m_paintWnd),cx,cy);
}
