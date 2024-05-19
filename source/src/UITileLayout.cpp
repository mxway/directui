#include <UITileLayout.h>
#include <UIScrollBar.h>
#include <cassert>

UITileLayout::UITileLayout()
    :m_columns{1},
    m_rows{0},
    m_columnsFixed{0},
    m_childVPadding{0},
    m_szItem {80, 80}
{

}

UIString UITileLayout::GetClass() const {
    return UIString{DUI_CTR_TILELAYOUT};
}

LPVOID UITileLayout::GetInterface(const UIString &name) {
    if(name == DUI_CTR_TILELAYOUT){
        return static_cast<UITileLayout*>(this);
    }
    return UIContainer::GetInterface(name);
}

void UITileLayout::SetPos(RECT rc, bool bNeedInvalidate) {
    UIControl::SetPos(rc, bNeedInvalidate);
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

    int nEstimateNum = 0;
    for( int it = 0; it < m_items.GetSize(); it++ ) {
        auto* pControl = static_cast<UIControl*>(m_items[it]);
        if( !pControl->IsVisible() ) continue;
        if( pControl->IsFloat() ) continue;
        nEstimateNum++;
    }

    int cxNeeded = 0;
    int cyNeeded = 0;
    int iChildPadding = m_iChildPadding;
    if (m_columnsFixed == 0) {
        if (rc.right - rc.left >= m_szItem.cx) {
            m_columns = (rc.right - rc.left)/m_szItem.cx;
            cxNeeded = rc.right - rc.left;
            if (m_columns > 1) {
                if (iChildPadding <= 0) {
                    iChildPadding = (cxNeeded-m_columns*m_szItem.cx)/(m_columns-1);
                }
                if (iChildPadding < 0) iChildPadding = 0;
            }
            else {
                iChildPadding = 0;
            }
        }
        else {
            m_columns = 1;
            cxNeeded = m_szItem.cx;
        }

        m_rows = (nEstimateNum-1)/m_columns+1;
        cyNeeded = m_rows*m_szItem.cy + (m_rows-1)*m_childVPadding;
    }
    else {
        m_columns = m_columnsFixed;
        if (m_columns > 1) {
            if (iChildPadding <= 0) {
                if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() && rc.right - rc.left >= m_columns*m_szItem.cx) {
                    iChildPadding = (rc.right - rc.left-m_columns*m_szItem.cx)/(m_columns-1);
                }
                else {
                    iChildPadding = (szAvailable.cx-m_columns*m_szItem.cx)/(m_columns-1);
                }
            }
            if (iChildPadding < 0) iChildPadding = 0;
        }
        else iChildPadding = 0;

        if (nEstimateNum >= m_columns) cxNeeded = m_columns*m_szItem.cx + (m_columns-1)*iChildPadding;
        else cxNeeded = nEstimateNum*m_szItem.cx + (nEstimateNum-1)*iChildPadding;
        m_rows = (nEstimateNum-1)/m_columns+1;
        cyNeeded = m_rows*m_szItem.cy + (m_rows-1)*m_childVPadding;
    }

    for( int it1 = 0; it1 < m_items.GetSize(); it1++ ) {
        auto* pControl = static_cast<UIControl*>(m_items[it1]);
        if( !pControl->IsVisible() ) continue;
        if( pControl->IsFloat() ) {
            SetFloatPos(it1);
            continue;
        }

        RECT rcPadding = pControl->GetPadding();
        SIZE sz = m_szItem;
        sz.cx -= rcPadding.left + rcPadding.right;
        sz.cy -= rcPadding.top + rcPadding.bottom;
        if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();
        if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
        if( sz.cx < 0) sz.cx = 0;
        if( sz.cy < 0) sz.cy = 0;

        uint32_t iChildAlign = GetChildAlign();
        uint32_t iChildVAlign = GetChildVAlign();
        int iColumnIndex = it1/m_columns;
        int iRowIndex = it1%m_columns;
        int iPosX = rc.left + iRowIndex*(m_szItem.cx+iChildPadding);
        if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
            iPosX -= m_pHorizontalScrollBar->GetScrollPos();
        }
        int iPosY = rc.top + iColumnIndex*(m_szItem.cy+m_childVPadding);
        if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
            iPosY -= m_pVerticalScrollBar->GetScrollPos();
        }
        if (iChildAlign == DT_CENTER) {
            if (iChildVAlign == DT_VCENTER) {
                RECT rcCtrl = { iPosX + (m_szItem.cx-sz.cx)/2+rcPadding.left, iPosY + (m_szItem.cy-sz.cy)/2+rcPadding.top, iPosX + (m_szItem.cx-sz.cx)/2 + sz.cx-rcPadding.right, iPosY + (m_szItem.cy-sz.cy)/2 + sz.cy-rcPadding.bottom };
                pControl->SetPos(rcCtrl, false);
            }
            else if (iChildVAlign == DT_BOTTOM) {
                RECT rcCtrl = { iPosX + (m_szItem.cx-sz.cx)/2+rcPadding.left, iPosY + m_szItem.cy - sz.cy+rcPadding.top, iPosX + (m_szItem.cx-sz.cx)/2 + sz.cx-rcPadding.right, iPosY + m_szItem.cy-rcPadding.bottom };
                pControl->SetPos(rcCtrl, false);
            }
            else {
                RECT rcCtrl = { iPosX + (m_szItem.cx-sz.cx)/2+rcPadding.left, iPosY+rcPadding.top, iPosX + (m_szItem.cx-sz.cx)/2 + sz.cx-rcPadding.right, iPosY + sz.cy-rcPadding.bottom };
                pControl->SetPos(rcCtrl, false);
            }
        }
        else if (iChildAlign == DT_RIGHT) {
            if (iChildVAlign == DT_VCENTER) {
                RECT rcCtrl = { iPosX + m_szItem.cx - sz.cx+rcPadding.left, iPosY + (m_szItem.cy-sz.cy)/2+rcPadding.top, iPosX + m_szItem.cx-rcPadding.right, iPosY + (m_szItem.cy-sz.cy)/2 + sz.cy-rcPadding.bottom };
                pControl->SetPos(rcCtrl, false);
            }
            else if (iChildVAlign == DT_BOTTOM) {
                RECT rcCtrl = { iPosX + m_szItem.cx - sz.cx+rcPadding.left, iPosY + m_szItem.cy - sz.cy+rcPadding.top, iPosX + m_szItem.cx-rcPadding.right, iPosY + m_szItem.cy-rcPadding.bottom };
                pControl->SetPos(rcCtrl, false);
            }
            else {
                RECT rcCtrl = { iPosX + m_szItem.cx - sz.cx+rcPadding.left, iPosY+rcPadding.top, iPosX + m_szItem.cx-rcPadding.right, iPosY + sz.cy-rcPadding.bottom };
                pControl->SetPos(rcCtrl, false);
            }
        }
        else {
            if (iChildVAlign == DT_VCENTER) {
                RECT rcCtrl = { iPosX+rcPadding.left, iPosY + (m_szItem.cy-sz.cy)/2+rcPadding.top, iPosX + sz.cx-rcPadding.right, iPosY + (m_szItem.cy-sz.cy)/2 + sz.cy-rcPadding.bottom };
                pControl->SetPos(rcCtrl, false);
            }
            else if (iChildVAlign == DT_BOTTOM) {
                RECT rcCtrl = { iPosX+rcPadding.left, iPosY + m_szItem.cy - sz.cy+rcPadding.top, iPosX + sz.cx-rcPadding.right, iPosY + m_szItem.cy-rcPadding.bottom };
                pControl->SetPos(rcCtrl, false);
            }
            else {
                RECT rcCtrl = { iPosX+rcPadding.left, iPosY+rcPadding.top, iPosX + sz.cx-rcPadding.right, iPosY + sz.cy-rcPadding.bottom };
                pControl->SetPos(rcCtrl, false);
            }
        }
    }

    // Process the scrollbar
    ProcessScrollBar(rc, cxNeeded, cyNeeded);
}

