#include <UIVerticalLayout.h>
#include <UIPaintManager.h>
#include "UIRect.h"
#include <UIScrollBar.h>
#include <UIRenderEngine.h>

UIVerticalLayout::UIVerticalLayout()
    :m_sepHeight{0},
    m_buttonState{0},
    m_immMode{false},
     m_ptLastMouse{0, 0}
{
    memset(&m_rcNewPos, 0, sizeof(m_rcNewPos));
}

UIString UIVerticalLayout::GetClass() const {
    return UIString{DUI_CTR_VERTICALLAYOUT};
}

LPVOID UIVerticalLayout::GetInterface(const UIString &name) {
    if(name == DUI_CTR_VERTICALLAYOUT){
        return static_cast<UIVerticalLayout*>(this);
    }

    return UIContainer::GetInterface(name);
}

uint32_t UIVerticalLayout::GetControlFlags() const {
    if(IsEnabled() && m_sepHeight != 0){
        return UIFLAG_SETCURSOR;
    }
    return 0;
}

void UIVerticalLayout::SetSepHeight(int height) {
    m_sepHeight = height;
}

int UIVerticalLayout::GetSepHeight() const {
    return m_sepHeight;
}

void UIVerticalLayout::SetSepImmMode(bool immediately) {
    if(m_immMode == immediately){
        return;
    }
    if((m_buttonState & UISTATE_CAPTURED) != 0 && !m_immMode && m_manager!=nullptr){
        //TODO RemovePostPaint;
        //m_manager->RemovePostPaint(this);
    }
    m_immMode = immediately;
}

bool UIVerticalLayout::IsSepImmMode() const {
    return m_immMode;
}

void UIVerticalLayout::SetAttribute(const char *pstrName, const char *pstrValue) {
    if( strcasecmp(pstrName, "sepheight") == 0 ) SetSepHeight(atoi(pstrValue));
    else if( strcasecmp(pstrName, "sepimm") == 0 ) SetSepImmMode(strcasecmp(pstrValue, "true") == 0);
    else UIContainer::SetAttribute(pstrName, pstrValue);
}

void UIVerticalLayout::DoEvent(TEventUI &event) {
    UIRect rcSeparator {GetThumbRect(false)};
    if( m_sepHeight != 0 ) {
        if( event.Type == UIEVENT_BUTTONDOWN && IsEnabled() )
        {
            //RECT rcSeparator = GetThumbRect(false);
            if( rcSeparator.IsPtIn(event.ptMouse) ) {
                m_buttonState |= UISTATE_CAPTURED;
                m_ptLastMouse = event.ptMouse;
                m_rcNewPos = m_rcItem;
                //TODO PostPaint
                //if( !m_immMode && m_manager ) m_manager->AddPostPaint(this);
                return;
            }
        }
        if( event.Type == UIEVENT_BUTTONUP )
        {
            if( (m_buttonState & UISTATE_CAPTURED) != 0 ) {
                m_buttonState &= ~UISTATE_CAPTURED;
                m_rcItem = m_rcNewPos;
                //TODO RemovePostPaint
                //if( !m_immMode && m_manager ) m_manager->RemovePostPaint(this);
                NeedParentUpdate();
                return;
            }
        }
        if( event.Type == UIEVENT_MOUSEMOVE )
        {
            if( (m_buttonState & UISTATE_CAPTURED) != 0 ) {
                long cy = event.ptMouse.y - m_ptLastMouse.y;
                m_ptLastMouse = event.ptMouse;
                RECT rc = m_rcNewPos;
                if( m_sepHeight >= 0 ) {
                    if( cy > 0 && event.ptMouse.y < m_rcNewPos.bottom + m_sepHeight ) return;
                    if( cy < 0 && event.ptMouse.y > m_rcNewPos.bottom ) return;
                    rc.bottom += cy;
                    if( rc.bottom - rc.top <= GetMinHeight() ) {
                        if( m_rcNewPos.bottom - m_rcNewPos.top <= GetMinHeight() ) return;
                        rc.bottom = rc.top + GetMinHeight();
                    }
                    if( rc.bottom - rc.top >= GetMaxHeight() ) {
                        if( m_rcNewPos.bottom - m_rcNewPos.top >= GetMaxHeight() ) return;
                        rc.bottom = rc.top + GetMaxHeight();
                    }
                }
                else {
                    if( cy > 0 && event.ptMouse.y < m_rcNewPos.top ) return;
                    if( cy < 0 && event.ptMouse.y > m_rcNewPos.top + m_sepHeight ) return;
                    rc.top += cy;
                    if( rc.bottom - rc.top <= GetMinHeight() ) {
                        if( m_rcNewPos.bottom - m_rcNewPos.top <= GetMinHeight() ) return;
                        rc.top = rc.bottom - GetMinHeight();
                    }
                    if( rc.bottom - rc.top >= GetMaxHeight() ) {
                        if( m_rcNewPos.bottom - m_rcNewPos.top >= GetMaxHeight() ) return;
                        rc.top = rc.bottom - GetMaxHeight();
                    }
                }

                UIRect rcInvalidate {GetThumbRect(true)};
                m_rcNewPos = rc;
                m_cxyFixed.cy = m_rcNewPos.bottom - m_rcNewPos.top;

                if( m_immMode ) {
                    m_rcItem = m_rcNewPos;
                    NeedParentUpdate();
                }
                else {
                    rcInvalidate.Join(GetThumbRect(true));
                    rcInvalidate.Join(GetThumbRect(false));
                    if( m_manager ) m_manager->Invalidate(rcInvalidate);
                }
                return;
            }
        }
        if( event.Type == UIEVENT_SETCURSOR )
        {
            //RECT rcSeparator = GetThumbRect(false);
            if( IsEnabled() && rcSeparator.IsPtIn(event.ptMouse) ) {
                UILoadCursor(m_manager, UI_IDC_RESIZENS);
                //::SetCursor(::LoadCursor(nullptr, IDC_SIZENS));
                return;
            }
        }
    }
    UIContainer::DoEvent(event);
}

