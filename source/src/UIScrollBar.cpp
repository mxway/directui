#include <UIScrollBar.h>
#include <UIContainer.h>
#include <UIPaintManager.h>
#include <UIRenderEngine.h>
#include <cstring>
#include "UIRect.h"
#include <iostream>

using namespace std;

UIScrollBar::UIScrollBar()
    :m_timerId{0},
    m_horizontal{false},
    m_range{100},
    m_scrollPos{0},
    m_lineSize{SCROLLBAR_LINESIZE},
    m_scrollUnit{1},
    m_owner { nullptr },
    m_lastScrollPos{0},
    m_lastScrollOffset{0},
    m_scrollRepeatDelay{0},
    m_button1State{0},
    m_button1Color{0},
    m_button2State{0},
    m_button2Color{0},
    m_thumbColor{0},
    m_thumbState{0},
    m_showButton1{true},
    m_showButton2{true}
{
    m_cxyFixed.cx = DEFAULT_SCROLLBAR_SIZE;
    ptLastMouse.x = ptLastMouse.y = 0;
    memset(&m_rcThumb, 0, sizeof(m_rcThumb));
    memset(&m_rcButton1, 0, sizeof(m_rcButton1));
    memset(&m_rcButton2, 0, sizeof(m_rcButton2));
}

UIString UIScrollBar::GetClass() const {
    return UIString{DUI_CTR_SCROLLBAR};
}

LPVOID UIScrollBar::GetInterface(const UIString &name) {
    if( name==DUI_CTR_SCROLLBAR) return static_cast<UIScrollBar*>(this);
    return UIControl::GetInterface(name);
}

UIContainer *UIScrollBar::GetOwner() const {
    return m_owner;
}

void UIScrollBar::SetOwner(UIContainer *owner) {
    m_owner = owner;
}

void UIScrollBar::SetVisible(bool visible) {
    if( m_visible == visible ) return;

    bool v = IsVisible();
    m_visible = visible;
    if( m_focused ) m_focused = false;
}

void UIScrollBar::SetEnabled(bool enabled) {
    UIControl::SetEnabled(enabled);
    if(!IsEnabled()){
        m_button1State = 0;
        m_button2State = 0;
        m_thumbState = 0;
    }
}

void UIScrollBar::SetFocus() {
    if(m_owner != nullptr){
        m_owner->SetFocus();
    }else{
        UIControl::SetFocus();
    }
}

bool UIScrollBar::IsHorizontal() {
    return m_horizontal;
}

void UIScrollBar::SetHorizontal(bool horizontal) {
    if( m_horizontal == horizontal ) return;

    m_horizontal = horizontal;
    if( m_horizontal ) {
        if( m_cxyFixed.cy == 0 ) {
            m_cxyFixed.cx = 0;
            m_cxyFixed.cy = DEFAULT_SCROLLBAR_SIZE;
        }
    }
    else {
        if( m_cxyFixed.cx == 0 ) {
            m_cxyFixed.cx = DEFAULT_SCROLLBAR_SIZE;
            m_cxyFixed.cy = 0;
        }
    }

    if( m_owner != NULL ) {
        m_owner->NeedUpdate();
    }else{
        NeedParentUpdate();
    }
}

int UIScrollBar::GetScrollRange() const {
    return m_range;
}

void UIScrollBar::SetScrollRange(int range) {
    if( m_range == range ) return;

    m_range = range;
    if( m_range < 0 ) m_range = 0;
    if( m_scrollPos > m_range ) m_scrollPos = m_range;
    SetPos(m_rcItem, true);
}

int UIScrollBar::GetScrollPos() const {
    return m_scrollPos;
}

void UIScrollBar::SetScrollPos(int pos, bool triggerEvent) {
    if( m_scrollPos == pos ) return;

    int iOldScrollPos = m_scrollPos;
    m_scrollPos = pos;
    if( m_scrollPos < 0 ) m_scrollPos = 0;
    if( m_scrollUnit > 1 ) {
        int iLeftOffset = m_scrollPos % m_scrollUnit;
        if( iLeftOffset != 0 ) {
            if( iLeftOffset >= m_scrollUnit/2 ) m_scrollPos += m_scrollUnit - iLeftOffset;
            else m_scrollPos -= iLeftOffset;
        }
    }
    if( m_scrollPos > m_range ) m_scrollPos = m_range;

    SetPos(m_rcItem, true);

    if(triggerEvent && m_manager != nullptr)
        m_manager->SendNotify(this, DUI_MSGTYPE_SCROLL, (WPARAM)(long)m_scrollPos, (LPARAM)(long)iOldScrollPos, true, false);
}

int UIScrollBar::GetLineSize() const {
    if (m_scrollUnit > 1) return m_scrollUnit;
    return m_lineSize;
}

void UIScrollBar::SetLineSize(int size) {
    if (size >= 0) m_lineSize = size;
}

int UIScrollBar::GetScrollUnit() const {
    return m_scrollUnit;
}

void UIScrollBar::SetScrollUnit(int unit) {
    if (unit >= 0) m_scrollUnit = unit;
}

bool UIScrollBar::GetShowButton1() {
    return m_showButton1;
}

void UIScrollBar::SetShowButton1(bool show) {
    m_showButton1 = show;
    SetPos(m_rcItem, true);
}

uint32_t UIScrollBar::GetButton1Color() const {
    return m_button1Color;
}

void UIScrollBar::SetButton1Color(uint32_t color) {
    if(m_button1Color == color){
        return;
    }
    m_button1Color = color;
    Invalidate();
}