int UITileLayout::GetFixedColumns() const {
    return m_columnsFixed;
}

void UITileLayout::SetFixedColumns(int columns) {
    if(columns<0 || m_columns == columns){
        return;
    }
    m_columns = columns;
    NeedUpdate();
}

int UITileLayout::GetChildVPadding() const {
    return m_childVPadding;
}

void UITileLayout::SetChildVPadding(int padding) {
    if(padding == m_childVPadding || padding<0){
        return;
    }
    m_childVPadding = padding;
    NeedUpdate();
}

SIZE UITileLayout::GetItemSize() const {
    return m_szItem;
}

void UITileLayout::SetItemSize(SIZE size) {
    if( m_szItem.cx != size.cx || m_szItem.cy != size.cy ) {
        m_szItem = size;
        NeedUpdate();
    }
}

int UITileLayout::GetColumns() const {
    return m_columns;
}

int UITileLayout::GetRows() const {
    return m_rows;
}

void UITileLayout::SetAttribute(const char *pstrName, const char *pstrValue) {
    if( strcasecmp(pstrName, "itemsize") == 0 ) {
        SIZE szItem = { 0 };
        char *pstr = nullptr;
        szItem.cx = strtol(pstrValue, &pstr, 10);  assert(pstr);
        szItem.cy = strtol(pstr + 1, &pstr, 10);   assert(pstr);
        SetItemSize(szItem);
    }
    else if( strcasecmp(pstrName, "columns") == 0 ) SetFixedColumns(atoi(pstrValue));
    else if( strcasecmp(pstrName, "childvpadding") == 0 ) SetChildVPadding(atoi(pstrValue));
    else UIContainer::SetAttribute(pstrName, pstrValue);
}
