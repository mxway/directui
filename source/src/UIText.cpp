#include <UIText.h>
#include "UIPaintManager.h"
#include "UIRect.h"
#include <UIRenderEngine.h>

UIText::UIText()
    :m_nLinks {0},
    m_rcLinks{0},
    m_nHoverLink{0}
{
    m_textStyle = DT_WORDBREAK;
    m_rcTextPadding.left = 2;
    m_rcTextPadding.right = 2;
    memset(m_rcLinks, 0, sizeof(m_rcLinks));
}

UIText::~UIText() = default;

uint32_t UIText::GetControlFlags() const {
    if(IsEnabled() && m_nLinks>0){
        return UIFLAG_SETCURSOR;
    }
    return 0;
}

UIString UIText::GetClass() const {
    return UIString{DUI_CTR_TEXT};
}

LPVOID UIText::GetInterface(const UIString &name) {
    if(name == DUI_CTR_TEXT){
        return static_cast<UIText*>(this);
    }
    return UILabel::GetInterface(name);
}

void UIText::DoEvent(TEventUI &event) {
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_parent != nullptr ) m_parent->DoEvent(event);
        else UILabel::DoEvent(event);
        return;
    }

    if( event.Type == UIEVENT_SETCURSOR ) {
        for( int i = 0; i < m_nLinks; i++ ) {
            UIRect rcLink {m_rcLinks[i]};
            if( rcLink.IsPtIn(event.ptMouse) ) {
                UILoadCursor(m_manager,UI_IDC_HAND);
                //::SetCursor(::LoadCursor(NULL, IDC_HAND));
                return;
            }
        }
    }
    if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK && IsEnabled() ) {
        for( int i = 0; i < m_nLinks; i++ ) {
            UIRect rcLink { m_rcLinks[i]};
            if( rcLink.IsPtIn(event.ptMouse) ) {
                Invalidate();
                return;
            }
        }
    }
    if( event.Type == UIEVENT_BUTTONUP && IsEnabled() ) {
        for( int i = 0; i < m_nLinks; i++ ) {
            UIRect rcLink { m_rcLinks[i]};
            if( rcLink.IsPtIn(event.ptMouse) ) {
                m_manager->SendNotify(this, DUI_MSGTYPE_LINK, (WPARAM)(long)i);
                return;
            }
        }
    }
    if( event.Type == UIEVENT_CONTEXTMENU )
    {
        return;
    }
    // When you move over a link
    if( m_nLinks > 0 && event.Type == UIEVENT_MOUSEMOVE && IsEnabled() ) {
        int nHoverLink = -1;
        for( int i = 0; i < m_nLinks; i++ ) {
            UIRect rcLink { m_rcLinks[i]};
            if( rcLink.IsPtIn(event.ptMouse) )  {
                nHoverLink = i;
                break;
            }
        }

        if(m_nHoverLink != nHoverLink) {
            m_nHoverLink = nHoverLink;
            Invalidate();
            return;
        }
    }
    if( event.Type == UIEVENT_MOUSELEAVE ) {
        if( m_nLinks > 0 && IsEnabled() ) {
            if(m_nHoverLink != -1) {
                UIRect  rclink{m_rcLinks[m_nHoverLink]};
                if( !rclink.IsPtIn(event.ptMouse) ) {
                    m_nHoverLink = -1;
                    Invalidate();
                    if (m_manager) m_manager->RemoveMouseLeaveNeeded(this);
                }
                else {
                    if (m_manager) m_manager->AddMouseLeaveNeeded(this);
                    return;
                }
            }
        }
    }

    UILabel::DoEvent(event);
}

void UIText::PaintText(HANDLE_DC hDC) {
    if( m_text.IsEmpty() ) {
        m_nLinks = 0;
        return;
    }

    if( m_textColor == 0 ) m_textColor = m_manager->GetDefaultFontColor();
    if( m_disabledTextColor == 0 ) m_disabledTextColor = m_manager->GetDefaultDisabledColor();

    m_nLinks = (sizeof(m_rcLinks)/sizeof(*m_rcLinks));
    RECT rc = m_rcItem;
    rc.left += m_rcTextPadding.left;
    rc.right -= m_rcTextPadding.right;
    rc.top += m_rcTextPadding.top;
    rc.bottom -= m_rcTextPadding.bottom;
    if( IsEnabled() ) {
        if( m_showHtml )
            UIRenderEngine::DrawHtmlText(hDC, m_manager, rc, m_text, m_textColor, \
				m_rcLinks, m_sLinks, m_nLinks, m_fontId, m_textStyle);
        else
            UIRenderEngine::DrawText(hDC, m_manager, rc, m_text, m_textColor, \
				m_fontId, m_textStyle);
    }
    else {
        if( m_showHtml )
            UIRenderEngine::DrawHtmlText(hDC, m_manager, rc, m_text, m_disabledTextColor, \
				m_rcLinks, m_sLinks, m_nLinks, m_fontId, m_textStyle);
        else
            UIRenderEngine::DrawText(hDC, m_manager, rc, m_text, m_disabledTextColor, \
				m_fontId, m_textStyle);
    }
}

UIString *UIText::GetLinkContent(int iIndex) {
    if( iIndex >= 0 && iIndex < m_nLinks ) return &m_sLinks[iIndex];
    return NULL;
}