UIString UIScrollBar::GetButton1NormalImage() const {
    return m_diButton1Normal.sDrawString;
}

void UIScrollBar::SetButton1NormalImage(const UIString &normalImage) {
    if( m_diButton1Normal.sDrawString == normalImage && m_diButton1Normal.pImageInfo != nullptr ) return;
    m_diButton1Normal.Clear();
    m_diButton1Normal.sDrawString = normalImage;
    Invalidate();
}

UIString UIScrollBar::GetButton1HotImage() const {
    return m_diButton1Hot.sDrawString;
}

void UIScrollBar::SetButton1HotImage(const UIString &hotImage) {
    if( m_diButton1Hot.sDrawString == hotImage && m_diButton1Hot.pImageInfo != nullptr ) return;
    m_diButton1Hot.Clear();
    m_diButton1Hot.sDrawString = hotImage;
    Invalidate();
}

UIString UIScrollBar::GetButton1PushedImage() const {
    return m_diButton1Pushed.sDrawString;
}

void UIScrollBar::SetButton1PushedImage(const UIString &pushedImage) {
    if( m_diButton1Pushed.sDrawString == pushedImage && m_diButton1Pushed.pImageInfo != NULL ) return;
    m_diButton1Pushed.Clear();
    m_diButton1Pushed.sDrawString = pushedImage;
    Invalidate();
}

UIString UIScrollBar::GetButton1DisabledImage() const {
    return m_diButton1Disabled.sDrawString;
}

void UIScrollBar::SetButton1DisabledImage(const UIString &disabledImage) {
    if( m_diButton1Disabled.sDrawString == disabledImage && m_diButton1Disabled.pImageInfo != NULL ) return;
    m_diButton1Disabled.Clear();
    m_diButton1Disabled.sDrawString = disabledImage;
    Invalidate();
}

bool UIScrollBar::GetShowButton2() {
    return m_showButton2;
}

void UIScrollBar::SetShowButton2(bool show) {
    m_showButton2 = show;
    SetPos(m_rcItem, true);
}

uint32_t UIScrollBar::GetButton2Color() const {
    return m_button2Color;
}

void UIScrollBar::SetButton2Color(uint32_t color) {
    if(m_button2Color == color){
        return;
    }
    m_button2Color = color;
    Invalidate();
}

UIString UIScrollBar::GetButton2NormalImage() const {
    return m_diButton2Normal.sDrawString;
}

void UIScrollBar::SetButton2NormalImage(const UIString &normalImage) {
    if( m_diButton2Normal.sDrawString == normalImage && m_diButton2Normal.pImageInfo != NULL ) return;
    m_diButton2Normal.Clear();
    m_diButton2Normal.sDrawString = normalImage;
    Invalidate();
}

UIString UIScrollBar::GetButton2HotImage() const {
    return m_diButton2Hot.sDrawString;;
}

void UIScrollBar::SetButton2HotImage(const UIString &hotImage) {
    if( m_diButton2Hot.sDrawString == hotImage && m_diButton2Hot.pImageInfo != NULL ) return;
    m_diButton2Hot.Clear();
    m_diButton2Hot.sDrawString = hotImage;
    Invalidate();
}

UIString UIScrollBar::GetButton2PushedImage() const {
    return m_diButton2Pushed.sDrawString;
}

void UIScrollBar::SetButton2PushedImage(const UIString &pushedImage) {
    if( m_diButton2Pushed.sDrawString == pushedImage && m_diButton2Pushed.pImageInfo != NULL ) return;
    m_diButton2Pushed.Clear();
    m_diButton2Pushed.sDrawString = pushedImage;
    Invalidate();
}

UIString UIScrollBar::GetButton2DisabledImage() const {
    return m_diButton2Disabled.sDrawString;
}

void UIScrollBar::SetButton2DisabledImage(const UIString &disabledImage) {
    if( m_diButton2Disabled.sDrawString == disabledImage && m_diButton2Disabled.pImageInfo != NULL ) return;
    m_diButton2Disabled.Clear();
    m_diButton2Disabled.sDrawString = disabledImage;
    Invalidate();
}

uint32_t UIScrollBar::GetThumbColor() const {
    return m_thumbColor;
}

void UIScrollBar::SetThumbColor(uint32_t color) {
    if(m_thumbColor == color){
        return;
    }
    m_thumbColor = color;
    Invalidate();
}

UIString UIScrollBar::GetThumbNormalImage() const {
    return m_diThumbNormal.sDrawString;
}

void UIScrollBar::SetThumbNormalImage(const UIString &normalImage) {
    if( m_diThumbNormal.sDrawString == normalImage && m_diThumbNormal.pImageInfo != NULL ) return;
    m_diThumbNormal.Clear();
    m_diThumbNormal.sDrawString = normalImage;
    Invalidate();
}

UIString UIScrollBar::GetThumbHotImage() const {
    return m_diThumbHot.sDrawString;
}

void UIScrollBar::SetThumbHotImage(const UIString &hotImage) {
    if( m_diThumbHot.sDrawString == hotImage && m_diThumbHot.pImageInfo != NULL ) return;
    m_diThumbHot.Clear();
    m_diThumbHot.sDrawString = hotImage;
    Invalidate();
}

UIString UIScrollBar::GetThumbPushedImage() const {
    return m_diThumbPushed.sDrawString;
}

void UIScrollBar::SetThumbPushedImage(const UIString &pushedImage) {
    if( m_diThumbPushed.sDrawString == pushedImage && m_diThumbPushed.pImageInfo != NULL ) return;
    m_diThumbPushed.Clear();
    m_diThumbPushed.sDrawString = pushedImage;
    Invalidate();
}

