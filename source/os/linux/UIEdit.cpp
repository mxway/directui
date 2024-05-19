#include <UIEdit.h>
#include <UIPaintManager.h>
#include "UIRect.h"

struct UIEditInternal
{

};

uint32_t UIEdit::GetCharNumber() const {
    const char *start = m_text.GetData();
    const char *end = m_text.GetData() + m_text.GetLength();
    uint32_t result = 0;
    while(start < end){
        start = CharNext(start);
        result++;
    }
    return result;
}

void UIEdit::InitInternal() {
    m_internalImpl = make_shared<UIEditInternal>();
}

void UIEdit::DoEvent(TEventUI &event) {
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_parent != nullptr ) m_parent->DoEvent(event);
        else UILabel::DoEvent(event);
        return;
    }

    if( event.Type == UIEVENT_SETCURSOR && IsEnabled() )
    {
        UILoadCursor(m_manager, UI_IDC_TEXT);
        return;
    }
    if( event.Type == UIEVENT_SETFOCUS && IsEnabled() )
    {

        Invalidate();
    }
    if( event.Type == UIEVENT_KILLFOCUS && IsEnabled() )
    {
        //m_internalImpl->ReleaseImmContext();
        //::HideCaret(m_manager->GetPaintWindow());
        Invalidate();
    }
    if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK || event.Type == UIEVENT_RBUTTONDOWN)
    {
        //m_internalImpl->DoEvent(event);
        if( IsEnabled() ) {
            //GetManager()->ReleaseCapture();
        }
        return;
    }
    if( event.Type == UIEVENT_MOUSEMOVE )
    {
        return;
    }
    if( event.Type == UIEVENT_BUTTONUP )
    {
        return;
    }
    if( event.Type == UIEVENT_CONTEXTMENU )
    {
        return;
    }
    if( event.Type == UIEVENT_MOUSEENTER )
    {
        UIRect rcItem {m_rcItem};
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
        if( rcItem.IsPtIn(event.ptMouse ) ) {
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
    if(event.Type == UIEVENT_CHAR || event.Type == UIEVENT_KEYDOWN){
        //m_internalImpl->DoEvent(event);
        return;
    }
    UILabel::DoEvent(event);
}
