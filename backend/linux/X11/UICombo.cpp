#include <UICombo.h>
#include <UIRect.h>

#include "DisplayInstance.h"
#include "../../src/UIComboWnd.h"

void get_workarea(Display *display,UIRect *workArea) {
    // 获取根窗口
    Window root = DefaultRootWindow(display);

    // 获取支持_NET_WORKAREA属性的原子
    Atom net_workarea = XInternAtom(display, "_NET_WORKAREA", False);

    // 提出请求以获取_NET_WORKAREA属性值
    Atom actual_type;
    int actual_format;
    unsigned long num_items, bytes_left;
    long *workarea_data;

    Status status = XGetWindowProperty(display, root, net_workarea, 0L, 4L, False,
                                       XA_CARDINAL, &actual_type, &actual_format,
                                       &num_items, &bytes_left, (unsigned char **)&workarea_data);

    if (status == Success && actual_type == XA_CARDINAL && num_items >= 4) {
        // workarea_data 包含 x, y, width, height 四个值
        workArea->left = workarea_data[0];
        workArea->top = workarea_data[1];
        workArea->right = workarea_data[0] + workarea_data[2];
        workArea->bottom = workarea_data[1] + workarea_data[3];
    }

    if (workarea_data) {
        XFree(workarea_data); // 记得释放内存
    }
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

    X11Window_s *wnd = pOwner->GetManager()->GetPaintWindow();
    rc.left += wnd->x;
    rc.right += wnd->x;
    rc.top += wnd->y;
    rc.bottom += wnd->y;

    UIRect rcWork{0,0,0,0};
    get_workarea(wnd->display,&rcWork);
    if( rc.bottom > rcWork.bottom ) {
        rc.left = rcOwner.left;
        rc.right = rcOwner.right;
        if( szDrop.cx > 0 ) rc.right = rc.left + szDrop.cx;
        rc.top = rcOwner.top - MIN((long)cyFixed, szDrop.cy);
        rc.bottom = rcOwner.top;
    }

    Create(pOwner->GetManager()->GetPaintWindow(), UIString{"ComboWnd"}, UI_WNDSTYLE_CHILD, 0, rc);
    // Disable window decorate,we will create a window without title bar and border
    X11Window *currentWindow = this->GetWND();
    Atom wm_state = XInternAtom(currentWindow->display, "_NET_WM_STATE", False);
    Atom wm_state_skip_taskbar = XInternAtom(currentWindow->display, "_NET_WM_STATE_SKIP_TASKBAR", False);
    Atom wm_state_skip_pager = XInternAtom(currentWindow->display, "_NET_WM_STATE_SKIP_PAGER", False);
    XChangeProperty(currentWindow->display, currentWindow->window, wm_state, XA_ATOM, 32, PropModeReplace,
                    (unsigned char *)&wm_state_skip_taskbar, 1);
    XChangeProperty(currentWindow->display, currentWindow->window, wm_state, XA_ATOM, 32, PropModeAppend,
                    (unsigned char *)&wm_state_skip_pager, 1);
    this->ShowWindow();
}

long UIComboWnd::HandleMessage_Internal(uint32_t uMsg, WPARAM wParam, LPARAM lParam) {
    if( uMsg == DUI_WM_MOUSEPRESS ) {
        auto *Event = (XButtonEvent*)wParam;

        POINT pt = {(long)Event->x, (long)Event->y};
        UIControl* pControl = m_pm.FindControl(pt);
        if( pControl && pControl->GetClass() == DUI_CTR_SCROLLBAR ) {
            m_scrollbarClicked = true;
        }
    }
    else if( uMsg == DUI_WM_MOUSERELEASE ) {
        if (m_scrollbarClicked) {
            m_scrollbarClicked = false;
        }
        else {
            auto *Event = (XButtonEvent*)wParam;
            POINT pt = {(long)Event->x, (long)Event->y};
            UIControl* pControl = m_pm.FindControl(pt);
            if( pControl && pControl->GetClass() != DUI_CTR_SCROLLBAR ) {
                //TODO  Send Focus out event
                //printf("Button Release Close Window..........\n");
                //this->Close();
            }
        }
    }
    else if( uMsg == DUI_WM_KEYPRESS ) {
        auto  *keyEvent = (XKeyEvent*)(wParam);
        KeySym keysym = XLookupKeysym(keyEvent,0);
        switch( keysym ) {
            case VK_ESCAPE:
                m_pOwner->SelectItem(m_oldSel, true);
                EnsureVisible(m_oldSel);
                break;
            case VK_RETURN:
                this->Close();
                break;

            default:
                TEventUI event;
                event.Type = UIEVENT_KEYDOWN;
                event.chKey = (uint16_t)keyEvent->keycode;
                m_pOwner->DoEvent(event);
                EnsureVisible(m_pOwner->GetCurSel());
                return 0;
        }
    }
    else if( uMsg == DUI_WM_MOUSEWHEEL ) {
        auto *EventScroll = (XButtonEvent *)wParam;
        //int zDelta = (int) (short) HIWORD(wParam);
        TEventUI event = { 0 };
        event.Type = UIEVENT_SCROLLWHEEL;
        if(EventScroll->button == Button4){
            event.wParam = (WPARAM)SB_LINEUP;
        }else if(EventScroll->button == Button5){
            event.wParam = (WPARAM)SB_LINEDOWN;
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
        printf("This Close Combo Wnd......\n");
        this->Close();
        //TODO delete EVENT?
    }
    return 0;
}