UIString UIScrollBar::GetThumbDisabledImage() const {
    return m_diThumbDisabled.sDrawString;
}

void UIScrollBar::SetThumbDisabledImage(const UIString &disabledImage) {
    if( m_diThumbDisabled.sDrawString == disabledImage && m_diThumbDisabled.pImageInfo != NULL ) return;
    m_diThumbDisabled.Clear();
    m_diThumbDisabled.sDrawString = disabledImage;
    Invalidate();
}

UIString UIScrollBar::GetRailNormalImage() const {
    return m_diRailNormal.sDrawString;
}

void UIScrollBar::SetRailNormalImage(const UIString &normalImage) {
    if( m_diRailNormal.sDrawString == normalImage && m_diRailNormal.pImageInfo != NULL ) return;
    m_diRailNormal.Clear();
    m_diRailNormal.sDrawString = normalImage;
    Invalidate();
}

UIString UIScrollBar::GetRailHotImage() const {
    return m_diRailHot.sDrawString;
}

void UIScrollBar::SetRailHotImage(const UIString &hotImage) {
    if( m_diRailHot.sDrawString == hotImage && m_diRailHot.pImageInfo != NULL ) return;
    m_diRailHot.Clear();
    m_diRailHot.sDrawString = hotImage;
    Invalidate();
}

UIString UIScrollBar::GetRailPushedImage() const {
    return m_diRailPushed.sDrawString;
}

void UIScrollBar::SetRailPushedImage(const UIString &pushedImage) {
    if( m_diRailPushed.sDrawString == pushedImage && m_diRailPushed.pImageInfo != NULL ) return;
    m_diRailPushed.Clear();
    m_diRailPushed.sDrawString = pushedImage;
    Invalidate();
}

UIString UIScrollBar::GetRailDisabledImage() const {
    return m_diRailDisabled.sDrawString;
}

void UIScrollBar::SetRailDisabledImage(const UIString &disabledImage) {
    if( m_diRailDisabled.sDrawString == disabledImage && m_diRailDisabled.pImageInfo != NULL ) return;
    m_diRailDisabled.Clear();
    m_diRailDisabled.sDrawString = disabledImage;
    Invalidate();
}

UIString UIScrollBar::GetBkNormalImage() const {
    return m_diBkNormal.sDrawString;
}

void UIScrollBar::SetBkNormalImage(const UIString &normalImage) {
    if( m_diBkNormal.sDrawString == normalImage && m_diBkNormal.pImageInfo != NULL ) return;
    m_diBkNormal.Clear();
    m_diBkNormal.sDrawString = normalImage;
    Invalidate();
}

UIString UIScrollBar::GetBkHotImage() const {
    return m_diBkHot.sDrawString;
}

void UIScrollBar::SetBkHotImage(const UIString &hotImage) {
    if( m_diBkHot.sDrawString == hotImage && m_diBkHot.pImageInfo != NULL ) return;
    m_diBkHot.Clear();
    m_diBkHot.sDrawString = hotImage;
    Invalidate();
}

UIString UIScrollBar::GetBkPushedImage() const {
    return m_diBkPushed.sDrawString;
}

void UIScrollBar::SetBkPushedImage(const UIString &pushedImage) {
    if( m_diBkPushed.sDrawString == pushedImage && m_diBkPushed.pImageInfo != NULL ) return;
    m_diBkPushed.Clear();
    m_diBkPushed.sDrawString = pushedImage;
    Invalidate();
}

UIString UIScrollBar::GetBkDisabledImage() const {
    return m_diBkDisabled.sDrawString;
}

void UIScrollBar::SetBkDisabledImage(const UIString &disabledImage) {
    if( m_diBkDisabled.sDrawString == disabledImage && m_diBkDisabled.pImageInfo != NULL ) return;
    m_diBkDisabled.Clear();
    m_diBkDisabled.sDrawString = disabledImage;
    Invalidate();
}

