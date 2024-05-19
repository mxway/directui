#include <UISlider.h>
#include "UIPaintManager.h"
#include "UIRect.h"
#include <cassert>

UISlider::UISlider()
    :m_buttonState{0},
    m_step{1},
    m_immMode{false},
     m_szThumb{10, 10}
{
    m_textStyle = DT_SINGLELINE | DT_CENTER;
}

uint32_t UISlider::GetControlFlags() const {

    if(IsEnabled()){
        return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
    }
    return 0;
}

UIString UISlider::GetClass() const {
    return UIString{DUI_CTR_SLIDER};
}

LPVOID UISlider::GetInterface(const UIString &name) {
    if(name == DUI_CTR_SLIDER){
        return static_cast<UISlider*>(this);
    }
    return UIProgress::GetInterface(name);
}

void UISlider::SetEnabled(bool bEnabled) {
    UIProgress::SetEnabled(bEnabled);
    if(!IsEnabled()){
        m_buttonState = 0;
    }
}

int UISlider::GetChangeStep() const {
    return m_step;
}

void UISlider::SetChangeStep(int step) {
    m_step = step;
}

RECT UISlider::GetThumbRect() const {
    if( m_horizontal ) {
        int left = m_rcItem.left + (m_rcItem.right - m_rcItem.left - m_szThumb.cx) * (m_value - m_min) / (m_max - m_min);
        int top = (m_rcItem.bottom + m_rcItem.top - m_szThumb.cy) / 2;
        return RECT{left, top, left + m_szThumb.cx, top + m_szThumb.cy};
    }
    else {
        int left = (m_rcItem.right + m_rcItem.left - m_szThumb.cx) / 2;
        int top = m_rcItem.bottom - m_szThumb.cy - (m_rcItem.bottom - m_rcItem.top - m_szThumb.cy) * (m_value - m_min) / (m_max - m_min);
        return RECT{left, top, left + m_szThumb.cx, top + m_szThumb.cy};
    }
}

void UISlider::SetThumbSize(SIZE szXY) {
    m_szThumb = szXY;
}

bool UISlider::IsImmMode() const {
    return m_immMode;
}

void UISlider::SetImmMode(bool immMode) {
    m_immMode = immMode;
}

UIString UISlider::GetThumbImage() const {
    return m_diThumb.sDrawString;
}

void UISlider::SetThumbImage(const UIString &thumbImage) {
    if(m_diThumb.sDrawString == thumbImage && m_diThumb.pImageInfo!=nullptr){
        return;
    }
    m_diThumb.Clear();
    m_diThumb.sDrawString = thumbImage;
    Invalidate();
}

UIString UISlider::GetThumbHotImage() const {
    return m_diThumbHot.sDrawString;
}

void UISlider::SetThumbHotImage(const UIString &hotImage) {
    if(m_diThumbHot.sDrawString == hotImage && m_diThumbHot.pImageInfo!=nullptr){
        return;
    }
    m_diThumbHot.Clear();
    m_diThumbHot.sDrawString = hotImage;
    Invalidate();
}

UIString UISlider::GetThumbPushedImage() const {
    return m_diThumbPushed.sDrawString;
}

void UISlider::SetThumbPushedImage(const UIString &pushedImage) {
    if( m_diThumbPushed.sDrawString == pushedImage && m_diThumbPushed.pImageInfo != nullptr ){
        return;
    }
    m_diThumbPushed.Clear();
    m_diThumbPushed.sDrawString = pushedImage;
    Invalidate();
}