void UIVerticalLayout::SetPos(RECT rc, bool needInvalidate) {
    UIControl::SetPos(rc, needInvalidate);
    rc = m_rcItem;

    // Adjust for inset
    rc.left += m_rcInset.left;
    rc.top += m_rcInset.top;
    rc.right -= m_rcInset.right;
    rc.bottom -= m_rcInset.bottom;
    if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
    if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

    if( m_items.GetSize() == 0) {
        ProcessScrollBar(rc, 0, 0);
        return;
    }

    // Determine the minimum size
    SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
    if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() )
        szAvailable.cx += m_pHorizontalScrollBar->GetScrollRange();
    if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() )
        szAvailable.cy += m_pVerticalScrollBar->GetScrollRange();

    int cxNeeded = 0;
    int nAdjustables = 0;
    int cyFixed = 0;
    int nEstimateNum = 0;
    SIZE szControlAvailable;
    int iControlMaxWidth = 0;
    int iControlMaxHeight = 0;
    for( int it1 = 0; it1 < m_items.GetSize(); it1++ ) {
        auto* pControl = static_cast<UIControl*>(m_items[it1]);
        if( !pControl->IsVisible() ) continue;
        if( pControl->IsFloat() ) continue;
        szControlAvailable = szAvailable;
        RECT rcPadding = pControl->GetPadding();
        szControlAvailable.cx -= rcPadding.left + rcPadding.right;
        iControlMaxWidth = pControl->GetFixedWidth();
        iControlMaxHeight = pControl->GetFixedHeight();
        if (iControlMaxWidth <= 0) iControlMaxWidth = pControl->GetMaxWidth();
        if (iControlMaxHeight <= 0) iControlMaxHeight = pControl->GetMaxHeight();
        if (szControlAvailable.cx > iControlMaxWidth) szControlAvailable.cx = iControlMaxWidth;
        if (szControlAvailable.cy > iControlMaxHeight) szControlAvailable.cy = iControlMaxHeight;
        SIZE sz = pControl->EstimateSize(szControlAvailable);
        if( sz.cy == 0 ) {
            nAdjustables++;
        }
        else {
            if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
            if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
        }
        cyFixed += sz.cy + pControl->GetPadding().top + pControl->GetPadding().bottom;

        sz.cx = MAX(sz.cx, (long)0);
        if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
        if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();
        cxNeeded = MAX((long)cxNeeded, sz.cx + rcPadding.left + rcPadding.right);
        nEstimateNum++;
    }
    cyFixed += (nEstimateNum - 1) * m_iChildPadding;

    // Place elements
    int cyNeeded = 0;
    int cyExpand = 0;
    if( nAdjustables > 0 ) cyExpand = MAX((long)0, (szAvailable.cy - cyFixed) / nAdjustables);
    // Position the elements
    SIZE szRemaining = szAvailable;
    int iPosY = rc.top;
    if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
        iPosY -= m_pVerticalScrollBar->GetScrollPos();
    }

    int iEstimate = 0;
    int iAdjustable = 0;
    int cyFixedRemaining = cyFixed;
    for( int it2 = 0; it2 < m_items.GetSize(); it2++ ) {
        auto* pControl = static_cast<UIControl*>(m_items[it2]);
        if( !pControl->IsVisible() ) continue;
        if( pControl->IsFloat() ) {
            SetFloatPos(it2);
            continue;
        }

        iEstimate += 1;
        RECT rcPadding = pControl->GetPadding();
        szRemaining.cy -= rcPadding.top;

        szControlAvailable = szRemaining;
        szControlAvailable.cx -= rcPadding.left + rcPadding.right;
        iControlMaxWidth = pControl->GetFixedWidth();
        iControlMaxHeight = pControl->GetFixedHeight();
        if (iControlMaxWidth <= 0) iControlMaxWidth = pControl->GetMaxWidth();
        if (iControlMaxHeight <= 0) iControlMaxHeight = pControl->GetMaxHeight();
        if (szControlAvailable.cx > iControlMaxWidth) szControlAvailable.cx = iControlMaxWidth;
        if (szControlAvailable.cy > iControlMaxHeight) szControlAvailable.cy = iControlMaxHeight;
        cyFixedRemaining = cyFixedRemaining - (rcPadding.top + rcPadding.bottom);
        if (iEstimate > 1) cyFixedRemaining = cyFixedRemaining - m_iChildPadding;
        SIZE sz = pControl->EstimateSize(szControlAvailable);
        if( sz.cy == 0 ) {
            iAdjustable++;
            sz.cy = cyExpand;
            // Distribute remaining to last element (usually round-off left-overs)
            if( iAdjustable == nAdjustables ) {
                sz.cy = MAX((long)0, szRemaining.cy - rcPadding.bottom - cyFixedRemaining);
            }
            if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
            if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
        }
        else {
            if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
            if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
            cyFixedRemaining -= sz.cy;
        }

        sz.cx = pControl->GetMaxWidth();
        if( sz.cx == 0 ) sz.cx = szAvailable.cx - rcPadding.left - rcPadding.right;
        if( sz.cx < 0 ) sz.cx = 0;
        if( sz.cx > szControlAvailable.cx ) sz.cx = szControlAvailable.cx;
        if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();

        uint32_t iChildAlign = GetChildAlign();
        if (iChildAlign == DT_CENTER) {
            int iPosX = (rc.right + rc.left) / 2;
            if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
                iPosX += m_pHorizontalScrollBar->GetScrollRange() / 2;
                iPosX -= m_pHorizontalScrollBar->GetScrollPos();
            }
            RECT rcCtrl = { iPosX - sz.cx/2, iPosY + rcPadding.top, iPosX + sz.cx - sz.cx/2, iPosY + sz.cy + rcPadding.top };
            pControl->SetPos(rcCtrl, false);
        }
        else if (iChildAlign == DT_RIGHT) {
            int iPosX = rc.right;
            if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
                iPosX += m_pHorizontalScrollBar->GetScrollRange();
                iPosX -= m_pHorizontalScrollBar->GetScrollPos();
            }
            RECT rcCtrl = { iPosX - rcPadding.right - sz.cx, iPosY + rcPadding.top, iPosX - rcPadding.right, iPosY + sz.cy + rcPadding.top };
            pControl->SetPos(rcCtrl, false);
        }
        else {
            int iPosX = rc.left;
            if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
                iPosX -= m_pHorizontalScrollBar->GetScrollPos();
            }
            RECT rcCtrl = { iPosX + rcPadding.left, iPosY + rcPadding.top, iPosX + rcPadding.left + sz.cx, iPosY + sz.cy + rcPadding.top };
            pControl->SetPos(rcCtrl, false);
        }

        iPosY += sz.cy + m_iChildPadding + rcPadding.top + rcPadding.bottom;
        cyNeeded += sz.cy + rcPadding.top + rcPadding.bottom;
        szRemaining.cy -= sz.cy + m_iChildPadding + rcPadding.bottom;
    }
    cyNeeded += (nEstimateNum - 1) * m_iChildPadding;

    // Process the scrollbar
    ProcessScrollBar(rc, cxNeeded, cyNeeded);
}