void UIScrollBar::SetPos(RECT rc, bool bNeedInvalidate) {
    UIControl::SetPos(rc, bNeedInvalidate);
    rc = m_rcItem;

    if( m_horizontal ) {
        int cx = rc.right - rc.left;
        if( m_showButton1 ) cx -= m_cxyFixed.cy;
        if( m_showButton2 ) cx -= m_cxyFixed.cy;
        if( cx > m_cxyFixed.cy ) {
            m_rcButton1.left = rc.left;
            m_rcButton1.top = rc.top;
            if( m_showButton1 ) {
                m_rcButton1.right = rc.left + m_cxyFixed.cy;
                m_rcButton1.bottom = rc.top + m_cxyFixed.cy;
            }
            else {
                m_rcButton1.right = m_rcButton1.left;
                m_rcButton1.bottom = m_rcButton1.top;
            }

            m_rcButton2.top = rc.top;
            m_rcButton2.right = rc.right;
            if( m_showButton2 ) {
                m_rcButton2.left = rc.right - m_cxyFixed.cy;
                m_rcButton2.bottom = rc.top + m_cxyFixed.cy;
            }
            else {
                m_rcButton2.left = m_rcButton2.right;
                m_rcButton2.bottom = m_rcButton2.top;
            }

            m_rcThumb.top = rc.top;
            m_rcThumb.bottom = rc.top + m_cxyFixed.cy;
            if( m_range > 0 ) {
                int cxThumb = cx * (rc.right - rc.left) / (m_range + rc.right - rc.left);
                if( cxThumb < m_cxyFixed.cy ) cxThumb = m_cxyFixed.cy;

                m_rcThumb.left = m_scrollPos * (cx - cxThumb) / m_range + m_rcButton1.right;
                m_rcThumb.right = m_rcThumb.left + cxThumb;
                if( m_rcThumb.right > m_rcButton2.left ) {
                    m_rcThumb.left = m_rcButton2.left - cxThumb;
                    m_rcThumb.right = m_rcButton2.left;
                }
            }
            else {
                m_rcThumb.left = m_rcButton1.right;
                m_rcThumb.right = m_rcButton2.left;
            }
        }
        else {
            int cxButton = (rc.right - rc.left) / 2;
            if( cxButton > m_cxyFixed.cy ) cxButton = m_cxyFixed.cy;
            m_rcButton1.left = rc.left;
            m_rcButton1.top = rc.top;
            if( m_showButton1 ) {
                m_rcButton1.right = rc.left + cxButton;
                m_rcButton1.bottom = rc.top + m_cxyFixed.cy;
            }
            else {
                m_rcButton1.right = m_rcButton1.left;
                m_rcButton1.bottom = m_rcButton1.top;
            }

            m_rcButton2.top = rc.top;
            m_rcButton2.right = rc.right;
            if( m_showButton2 ) {
                m_rcButton2.left = rc.right - cxButton;
                m_rcButton2.bottom = rc.top + m_cxyFixed.cy;
            }
            else {
                m_rcButton2.left = m_rcButton2.right;
                m_rcButton2.bottom = m_rcButton2.top;
            }
            memset(&m_rcThumb, 0, sizeof(m_rcThumb));
        }
    }
    else {
        int cy = rc.bottom - rc.top;
        if( m_showButton1 ) cy -= m_cxyFixed.cx;
        if( m_showButton2 ) cy -= m_cxyFixed.cx;
        if( cy > m_cxyFixed.cx ) {
            m_rcButton1.left = rc.left;
            m_rcButton1.top = rc.top;
            if( m_showButton1 ) {
                m_rcButton1.right = rc.left + m_cxyFixed.cx;
                m_rcButton1.bottom = rc.top + m_cxyFixed.cx;
            }
            else {
                m_rcButton1.right = m_rcButton1.left;
                m_rcButton1.bottom = m_rcButton1.top;
            }

            m_rcButton2.left = rc.left;
            m_rcButton2.bottom = rc.bottom;
            if( m_showButton2 ) {
                m_rcButton2.top = rc.bottom - m_cxyFixed.cx;
                m_rcButton2.right = rc.left + m_cxyFixed.cx;
            }
            else {
                m_rcButton2.top = m_rcButton2.bottom;
                m_rcButton2.right = m_rcButton2.left;
            }

            m_rcThumb.left = rc.left;
            m_rcThumb.right = rc.left + m_cxyFixed.cx;
            if( m_range > 0 ) {
                int cyThumb = cy * (rc.bottom - rc.top) / (m_range + rc.bottom - rc.top);
                if( cyThumb < m_cxyFixed.cx ) cyThumb = m_cxyFixed.cx;

                m_rcThumb.top = m_scrollPos * (cy - cyThumb) / m_range + m_rcButton1.bottom;
                m_rcThumb.bottom = m_rcThumb.top + cyThumb;
                if( m_rcThumb.bottom > m_rcButton2.top ) {
                    m_rcThumb.top = m_rcButton2.top - cyThumb;
                    m_rcThumb.bottom = m_rcButton2.top;
                }
            }
            else {
                m_rcThumb.top = m_rcButton1.bottom;
                m_rcThumb.bottom = m_rcButton2.top;
            }
        }
        else {
            int cyButton = (rc.bottom - rc.top) / 2;
            if( cyButton > m_cxyFixed.cx ) cyButton = m_cxyFixed.cx;
            m_rcButton1.left = rc.left;
            m_rcButton1.top = rc.top;
            if( m_showButton1 ) {
                m_rcButton1.right = rc.left + m_cxyFixed.cx;
                m_rcButton1.bottom = rc.top + cyButton;
            }
            else {
                m_rcButton1.right = m_rcButton1.left;
                m_rcButton1.bottom = m_rcButton1.top;
            }

            m_rcButton2.left = rc.left;
            m_rcButton2.bottom = rc.bottom;
            if( m_showButton2 ) {
                m_rcButton2.top = rc.bottom - cyButton;
                m_rcButton2.right = rc.left + m_cxyFixed.cx;
            }
            else {
                m_rcButton2.top = m_rcButton2.bottom;
                m_rcButton2.right = m_rcButton2.left;
            }
            memset(&m_rcThumb, 0, sizeof(m_rcThumb));
        }
    }
}

