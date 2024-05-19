#include <UIHorizontalLayout.h>
#include <UIPaintManager.h>
#include <UIScrollBar.h>
#include <UIRenderEngine.h>
#include <UIRect.h>

UIHorizontalLayout::UIHorizontalLayout()
    :m_sepWidth{0},
    m_buttonState {0},
    m_immMode{false},
     m_ptLastMouse{0,0},
     m_rcNewPos{0,0,0,0}
{

}

UIString UIHorizontalLayout::GetClass() const {
    return UIString{DUI_CTR_HORIZONTALLAYOUT};
}

LPVOID UIHorizontalLayout::GetInterface(const UIString &name) {
    if(name == DUI_CTR_HORIZONTALLAYOUT){
        return static_cast<UIHorizontalLayout*>(this);
    }
    return UIContainer::GetInterface(name);
}

uint32_t UIHorizontalLayout::GetControlFlags() const {
    if (IsEnabled() && m_sepWidth != 0) {
        return UIFLAG_SETCURSOR;
    }
    return 0;
}

void UIHorizontalLayout::SetSepWidth(int width) {
    m_sepWidth = width;
}

int UIHorizontalLayout::GetSepWidth() const {
    return m_sepWidth;
}

void UIHorizontalLayout::SetSepImmMode(bool immediately) {
    if(m_immMode == immediately){
        return;
    }
    if((m_buttonState & UISTATE_CAPTURED) != 0 && !m_immMode && m_manager!=nullptr ){
        //m_manager->RemovePostPaint(this);
    }
    m_immMode = immediately;
}

bool UIHorizontalLayout::IsSepImmMode() const {
    return m_immMode;
}

void UIHorizontalLayout::SetAttribute(const char *pstrName, const char *pstrValue) {
    if( strcasecmp(pstrName, "sepwidth") == 0 ) SetSepWidth(atoi(pstrValue));
    else if( strcasecmp(pstrName, "sepimm") == 0 ) SetSepImmMode(strcasecmp(pstrValue, "true") == 0);
    else UIContainer::SetAttribute(pstrName, pstrValue);
}