void UISlider::DoEvent(TEventUI &event) {
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_parent != nullptr ) m_parent->DoEvent(event);
        else UIProgress::DoEvent(event);
        return;
    }

    if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK )
    {
        if( IsEnabled() ) {
            m_buttonState |= UISTATE_CAPTURED;

            int nValue;

            if (m_horizontal) {
                if (event.ptMouse.x >= m_rcItem.right - m_szThumb.cx / 2) nValue = m_max;
                else if (event.ptMouse.x <= m_rcItem.left + m_szThumb.cx / 2) nValue = m_min;
                else nValue = m_min + (m_max - m_min) * (event.ptMouse.x - m_rcItem.left - m_szThumb.cx / 2) / (m_rcItem.right - m_rcItem.left - m_szThumb.cx);
            }
            else {
                if (event.ptMouse.y >= m_rcItem.bottom - m_szThumb.cy / 2) nValue = m_min;
                else if (event.ptMouse.y <= m_rcItem.top + m_szThumb.cy / 2) nValue = m_max;
                else nValue = m_min + (m_max - m_min) * (m_rcItem.bottom - event.ptMouse.y - m_szThumb.cy / 2) / (m_rcItem.bottom - m_rcItem.top - m_szThumb.cy);
            }
            if (m_value != nValue && nValue >= m_min && nValue <= m_max)
            {
                m_value = nValue;
                Invalidate();
            }
        }
        return;
    }
    if( event.Type == UIEVENT_BUTTONUP )
    {
        if( (m_buttonState & UISTATE_CAPTURED) != 0 ) {
            if( m_horizontal ) {
                if( event.ptMouse.x >= m_rcItem.right - m_szThumb.cx / 2 ) m_value = m_max;
                else if( event.ptMouse.x <= m_rcItem.left + m_szThumb.cx / 2 ) m_value = m_min;
                else m_value = m_min + (m_max - m_min) * (event.ptMouse.x - m_rcItem.left - m_szThumb.cx / 2 ) / (m_rcItem.right - m_rcItem.left - m_szThumb.cx);
            }
            else {
                if( event.ptMouse.y >= m_rcItem.bottom - m_szThumb.cy / 2 ) m_value = m_min;
                else if( event.ptMouse.y <= m_rcItem.top + m_szThumb.cy / 2  ) m_value = m_max;
                else m_value = m_min + (m_max - m_min) * (m_rcItem.bottom - event.ptMouse.y - m_szThumb.cy / 2 ) / (m_rcItem.bottom - m_rcItem.top - m_szThumb.cy);
            }
            m_manager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED);
            m_buttonState &= ~UISTATE_CAPTURED;
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_CONTEXTMENU )
    {
        return;
    }
    if( event.Type == UIEVENT_SCROLLWHEEL )
    {
        switch( LOWORD(event.wParam) ) {
            case SB_LINEUP:
                SetValue(GetValue() + GetChangeStep());
                m_manager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED);
                return;
            case SB_LINEDOWN:
                SetValue(GetValue() - GetChangeStep());
                m_manager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED);
                return;
        }
    }
    if( event.Type == UIEVENT_MOUSEMOVE )
    {
        if( (m_buttonState & UISTATE_CAPTURED) != 0 ) {
            if( m_horizontal ) {
                if( event.ptMouse.x >= m_rcItem.right - m_szThumb.cx / 2 ) m_value = m_max;
                else if( event.ptMouse.x <= m_rcItem.left + m_szThumb.cx / 2 ) m_value = m_min;
                else m_value = m_min + (m_max - m_min) * (event.ptMouse.x - m_rcItem.left - m_szThumb.cx / 2 ) / (m_rcItem.right - m_rcItem.left - m_szThumb.cx);
            }
            else {
                if( event.ptMouse.y >= m_rcItem.bottom - m_szThumb.cy / 2 ) m_value = m_min;
                else if( event.ptMouse.y <= m_rcItem.top + m_szThumb.cy / 2  ) m_value = m_max;
                else m_value = m_min + (m_max - m_min) * (m_rcItem.bottom - event.ptMouse.y - m_szThumb.cy / 2 ) / (m_rcItem.bottom - m_rcItem.top - m_szThumb.cy);
            }
            if( m_immMode ) m_manager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED);
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_SETCURSOR )
    {
        UIRect rcThumb {GetThumbRect()};
        if( IsEnabled() && rcThumb.IsPtIn(event.ptMouse) ) {
            UILoadCursor(m_manager, UI_IDC_HAND);
            return;
        }
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
        if( !rcItem.IsPtIn(event.ptMouse ) ) {
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
    UIProgress::DoEvent(event);
}

void UISlider::SetAttribute(const char *pstrName, const char *pstrValue) {
    if( strcasecmp(pstrName, "thumbimage") == 0 ) SetThumbImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "thumbhotimage") == 0 ) SetThumbHotImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "thumbpushedimage") == 0 ) SetThumbPushedImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "thumbsize") == 0 ) {
        SIZE szXY = {0};
        char *pstr = nullptr;
        szXY.cx = strtol(pstrValue, &pstr, 10);  assert(pstr);
        szXY.cy = strtol(pstr + 1, &pstr, 10);    assert(pstr);
        SetThumbSize(szXY);
    }
    else if( strcasecmp(pstrName, "step") == 0 ) {
        SetChangeStep(atoi(pstrValue));
    }
    else if( strcasecmp(pstrName, "imm") == 0 ) SetImmMode(strcasecmp(pstrValue, "true") == 0);
    else UIProgress::SetAttribute(pstrName, pstrValue);
}

void UISlider::PaintStatusImage(HANDLE_DC hDC) {
    UIProgress::PaintStatusImage(hDC);
}