void UIScrollBar::DoEvent(TEventUI &event) {
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_owner != NULL ) m_owner->DoEvent(event);
        else UIControl::DoEvent(event);
        return;
    }

    if( event.Type == UIEVENT_SETFOCUS )
    {
        return;
    }
    if( event.Type == UIEVENT_KILLFOCUS )
    {
        return;
    }
    if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK )
    {
        if( !IsEnabled() ) return;

        m_lastScrollOffset = 0;
        m_scrollRepeatDelay = 0;
        if(m_timerId == 0){
            m_timerId = m_manager->SetTimer(this, 50U);
        }
        cout<<"New timerId:"<<m_timerId<<endl;
        UIRect  rcButton1 {m_rcButton1};
        UIRect  rcButton2 {m_rcButton2};
        UIRect  rcThumb {m_rcThumb};
        if(rcButton1.IsPtIn(event.ptMouse)){
            m_button1State |= UISTATE_PUSHED;
            if( !m_horizontal ) {
                if( m_owner != NULL ) m_owner->LineUp();
                else SetScrollPos(m_scrollPos - GetLineSize());
            }
            else {
                if( m_owner != NULL ) m_owner->LineLeft();
                else SetScrollPos(m_scrollPos - GetLineSize());
            }
        }
        else if( rcButton2.IsPtIn(event.ptMouse) ) {
            m_button2State |= UISTATE_PUSHED;
            if( !m_horizontal ) {
                if( m_owner != NULL ) m_owner->LineDown();
                else SetScrollPos(m_scrollPos + GetLineSize());
            }
            else {
                if( m_owner != NULL ) m_owner->LineRight();
                else SetScrollPos(m_scrollPos + GetLineSize());
            }
        }
        else if( rcThumb.IsPtIn(event.ptMouse) ) {
            m_thumbState |= UISTATE_CAPTURED | UISTATE_PUSHED;
            ptLastMouse = event.ptMouse;
            m_lastScrollPos = m_scrollPos;
        }
        else {
            if( !m_horizontal ) {
                if( event.ptMouse.y < m_rcThumb.top ) {
                    if( m_owner != NULL ) m_owner->PageUp();
                    else SetScrollPos(m_scrollPos + m_rcItem.top - m_rcItem.bottom);
                }
                else if ( event.ptMouse.y > m_rcThumb.bottom ){
                    if( m_owner != NULL ) m_owner->PageDown();
                    else SetScrollPos(m_scrollPos - m_rcItem.top + m_rcItem.bottom);
                }
            }
            else {
                if( event.ptMouse.x < m_rcThumb.left ) {
                    if( m_owner != NULL ) m_owner->PageLeft();
                    else SetScrollPos(m_scrollPos + m_rcItem.left - m_rcItem.right);
                }
                else if ( event.ptMouse.x > m_rcThumb.right ){
                    if( m_owner != NULL ) m_owner->PageRight();
                    else SetScrollPos(m_scrollPos - m_rcItem.left + m_rcItem.right);
                }
            }
        }
        return;
    }
    if( event.Type == UIEVENT_BUTTONUP )
    {
        m_scrollRepeatDelay = 0;
        m_lastScrollOffset = 0;
        cout<<"KillTimer:"<<m_timerId<<endl;
        if(m_timerId != 0){
            m_manager->KillTimer(this, m_timerId);
            m_timerId = 0;
        }

        if( (m_thumbState & UISTATE_CAPTURED) != 0 ) {
            m_thumbState &= ~( UISTATE_CAPTURED | UISTATE_PUSHED );
            Invalidate();
        }
        else if( (m_button1State & UISTATE_PUSHED) != 0 ) {
            m_button1State &= ~UISTATE_PUSHED;
            Invalidate();
        }
        else if( (m_button2State & UISTATE_PUSHED) != 0 ) {
            m_button2State &= ~UISTATE_PUSHED;
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_MOUSEMOVE )
    {
        if( (m_thumbState & UISTATE_CAPTURED) != 0 ) {
            if( !m_horizontal ) {

                int vRange = m_rcItem.bottom - m_rcItem.top - m_rcThumb.bottom + m_rcThumb.top;
                if( m_showButton1 )
                    vRange -= m_cxyFixed.cx;
                if( m_showButton2 )
                    vRange -= m_cxyFixed.cx;
                if (vRange != 0)
                    m_lastScrollOffset = (event.ptMouse.y - ptLastMouse.y) * m_range / vRange;
            }
            else {

                int hRange = m_rcItem.right - m_rcItem.left - m_rcThumb.right + m_rcThumb.left;
                if( m_showButton1 )
                    hRange -= m_cxyFixed.cy;
                if( m_showButton2 )
                    hRange -= m_cxyFixed.cy;

                if (hRange != 0)
                    m_lastScrollOffset = (event.ptMouse.x - ptLastMouse.x) * m_range / hRange;
            }
        }
        else {
            if( (m_thumbState & UISTATE_HOT) != 0 ) {
                UIRect  rcThumb{m_rcThumb};
                if( !rcThumb.IsPtIn(event.ptMouse) ) {
                    m_thumbState &= ~UISTATE_HOT;
                    Invalidate();
                }
            }
            else {
                if( !IsEnabled() ) return;
                UIRect  rcThumb{m_rcThumb};
                if( rcThumb.IsPtIn(event.ptMouse) ) {
                    m_thumbState |= UISTATE_HOT;
                    Invalidate();
                }
            }
        }
        return;
    }
    if( event.Type == UIEVENT_CONTEXTMENU )
    {
        return;
    }
    if( event.Type == UIEVENT_TIMER && (long)(event.wParam) == m_timerId )
    {
        ++m_scrollRepeatDelay;
        if( (m_thumbState & UISTATE_CAPTURED) != 0 ) {
            if( !m_horizontal ) {
                if( m_owner != NULL ) m_owner->SetScrollPos(SIZE{m_owner->GetScrollPos().cx, \
					m_lastScrollPos + m_lastScrollOffset});
                else SetScrollPos(m_lastScrollPos + m_lastScrollOffset);
            }
            else {
                if( m_owner != NULL ) m_owner->SetScrollPos(SIZE{m_lastScrollPos + m_lastScrollOffset, \
					m_owner->GetScrollPos().cy});
                else SetScrollPos(m_lastScrollPos + m_lastScrollOffset);
            }
            Invalidate();
        }
        else if( (m_button1State & UISTATE_PUSHED) != 0 ) {
            if( m_scrollRepeatDelay <= 5 ) return;
            if( !m_horizontal ) {
                if( m_owner != NULL ) m_owner->LineUp();
                else SetScrollPos(m_scrollPos - GetLineSize());
            }
            else {
                if( m_owner != NULL ) m_owner->LineLeft();
                else SetScrollPos(m_scrollPos - GetLineSize());
            }
        }
        else if( (m_button2State & UISTATE_PUSHED) != 0 ) {
            if( m_scrollRepeatDelay <= 5 ) return;
            if( !m_horizontal ) {
                if( m_owner != NULL ) m_owner->LineDown();
                else SetScrollPos(m_scrollPos + GetLineSize());
            }
            else {
                if( m_owner != NULL ) m_owner->LineRight();
                else SetScrollPos(m_scrollPos + GetLineSize());
            }
        }
        else {
            if( m_scrollRepeatDelay <= 5 ) return;
            POINT pt = { 0 };
            //gdk_window_get_cursor()
#ifdef WIN32
            ::GetCursorPos(&pt);
            ::ScreenToClient(m_manager->GetPaintWindow(), &pt);
#else
#if GTK_CHECK_VERSION(3, 20, 0)
            GtkWidget  *widget = m_manager->GetPaintWindow();
            gdk_window_get_device_position(gtk_widget_get_window(widget),
                                           gdk_seat_get_pointer(gdk_display_get_default_seat(gtk_widget_get_display(widget))),
                                           reinterpret_cast<gint *>(&pt.x), reinterpret_cast<gint *>(&pt.y), 0);
#else
            gdk_window_get_device_position(gtk_widget_get_window(m_manager->GetPaintWindow()),
                                 gdk_device_manager_get_client_pointer(
                                     gdk_display_get_device_manager(gdk_window_get_display(m_manager->GetPaintWindow()))),
                                 reinterpret_cast<gint *>(&pt.x), reinterpret_cast<gint *>(&pt.y), NULL);
#endif
#endif
            if( !m_horizontal ) {
                if( pt.y < m_rcThumb.top ) {
                    if( m_owner != NULL ) m_owner->PageUp();
                    else SetScrollPos(m_scrollPos + m_rcItem.top - m_rcItem.bottom);
                }
                else if ( pt.y > m_rcThumb.bottom ){
                    if( m_owner != NULL ) m_owner->PageDown();
                    else SetScrollPos(m_scrollPos - m_rcItem.top + m_rcItem.bottom);
                }
            }
            else {
                if( pt.x < m_rcThumb.left ) {
                    if( m_owner != NULL ) m_owner->PageLeft();
                    else SetScrollPos(m_scrollPos + m_rcItem.left - m_rcItem.right);
                }
                else if ( pt.x > m_rcThumb.right ){
                    if( m_owner != NULL ) m_owner->PageRight();
                    else SetScrollPos(m_scrollPos - m_rcItem.left + m_rcItem.right);
                }
            }
        }
        return;
    }
    if( event.Type == UIEVENT_MOUSEENTER )
    {
        UIRect rcItem { m_rcItem};
        if( rcItem.IsPtIn(event.ptMouse ) ) {
            if( IsEnabled() ) {
                m_button1State |= UISTATE_HOT;
                m_button2State |= UISTATE_HOT;
                UIRect rcThumb { m_rcThumb};
                if( rcThumb.IsPtIn(event.ptMouse) ) m_thumbState |= UISTATE_HOT;
                Invalidate();
            }
        }
    }
    if( event.Type == UIEVENT_MOUSELEAVE )
    {
        UIRect rcItem{m_rcItem};
        if( rcItem.IsPtIn(event.ptMouse ) ) {
            if( IsEnabled() ) {
                m_button1State &= ~UISTATE_HOT;
                m_button2State &= ~UISTATE_HOT;
                m_thumbState &= ~UISTATE_HOT;
                Invalidate();
            }
            if (m_manager) m_manager->RemoveMouseLeaveNeeded(this);
        }
        else {
            if (m_manager) m_manager->AddMouseLeaveNeeded(this);
            return;
        }
    }

    if( m_owner != NULL ) m_owner->DoEvent(event); else UIControl::DoEvent(event);
}