void UIHorizontalLayout::DoEvent(TEventUI &event) {
    if( m_sepWidth != 0 ) {
        if( event.Type == UIEVENT_BUTTONDOWN && IsEnabled() )
        {
            UIRect rcSeparator{GetThumbRect(false)};
            if( rcSeparator.IsPtIn(event.ptMouse) ) {
                m_buttonState |= UISTATE_CAPTURED;
                m_ptLastMouse = event.ptMouse;
                m_rcNewPos = m_rcItem;
                //if( !m_immMode && m_manager ) m_manager->AddPostPaint(this);
                return;
            }
        }
        if( event.Type == UIEVENT_BUTTONUP )
        {
            if( (m_buttonState & UISTATE_CAPTURED) != 0 ) {
                m_buttonState &= ~UISTATE_CAPTURED;
                m_rcItem = m_rcNewPos;
                //if( !m_immMode && m_manager ) m_manager->RemovePostPaint(this);
                NeedParentUpdate();
                return;
            }
        }
        if( event.Type == UIEVENT_MOUSEMOVE )
        {
            if( (m_buttonState & UISTATE_CAPTURED) != 0 ) {
                long cx = event.ptMouse.x - m_ptLastMouse.x;
                m_ptLastMouse = event.ptMouse;
                RECT rc = m_rcNewPos;
                if( m_sepWidth >= 0 ) {
                    if( cx > 0 && event.ptMouse.x < m_rcNewPos.right - m_sepWidth ) return;
                    if( cx < 0 && event.ptMouse.x > m_rcNewPos.right ) return;
                    rc.right += cx;
                    if( rc.right - rc.left <= GetMinWidth() ) {
                        if( m_rcNewPos.right - m_rcNewPos.left <= GetMinWidth() ) return;
                        rc.right = rc.left + GetMinWidth();
                    }
                    if( rc.right - rc.left >= GetMaxWidth() ) {
                        if( m_rcNewPos.right - m_rcNewPos.left >= GetMaxWidth() ) return;
                        rc.right = rc.left + GetMaxWidth();
                    }
                }
                else {
                    if( cx > 0 && event.ptMouse.x < m_rcNewPos.left ) return;
                    if( cx < 0 && event.ptMouse.x > m_rcNewPos.left - m_sepWidth ) return;
                    rc.left += cx;
                    if( rc.right - rc.left <= GetMinWidth() ) {
                        if( m_rcNewPos.right - m_rcNewPos.left <= GetMinWidth() ) return;
                        rc.left = rc.right - GetMinWidth();
                    }
                    if( rc.right - rc.left >= GetMaxWidth() ) {
                        if( m_rcNewPos.right - m_rcNewPos.left >= GetMaxWidth() ) return;
                        rc.left = rc.right - GetMaxWidth();
                    }
                }

                UIRect rcInvalidate {GetThumbRect(true)};
                m_rcNewPos = rc;
                m_cxyFixed.cx = m_rcNewPos.right - m_rcNewPos.left;

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
            UIRect rcSeparator{GetThumbRect(false)};
            if( IsEnabled() && rcSeparator.IsPtIn(event.ptMouse) ) {
                UILoadCursor(m_manager,UI_IDC_RESIZEWE);
                //::SetCursor(::LoadCursor(nullptr, IDC_SIZEWE));
                return;
            }
        }
    }
    UIContainer::DoEvent(event);
}

void UIHorizontalLayout::SetPos(RECT rc, bool needInvalidate) {
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

    int cyNeeded = 0;
    int nAdjustables = 0;
    int cxFixed = 0;
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
        szControlAvailable.cy -= rcPadding.top + rcPadding.bottom;
        iControlMaxWidth = pControl->GetFixedWidth();
        iControlMaxHeight = pControl->GetFixedHeight();
        if (iControlMaxWidth <= 0) iControlMaxWidth = pControl->GetMaxWidth();
        if (iControlMaxHeight <= 0) iControlMaxHeight = pControl->GetMaxHeight();
        if (szControlAvailable.cx > iControlMaxWidth) szControlAvailable.cx = iControlMaxWidth;
        if (szControlAvailable.cy > iControlMaxHeight) szControlAvailable.cy = iControlMaxHeight;
        SIZE sz = { 0 };
        if (pControl->GetFixedWidth() == 0) {
            nAdjustables++;
            sz.cy = pControl->GetFixedHeight();
        }
        else {
            sz = pControl->EstimateSize(szControlAvailable);
            if (sz.cx == 0) {
                nAdjustables++;
            }
            else {
                if (sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
                if (sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();
            }
        }

        cxFixed += sz.cx + pControl->GetPadding().left + pControl->GetPadding().right;

        sz.cy = MAX(sz.cy, (long)0);
        if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
        if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
        cyNeeded = MAX((long)cyNeeded, sz.cy + rcPadding.top + rcPadding.bottom);
        nEstimateNum++;
    }
    cxFixed += (nEstimateNum - 1) * m_iChildPadding;

    // Place elements
    int cxNeeded = 0;
    int cxExpand = 0;
    if( nAdjustables > 0 ) cxExpand = MAX((long)0, (szAvailable.cx - cxFixed) / nAdjustables);
    // Position the elements
    SIZE szRemaining = szAvailable;
    int iPosX = rc.left;
    if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
        iPosX -= m_pHorizontalScrollBar->GetScrollPos();
    }
    int iEstimate = 0;
    int iAdjustable = 0;
    int cxFixedRemaining = cxFixed;
    for( int it2 = 0; it2 < m_items.GetSize(); it2++ ) {
        auto* pControl = static_cast<UIControl*>(m_items[it2]);
        if( !pControl->IsVisible() ) continue;
        if( pControl->IsFloat() ) {
            SetFloatPos(it2);
            continue;
        }

        iEstimate += 1;
        RECT rcPadding = pControl->GetPadding();
        szRemaining.cx -= rcPadding.left;

        szControlAvailable = szRemaining;
        szControlAvailable.cy -= rcPadding.top + rcPadding.bottom;
        iControlMaxWidth = pControl->GetFixedWidth();
        iControlMaxHeight = pControl->GetFixedHeight();
        if (iControlMaxWidth <= 0) iControlMaxWidth = pControl->GetMaxWidth();
        if (iControlMaxHeight <= 0) iControlMaxHeight = pControl->GetMaxHeight();
        if (szControlAvailable.cx > iControlMaxWidth) szControlAvailable.cx = iControlMaxWidth;
        if (szControlAvailable.cy > iControlMaxHeight) szControlAvailable.cy = iControlMaxHeight;
        cxFixedRemaining = cxFixedRemaining - (rcPadding.left + rcPadding.right);
        if (iEstimate > 1) cxFixedRemaining = cxFixedRemaining - m_iChildPadding;
        SIZE sz = pControl->EstimateSize(szControlAvailable);
        if (pControl->GetFixedWidth() == 0 || sz.cx == 0) {
            iAdjustable++;
            sz.cx = cxExpand;
            // Distribute remaining to last element (usually round-off left-overs)
            if( iAdjustable == nAdjustables ) {
                sz.cx = MAX((long)0, szRemaining.cx - rcPadding.right - cxFixedRemaining);
            }
            if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
            if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();
        }
        else {
            if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
            if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();
            cxFixedRemaining -= sz.cx;
        }

        sz.cy = pControl->GetMaxHeight();
        if( sz.cy == 0 ) sz.cy = szAvailable.cy - rcPadding.top - rcPadding.bottom;
        if( sz.cy < 0 ) sz.cy = 0;
        if( sz.cy > szControlAvailable.cy ) sz.cy = szControlAvailable.cy;
        if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();

        uint32_t iChildAlign = GetChildVAlign();
        if (iChildAlign == DT_VCENTER) {
            int iPosY = (rc.bottom + rc.top) / 2;
            if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
                iPosY += m_pVerticalScrollBar->GetScrollRange() / 2;
                iPosY -= m_pVerticalScrollBar->GetScrollPos();
            }
            RECT rcCtrl = { iPosX + rcPadding.left, iPosY - sz.cy/2, iPosX + sz.cx + rcPadding.left, iPosY + sz.cy - sz.cy/2 };
            pControl->SetPos(rcCtrl, false);
        }
        else if (iChildAlign == DT_BOTTOM) {
            int iPosY = rc.bottom;
            if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
                iPosY += m_pVerticalScrollBar->GetScrollRange();
                iPosY -= m_pVerticalScrollBar->GetScrollPos();
            }
            RECT rcCtrl = { iPosX + rcPadding.left, iPosY - rcPadding.bottom - sz.cy, iPosX + sz.cx + rcPadding.left, iPosY - rcPadding.bottom };
            pControl->SetPos(rcCtrl, false);
        }
        else {
            int iPosY = rc.top;
            if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
                iPosY -= m_pVerticalScrollBar->GetScrollPos();
            }
            RECT rcCtrl = { iPosX + rcPadding.left, iPosY + rcPadding.top, iPosX + sz.cx + rcPadding.left, iPosY + sz.cy + rcPadding.top };
            pControl->SetPos(rcCtrl, false);
        }

        iPosX += sz.cx + m_iChildPadding + rcPadding.left + rcPadding.right;
        cxNeeded += sz.cx + rcPadding.left + rcPadding.right;
        szRemaining.cx -= sz.cx + m_iChildPadding + rcPadding.right;
    }
    cxNeeded += (nEstimateNum - 1) * m_iChildPadding;

    // Process the scrollbar
    ProcessScrollBar(rc, cxNeeded, cyNeeded);
}

void UIHorizontalLayout::DoPostPaint(HANDLE_DC hDC, const RECT &rcPaint) {
    if( (m_buttonState & UISTATE_CAPTURED) != 0 && !m_immMode ) {
        RECT rcSeparator = GetThumbRect(true);
        UIRenderEngine::DrawColor(hDC, rcSeparator, 0xAA000000);
    }
}

RECT UIHorizontalLayout::GetThumbRect(bool useNew) const {
    if( (m_buttonState & UISTATE_CAPTURED) != 0 && useNew) {
        if( m_sepWidth >= 0 ) return UIRect(m_rcNewPos.right - m_sepWidth, m_rcNewPos.top, m_rcNewPos.right, m_rcNewPos.bottom);
        else return UIRect(m_rcNewPos.left, m_rcNewPos.top, m_rcNewPos.left - m_sepWidth, m_rcNewPos.bottom);
    }
    else {
        if( m_sepWidth >= 0 ) return UIRect(m_rcItem.right - m_sepWidth, m_rcItem.top, m_rcItem.right, m_rcItem.bottom);
        else return UIRect(m_rcItem.left, m_rcItem.top, m_rcItem.left - m_sepWidth, m_rcItem.bottom);
    }
}