void UIVerticalLayout::DoPostPaint(HANDLE_DC hDC, const RECT &rcPaint) {
    if( (m_buttonState & UISTATE_CAPTURED) != 0 && !m_immMode ) {
        RECT rcSeparator = GetThumbRect(true);
        UIRenderEngine::DrawColor(hDC, rcSeparator, 0xAA000000);
    }
}

RECT UIVerticalLayout::GetThumbRect(bool useNew) const {
    if( (m_buttonState & UISTATE_CAPTURED) != 0 && useNew) {
        if( m_sepHeight >= 0 )
            return UIRect(m_rcNewPos.left, MAX(m_rcNewPos.bottom - m_sepHeight, m_rcNewPos.top),
                            m_rcNewPos.right, m_rcNewPos.bottom);
        else
            return UIRect(m_rcNewPos.left, m_rcNewPos.top, m_rcNewPos.right,
                            MIN(m_rcNewPos.top - m_sepHeight, m_rcNewPos.bottom));
    }
    else {
        if( m_sepHeight >= 0 )
            return UIRect(m_rcItem.left, MAX(m_rcItem.bottom - m_sepHeight, m_rcItem.top), m_rcItem.right,
                            m_rcItem.bottom);
        else
            return UIRect(m_rcItem.left, m_rcItem.top, m_rcItem.right,
                            MIN(m_rcItem.top - m_sepHeight, m_rcItem.bottom));

    }
}