void UIScrollBar::SetAttribute(const char *name, const char *value) {
    if( strcasecmp(name, "button1color") == 0 ) {
        while( *value > '\0' && *value <= ' ' ) value = CharNext(value);
        if( *value == '#') value = CharNext(value);
        char *pstr = NULL;
        uint32_t clrColor = strtoul(value, &pstr, 16);
        SetButton1Color(clrColor);
    }
    else if( strcasecmp(name, "button1normalimage") == 0 ) SetButton1NormalImage(UIString{value});
    else if( strcasecmp(name, "button1hotimage") == 0 ) SetButton1HotImage(UIString{value});
    else if( strcasecmp(name, "button1pushedimage") == 0 ) SetButton1PushedImage(UIString{value});
    else if( strcasecmp(name, "button1disabledimage") == 0 ) SetButton1DisabledImage(UIString{value});
    else if( strcasecmp(name, "button2color") == 0 ) {
        while( *value > '\0' && *value <= ' ' ) value = CharNext(value);
        if( *value == '#') value = CharNext(value);
        char *pstr = NULL;
        uint32_t clrColor = strtoul(value, &pstr, 16);
        SetButton2Color(clrColor);
    }
    else if( strcasecmp(name, "button2normalimage") == 0 ) SetButton2NormalImage(UIString{value});
    else if( strcasecmp(name, "button2hotimage") == 0 ) SetButton2HotImage(UIString{value});
    else if( strcasecmp(name, "button2pushedimage") == 0 ) SetButton2PushedImage(UIString{value});
    else if( strcasecmp(name, "button2disabledimage") == 0 ) SetButton2DisabledImage(UIString{value});
    else if( strcasecmp(name, "thumbcolor") == 0 ) {
        while( *value > '\0' && *value <= ' ' ) value = CharNext(value);
        if( *value == '#') value = CharNext(value);
        char *pstr = NULL;
        uint32_t clrColor = strtoul(value, &pstr, 16);
        SetThumbColor(clrColor);
    }
    else if( strcasecmp(name, "thumbnormalimage") == 0 ) SetThumbNormalImage(UIString{value});
    else if( strcasecmp(name, "thumbhotimage") == 0 ) SetThumbHotImage(UIString{value});
    else if( strcasecmp(name, "thumbpushedimage") == 0 ) SetThumbPushedImage(UIString{value});
    else if( strcasecmp(name, "thumbdisabledimage") == 0 ) SetThumbDisabledImage(UIString{value});
    else if( strcasecmp(name, "railnormalimage") == 0 ) SetRailNormalImage(UIString{value});
    else if( strcasecmp(name, "railhotimage") == 0 ) SetRailHotImage(UIString{value});
    else if( strcasecmp(name, "railpushedimage") == 0 ) SetRailPushedImage(UIString{value});
    else if( strcasecmp(name, "raildisabledimage") == 0 ) SetRailDisabledImage(UIString{value});
    else if( strcasecmp(name, "bknormalimage") == 0 ) SetBkNormalImage(UIString{value});
    else if( strcasecmp(name, "bkhotimage") == 0 ) SetBkHotImage(UIString{value});
    else if( strcasecmp(name, "bkpushedimage") == 0 ) SetBkPushedImage(UIString{value});
    else if( strcasecmp(name, "bkdisabledimage") == 0 ) SetBkDisabledImage(UIString{value});
    else if( strcasecmp(name, "hor") == 0 ) SetHorizontal(strcasecmp(value, "true") == 0);
    else if( strcasecmp(name, "linesize") == 0 ) SetLineSize(atoi(value));
    else if( strcasecmp(name, "range") == 0 ) SetScrollRange(atoi(value));
    else if( strcasecmp(name, "value") == 0 ) SetScrollPos(atoi(value));
    else if( strcasecmp(name, "scrollunit") == 0 ) SetScrollUnit(atoi(value));
    else if( strcasecmp(name, "showbutton1") == 0 ) SetShowButton1(strcasecmp(value, "true") == 0);
    else if( strcasecmp(name, "showbutton2") == 0 ) SetShowButton2(strcasecmp(value, "true") == 0);
    else UIControl::SetAttribute(name, value);
}

bool UIScrollBar::DoPaint(HANDLE_DC hDC, const RECT &rcPaint, UIControl *stopControl) {
    PaintBkColor(hDC);
    PaintBkImage(hDC);
    PaintBk(hDC);
    PaintButton1(hDC);
    PaintButton2(hDC);
    PaintThumb(hDC);
    PaintRail(hDC);
    PaintBorder(hDC);
    return true;
}

void UIScrollBar::PaintBk(HANDLE_DC hDC) {
    if( !IsEnabled() ) m_thumbState |= UISTATE_DISABLED;
    else m_thumbState &= ~ UISTATE_DISABLED;

    if( (m_thumbState & UISTATE_DISABLED) != 0 ) {
        if( DrawImage(hDC, m_diBkDisabled) ) return;
    }
    else if( (m_thumbState & UISTATE_PUSHED) != 0 ) {
        if( DrawImage(hDC, m_diBkPushed) ) return;
    }
    else if( (m_thumbState & UISTATE_HOT) != 0 ) {
        if( DrawImage(hDC, m_diBkHot) ) return;
    }

    if( DrawImage(hDC, m_diBkNormal) ) return;
}

void UIScrollBar::PaintButton1(HANDLE_DC hDC) {
    if( !m_showButton1 ) return;

    if( !IsEnabled() ) m_button1State |= UISTATE_DISABLED;
    else m_button1State &= ~ UISTATE_DISABLED;

    RECT rc = { 0 };
    rc.left = m_rcButton1.left - m_rcItem.left;
    rc.top = m_rcButton1.top - m_rcItem.top;
    rc.right = m_rcButton1.right - m_rcItem.left;
    rc.bottom = m_rcButton1.bottom - m_rcItem.top;

    if( m_button1Color != 0 ) {
        if( m_button1Color >= 0xFF000000 ) UIRenderEngine::DrawColor(hDC, m_rcButton1, GetAdjustColor(m_button1Color));
        else UIRenderEngine::DrawColor(hDC, m_rcButton1, GetAdjustColor(m_button1Color));
    }

    if( (m_button1State & UISTATE_DISABLED) != 0 ) {
        m_diButton1Disabled.rcDestOffset = rc;
        if( DrawImage(hDC, m_diButton1Disabled) ) return;
    }
    else if( (m_button1State & UISTATE_PUSHED) != 0 ) {
        m_diButton1Pushed.rcDestOffset = rc;
        if( DrawImage(hDC, m_diButton1Pushed) ) return;
    }
    else if( (m_button1State & UISTATE_HOT) != 0 ) {
        m_diButton1Hot.rcDestOffset = rc;
        if( DrawImage(hDC, m_diButton1Hot) ) return;
    }

    m_diButton1Normal.rcDestOffset = rc;
    if( DrawImage(hDC, m_diButton1Normal) ) return;
}

void UIScrollBar::PaintButton2(HANDLE_DC hDC) {
    if( !m_showButton2 ) return;

    if( !IsEnabled() ) m_button2State |= UISTATE_DISABLED;
    else m_button2State &= ~ UISTATE_DISABLED;

    RECT rc = { 0 };
    rc.left = m_rcButton2.left - m_rcItem.left;
    rc.top = m_rcButton2.top - m_rcItem.top;
    rc.right = m_rcButton2.right - m_rcItem.left;
    rc.bottom = m_rcButton2.bottom - m_rcItem.top;

    if( m_button2Color != 0 ) {
        if( m_button2Color >= 0xFF000000 ) UIRenderEngine::DrawColor(hDC, m_rcButton2, GetAdjustColor(m_button2Color));
        else UIRenderEngine::DrawColor(hDC, m_rcButton2, GetAdjustColor(m_button2Color));
    }

    if( (m_button2State & UISTATE_DISABLED) != 0 ) {
        m_diButton2Disabled.rcDestOffset = rc;
        if( DrawImage(hDC, m_diButton2Disabled) ) return;
    }
    else if( (m_button2State & UISTATE_PUSHED) != 0 ) {
        m_diButton2Pushed.rcDestOffset = rc;
        if( DrawImage(hDC, m_diButton2Pushed) ) return;
    }
    else if( (m_button2State & UISTATE_HOT) != 0 ) {
        m_diButton2Hot.rcDestOffset = rc;
        if( DrawImage(hDC, m_diButton2Hot) ) return;
    }

    m_diButton2Normal.rcDestOffset = rc;
    if( DrawImage(hDC, m_diButton2Normal) ) return;
}

void UIScrollBar::PaintThumb(HANDLE_DC hDC) {
    if( m_rcThumb.left == 0 && m_rcThumb.top == 0 && m_rcThumb.right == 0 && m_rcThumb.bottom == 0 ) return;
    if( !IsEnabled() ) m_thumbState |= UISTATE_DISABLED;
    else m_thumbState &= ~ UISTATE_DISABLED;

    RECT rc = { 0 };
    rc.left = m_rcThumb.left - m_rcItem.left;
    rc.top = m_rcThumb.top - m_rcItem.top;
    rc.right = m_rcThumb.right - m_rcItem.left;
    rc.bottom = m_rcThumb.bottom - m_rcItem.top;

    if( m_thumbColor != 0 ) {
        if( m_thumbColor >= 0xFF000000 ) UIRenderEngine::DrawColor(hDC, m_rcThumb, GetAdjustColor(m_thumbColor));
        else UIRenderEngine::DrawColor(hDC, m_rcThumb, GetAdjustColor(m_thumbColor));
    }

    if( (m_thumbState & UISTATE_DISABLED) != 0 ) {
        m_diThumbDisabled.rcDestOffset = rc;
        if( DrawImage(hDC, m_diThumbDisabled) ) return;
    }
    else if( (m_thumbState & UISTATE_PUSHED) != 0 ) {
        m_diThumbPushed.rcDestOffset = rc;
        if( DrawImage(hDC, m_diThumbPushed) ) return;
    }
    else if( (m_thumbState & UISTATE_HOT) != 0 ) {
        m_diThumbHot.rcDestOffset = rc;
        if( DrawImage(hDC, m_diThumbHot) ) return;
    }
    m_diThumbNormal.rcDestOffset = rc;
    if( DrawImage(hDC, m_diThumbNormal) ) return;
}

void UIScrollBar::PaintRail(HANDLE_DC hDC) {
    if( m_rcThumb.left == 0 && m_rcThumb.top == 0 && m_rcThumb.right == 0 && m_rcThumb.bottom == 0 ) return;
    if( !IsEnabled() ) m_thumbState |= UISTATE_DISABLED;
    else m_thumbState &= ~ UISTATE_DISABLED;

    RECT rc = { 0 };
    if( !m_horizontal ) {
        rc.left = m_rcThumb.left - m_rcItem.left;
        rc.top = (m_rcThumb.top + m_rcThumb.bottom) / 2 - m_rcItem.top - m_cxyFixed.cx / 2;
        rc.right = m_rcThumb.right - m_rcItem.left;
        rc.bottom = (m_rcThumb.top + m_rcThumb.bottom) / 2 - m_rcItem.top + m_cxyFixed.cx - m_cxyFixed.cx / 2;
    }
    else {
        rc.left = (m_rcThumb.left + m_rcThumb.right) / 2 - m_rcItem.left - m_cxyFixed.cy / 2;
        rc.top = m_rcThumb.top - m_rcItem.top;
        rc.right = (m_rcThumb.left + m_rcThumb.right) / 2 - m_rcItem.left + m_cxyFixed.cy - m_cxyFixed.cy / 2;
        rc.bottom = m_rcThumb.bottom - m_rcItem.top;
    }

    if( (m_thumbState & UISTATE_DISABLED) != 0 ) {
        m_diRailDisabled.rcDestOffset = rc;
        if( DrawImage(hDC, m_diRailDisabled) ) return;
    }
    else if( (m_thumbState & UISTATE_PUSHED) != 0 ) {
        m_diRailPushed.rcDestOffset = rc;
        if( DrawImage(hDC, m_diRailPushed) ) return;
    }
    else if( (m_thumbState & UISTATE_HOT) != 0 ) {
        m_diRailHot.rcDestOffset = rc;
        if( DrawImage(hDC, m_diRailHot) ) return;
    }
    m_diRailNormal.rcDestOffset = rc;
    if( DrawImage(hDC, m_diRailNormal) ) return;
}
