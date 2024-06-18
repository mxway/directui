#include <UIList.h>
#include <UIPaintManager.h>
#include <UIScrollBar.h>
#include <cassert>
#include <UIRenderClip.h>
#include <UIRenderEngine.h>
#include "UIResourceMgr.h"

class UIListBody : public UIVerticalLayout
{
public:
    explicit UIListBody(UIList *owner);

    void        SetScrollPos(SIZE szPos)override;
    void        SetPos(RECT rc, bool needInvalidate=true)override;
    void        DoEvent(TEventUI &event)override;
    bool        DoPaint(HANDLE_DC hDC, const RECT &rcPaint, UIControl *stopControl)override;
    bool        SortItems(PULVCompareFunc pfnCompare, LPVOID data, int curSel);
protected:
#ifdef WIN32
    static  int ItemCompareFunc(void *pvlocale, const void *item1, const void *item2);
#else
    static  int ItemCompareFunc(const void *item1, const void *item2,void *pvlocale);
#endif
    int         ItemCompareFunc(const void *item1, const void *item2);

protected:
    UIList              *m_owner;
    PULVCompareFunc     m_compareFunc;
    LPVOID              m_compareData;
};

UIList::UIList()
    :m_scrollSelect{false},
    m_curSel{-1},
    m_expandedItem{-1},
    m_callback{nullptr}
{
    m_pList = new UIListBody(this);
    m_pHeader = new UIListHeader;
    Add(m_pHeader);
    UIVerticalLayout::Add(m_pList);
    m_ListInfo.nColumns = 0;
    m_ListInfo.uFixedHeight = 0;
    m_ListInfo.nFont = -1;
    m_ListInfo.textStyle = DT_VCENTER|DT_SINGLELINE;
    m_ListInfo.dwTextColor = 0xFF000000;
    m_ListInfo.dwBkColor = 0;
    m_ListInfo.bAlternateBk = false;
    m_ListInfo.dwSelectedTextColor = 0xFF000000;
    m_ListInfo.dwSelectedBkColor = 0xFFC1E3FF;
    m_ListInfo.dwHotTextColor = 0xFF000000;
    m_ListInfo.dwHotBkColor = 0xFFE9F5FF;
    m_ListInfo.dwDisabledTextColor = 0xFFCCCCCC;
    m_ListInfo.dwDisabledBkColor = 0xFFFFFFFF;
    m_ListInfo.iHLineSize = 0;
    m_ListInfo.dwHLineColor = 0xFF3C3C3C;
    m_ListInfo.iVLineSize = 0;
    m_ListInfo.dwVLineColor = 0xFF3C3C3C;
    m_ListInfo.bShowHtml = false;
    m_ListInfo.bMultiExpandable = false;
    memset(&m_ListInfo.rcTextPadding, 0, sizeof(m_ListInfo.rcTextPadding));
    memset(&m_ListInfo.rcColumn, 0, sizeof(m_ListInfo.rcColumn));
}

UIString UIList::GetClass() const {
    return UIString{DUI_CTR_LIST};
}

LPVOID UIList::GetInterface(const UIString &name) {
    if(name == DUI_CTR_LIST){
        return static_cast<UIList*>(this);
    }
    if(name == DUI_CTR_ILIST){
        return static_cast<IListUI*>(this);
    }
    if(name == DUI_CTR_ILISTOWNER){
        return static_cast<IListOwnerUI*>(this);
    }
    return UIVerticalLayout::GetInterface(name);
}

uint32_t UIList::GetControlFlags() const {
    return UIFLAG_TABSTOP;
}

bool UIList::GetScrollSelect()const {
    return m_scrollSelect;
}

void UIList::SetScrollSelect(bool bScrollSelect) {
    m_scrollSelect = bScrollSelect;
}

int UIList::GetCurSel() const {
    return m_curSel;
}

bool UIList::SelectItem(int iIndex, bool bTakeFocus, bool bTriggerEvent) {
    if(iIndex == m_curSel){
        return true;
    }
    int iOldSel = m_curSel;
    // We should first unselect the currently selected item
    if( m_curSel >= 0 ) {
        UIControl* pControl = GetItemAt(m_curSel);
        if( pControl != nullptr) {
            IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(UIString{DUI_CTR_ILISTITEM}));
            if( pListItem != nullptr ) pListItem->Select(false, bTriggerEvent);
        }

        m_curSel = -1;
    }
    if( iIndex < 0 ) return false;

    UIControl* pControl = GetItemAt(iIndex);
    if( pControl == nullptr ) return false;
    if( !pControl->IsVisible() ) return false;
    if( !pControl->IsEnabled() ) return false;

    IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(UIString{DUI_CTR_ILISTITEM}));
    if( pListItem == nullptr ) return false;
    m_curSel = iIndex;
    if( !pListItem->Select(true, bTriggerEvent) ) {
        m_curSel = -1;
        return false;
    }
    EnsureVisible(m_curSel);
    if( bTakeFocus ) pControl->SetFocus();
    if( m_manager != nullptr && bTriggerEvent ) {
        m_manager->SendNotify(this, DUI_MSGTYPE_ITEMSELECT, (WPARAM)(long)m_curSel,(LPARAM)(long)iOldSel);
    }

    return true;
}

UIControl *UIList::GetItemAt(int iIndex) const {
    return m_pList->GetItemAt(iIndex);
}

int UIList::GetItemIndex(UIControl *control) const {
    if( control->GetInterface(UIString{DUI_CTR_LISTHEADER}) != nullptr ) return UIVerticalLayout::GetItemIndex(control);
    // We also need to recognize header sub-items
    if( control->GetClass().Find(DUI_CTR_LISTHEADERITEM) != -1 ) return m_pHeader->GetItemIndex(control);

    return m_pList->GetItemIndex(control);
}

bool UIList::SetItemIndex(UIControl *control, int iIndex) {
    if( control->GetInterface(UIString{DUI_CTR_LISTHEADER}) != nullptr ) return UIVerticalLayout::SetItemIndex(control, iIndex);
    // We also need to recognize header sub-items
    if( control->GetClass().Find(DUI_CTR_LISTHEADERITEM) != -1 ) return m_pHeader->SetItemIndex(control, iIndex);

    int iOrginIndex = m_pList->GetItemIndex(control);
    if( iOrginIndex == -1 ) return false;
    if( iOrginIndex == iIndex ) return true;

    IListItemUI* pSelectedListItem = nullptr;
    if( m_curSel >= 0 ) pSelectedListItem =
                                 static_cast<IListItemUI*>(GetItemAt(m_curSel)->GetInterface(UIString{DUI_CTR_ILISTITEM}));
    if( !m_pList->SetItemIndex(control, iIndex) ) return false;
    int iMinIndex = min(iOrginIndex, iIndex);
    int iMaxIndex = max(iOrginIndex, iIndex);
    for(int i = iMinIndex; i < iMaxIndex + 1; ++i) {
        UIControl* p = m_pList->GetItemAt(i);
        IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(UIString{DUI_CTR_ILISTITEM}));
        if( pListItem != nullptr ) {
            pListItem->SetIndex(i);
        }
    }
    if( m_curSel >= 0 && pSelectedListItem != nullptr ) m_curSel = pSelectedListItem->GetIndex();
    return true;
}

bool UIList::SetMultiItemIndex(UIControl *startControl, int iCount, int iNewStartIndex) {
    if (startControl == nullptr || iCount < 0 || iNewStartIndex < 0) return false;
    if( startControl->GetInterface(UIString{DUI_CTR_LISTHEADER}) != nullptr ) return UIVerticalLayout::SetMultiItemIndex(startControl, iCount, iNewStartIndex);
    // We also need to recognize header sub-items
    if( startControl->GetClass().Find(DUI_CTR_LISTHEADERITEM) != -1 ) return m_pHeader->SetMultiItemIndex(startControl, iCount, iNewStartIndex);

    int iStartIndex = GetItemIndex(startControl);
    if (iStartIndex == iNewStartIndex) return true;
    if (iStartIndex + iCount > GetCount()) return false;
    if (iNewStartIndex + iCount > GetCount()) return false;

    IListItemUI* pSelectedListItem = nullptr;
    if( m_curSel >= 0 ) pSelectedListItem =
                                 static_cast<IListItemUI*>(GetItemAt(m_curSel)->GetInterface(UIString{DUI_CTR_ILISTITEM}));
    if( !m_pList->SetMultiItemIndex(startControl, iCount, iNewStartIndex) ) return false;
    int iMinIndex = min(iStartIndex, iNewStartIndex);
    int iMaxIndex = max(iStartIndex + iCount, iNewStartIndex + iCount);
    for(int i = iMinIndex; i < iMaxIndex + 1; ++i) {
        UIControl* p = m_pList->GetItemAt(i);
        IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(UIString{DUI_CTR_ILISTITEM}));
        if( pListItem != nullptr ) {
            pListItem->SetIndex(i);
        }
    }
    if( m_curSel >= 0 && pSelectedListItem != nullptr ) m_curSel = pSelectedListItem->GetIndex();
    return true;
}

int UIList::GetCount() const {
    return m_pList->GetCount();
}

bool UIList::Add(UIControl *control) {
    // Override the Add() method so we can add items specifically to
    // the intended widgets. Headers are assumed to be
    // answer the correct interface so we can add multiple list headers.
    if( control->GetInterface(UIString{DUI_CTR_LISTHEADER}) != nullptr ) {
        if( m_pHeader != control && m_pHeader->GetCount() == 0 ) {
            UIVerticalLayout::Remove(m_pHeader);
            m_pHeader = dynamic_cast<UIListHeader*>(control);
        }
        m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);
        return UIVerticalLayout::AddAt(control, 0);
    }
    // We also need to recognize header sub-items
    if( control->GetClass().Find(DUI_CTR_LISTHEADERITEM) != -1 ) {
        bool ret = m_pHeader->Add(control);
        m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);
        return ret;
    }
    // The list items should know about us
    IListItemUI* pListItem = static_cast<IListItemUI*>(control->GetInterface(UIString{DUI_CTR_ILISTITEM}));
    if( pListItem != nullptr ) {
        pListItem->SetOwner(this);
        pListItem->SetIndex(GetCount());
    }
    return m_pList->Add(control);
}

bool UIList::AddAt(UIControl *control, int iIndex) {
    // Override the AddAt() method so we can add items specifically to
    // the intended widgets. Headers and are assumed to be
    // answer the correct interface so we can add multiple list headers.
    if( control->GetInterface(UIString{DUI_CTR_LISTHEADER}) != nullptr ) {
        if( m_pHeader != control && m_pHeader->GetCount() == 0 ) {
            UIVerticalLayout::Remove(m_pHeader);
            m_pHeader = dynamic_cast<UIListHeader*>(control);
        }
        m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);
        return UIVerticalLayout::AddAt(control, 0);
    }
    // We also need to recognize header sub-items
    if( control->GetClass().Find(DUI_CTR_LISTHEADERITEM) != -1 ) {
        bool ret = m_pHeader->AddAt(control, iIndex);
        m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);
        return ret;
    }
    if (!m_pList->AddAt(control, iIndex)) return false;

    // The list items should know about us
    IListItemUI* pListItem = static_cast<IListItemUI*>(control->GetInterface(UIString{DUI_CTR_ILISTITEM}));
    if( pListItem != nullptr ) {
        pListItem->SetOwner(this);
        pListItem->SetIndex(iIndex);
    }

    for(int i = iIndex + 1; i < m_pList->GetCount(); ++i) {
        UIControl* p = m_pList->GetItemAt(i);
        pListItem = static_cast<IListItemUI*>(p->GetInterface(UIString{DUI_CTR_ILISTITEM}));
        if( pListItem != nullptr ) {
            pListItem->SetIndex(i);
        }
    }
    if( m_curSel >= iIndex ) m_curSel += 1;
    return true;
}

bool UIList::Remove(UIControl *control, bool bDoNotDestroy) {
    if( control->GetInterface(UIString{DUI_CTR_LISTHEADER}) != nullptr ) return UIVerticalLayout::Remove(control, bDoNotDestroy);
    // We also need to recognize header sub-items
    if( control->GetClass().Find(DUI_CTR_LISTHEADERITEM) != -1 ) return m_pHeader->Remove(control, bDoNotDestroy);

    int iIndex = m_pList->GetItemIndex(control);
    if (iIndex == -1) return false;

    if (!m_pList->RemoveAt(iIndex, bDoNotDestroy)) return false;

    for(int i = iIndex; i < m_pList->GetCount(); ++i) {
        UIControl* p = m_pList->GetItemAt(i);
        IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(UIString{DUI_CTR_ILISTITEM}));
        if( pListItem != nullptr ) {
            pListItem->SetIndex(i);
        }
    }

    if( iIndex == m_curSel && m_curSel >= 0 ) {
        int iSel = m_curSel;
        m_curSel = -1;
        SelectItem(FindSelectable(iSel, false));
    }
    else if( iIndex < m_curSel ) m_curSel -= 1;
    return true;
}

bool UIList::RemoveAt(int iIndex, bool bDoNotDestroy) {
    if (!m_pList->RemoveAt(iIndex, bDoNotDestroy)) return false;

    for(int i = iIndex; i < m_pList->GetCount(); ++i) {
        UIControl* p = m_pList->GetItemAt(i);
        auto* pListItem = static_cast<IListItemUI*>(p->GetInterface(UIString{DUI_CTR_ILISTITEM}));
        if( pListItem != nullptr ) pListItem->SetIndex(i);
    }

    if( iIndex == m_curSel && m_curSel >= 0 ) {
        int iSel = m_curSel;
        m_curSel = -1;
        SelectItem(FindSelectable(iSel, false));
    }
    else if( iIndex < m_curSel ) m_curSel -= 1;
    return true;
}

void UIList::RemoveAll() {
    m_curSel = -1;
    m_expandedItem = -1;
    m_pList->RemoveAll();
}

void UIList::EnsureVisible(int iIndex) {
    if( m_curSel < 0 ) return;
    RECT rcItem = m_pList->GetItemAt(iIndex)->GetPos();
    RECT rcList = m_pList->GetPos();
    RECT rcListInset = m_pList->GetInset();

    rcList.left += rcListInset.left;
    rcList.top += rcListInset.top;
    rcList.right -= rcListInset.right;
    rcList.bottom -= rcListInset.bottom;

    UIScrollBar* pHorizontalScrollBar = m_pList->GetHorizontalScrollBar();
    if( pHorizontalScrollBar && pHorizontalScrollBar->IsVisible() ) rcList.bottom -= pHorizontalScrollBar->GetFixedHeight();

    long iPos = m_pList->GetScrollPos().cy;
    if( rcItem.top >= rcList.top && rcItem.bottom < rcList.bottom ) return;
    long dx = 0;
    if( rcItem.top < rcList.top ) dx = rcItem.top - rcList.top;
    if( rcItem.bottom > rcList.bottom ) dx = rcItem.bottom - rcList.bottom;
    Scroll(0, dx);
}

void UIList::Scroll(int dx, int dy) {
    if( dx == 0 && dy == 0 ) return;
    SIZE sz = m_pList->GetScrollPos();
    m_pList->SetScrollPos(SIZE{sz.cx + dx, sz.cy + dy});
}

int UIList::GetChildPadding() const {
    return m_pList->GetChildPadding();
}

void UIList::SetChildPadding(int iPadding) {
    m_pList->SetChildPadding(iPadding);
}

UIListHeader *UIList::GetHeader() const {
    return m_pHeader;
}

UIContainer *UIList::GetList() const {
    return m_pList;
}

TListInfoUI *UIList::GetListInfo() {
    return &m_ListInfo;
}

uint32_t UIList::GetItemFixedHeight() const {
    return m_ListInfo.uFixedHeight;
}

void UIList::SetItemFixedHeight(uint32_t nHeight) {
    m_ListInfo.uFixedHeight = nHeight;
    NeedUpdate();
}

int UIList::GetItemFont(int index) const {
    return m_ListInfo.nFont;
}

void UIList::SetItemFont(int index) {
    m_ListInfo.nFont = index;
    NeedUpdate();
}

uint32_t UIList::GetItemTextStyle() const {
    return m_ListInfo.textStyle;
}

void UIList::SetItemTextStyle(uint32_t style) {
    m_ListInfo.textStyle = style;
    NeedUpdate();
}

RECT UIList::GetItemTextPadding() const {
    return m_ListInfo.rcTextPadding;
}

void UIList::SetItemTextPadding(RECT rc) {
    m_ListInfo.rcTextPadding = rc;
    NeedUpdate();
}

uint32_t UIList::GetItemTextColor() const {
    return m_ListInfo.dwTextColor;
}

void UIList::SetItemTextColor(uint32_t textColor) {
    m_ListInfo.dwTextColor = textColor;
    Invalidate();
}

uint32_t UIList::GetItemBkColor() const {
    return m_ListInfo.dwBkColor;
}

void UIList::SetItemBkColor(uint32_t bkColor) {
    m_ListInfo.dwBkColor = bkColor;
    Invalidate();
}

UIString UIList::GetItemBkImage() const {
    return m_ListInfo.diBk.sDrawString;
}

void UIList::SetItemBkImage(const UIString &bkImage) {
    if( m_ListInfo.diBk.sDrawString == bkImage && m_ListInfo.diBk.pImageInfo != nullptr ) return;
    m_ListInfo.diBk.Clear();
    m_ListInfo.diBk.sDrawString = bkImage;
    Invalidate();
}

bool UIList::IsAlternateBk() const {
    return m_ListInfo.bAlternateBk;
}

void UIList::SetAlternateBk(bool alternateBk) {
    if(m_ListInfo.bAlternateBk == alternateBk){
        return;
    }
    m_ListInfo.bAlternateBk = alternateBk;
    Invalidate();
}

uint32_t UIList::GetSelectedItemTextColor() const {
    return m_ListInfo.dwSelectedTextColor;
}

void UIList::SetSelectedItemTextColor(uint32_t textColor) {
    if(m_ListInfo.dwSelectedTextColor == textColor){
        return;
    }
    m_ListInfo.dwSelectedTextColor = textColor;
    Invalidate();
}

uint32_t UIList::GetSelectedItemBkColor() const {
    return m_ListInfo.dwSelectedBkColor;
}

void UIList::SetSelectedItemBkColor(uint32_t bkColor) {
    m_ListInfo.dwSelectedBkColor = bkColor;
    Invalidate();
}

UIString UIList::GetSelectedItemImage() const {
    return m_ListInfo.diSelected.sDrawString;
}

void UIList::SetSelectedItemImage(const UIString &selectedImage) {
    if( m_ListInfo.diSelected.sDrawString == selectedImage && m_ListInfo.diSelected.pImageInfo != nullptr ) return;
    m_ListInfo.diSelected.Clear();
    m_ListInfo.diSelected.sDrawString = selectedImage;
    Invalidate();
}

uint32_t UIList::GetHotItemTextColor() const {
    return m_ListInfo.dwHotTextColor;
}

void UIList::SetHotItemTextColor(uint32_t textColor) {
    m_ListInfo.dwHotTextColor = textColor;
    Invalidate();
}

uint32_t UIList::GetHotItemBkColor() const {
    return m_ListInfo.dwHotBkColor;
}

void UIList::SetHotItemBkColor(uint32_t bkColor) {
    m_ListInfo.dwHotBkColor = bkColor;
    Invalidate();
}

UIString UIList::GetHotItemImage() const {
    return m_ListInfo.diHot.sDrawString;
}

void UIList::SetHotItemImage(const UIString &hotItemImage) {
    if( m_ListInfo.diHot.sDrawString == hotItemImage && m_ListInfo.diHot.pImageInfo != nullptr ) return;
    m_ListInfo.diHot.Clear();
    m_ListInfo.diHot.sDrawString = hotItemImage;
    Invalidate();
}

uint32_t UIList::GetDisabledItemTextColor() const {
    return m_ListInfo.dwDisabledTextColor;
}

void UIList::SetDisabledItemTextColor(uint32_t itemTextColor) {
    m_ListInfo.dwDisabledTextColor = itemTextColor;
    Invalidate();
}

uint32_t UIList::GetDisabledItemBkColor() const {
    return m_ListInfo.dwDisabledBkColor;
}

void UIList::SetDisabledItemBkColor(uint32_t bkColor) {
    m_ListInfo.dwDisabledBkColor = bkColor;
    Invalidate();
}

UIString UIList::GetDisabledItemImage() const {
    return m_ListInfo.diDisabled.sDrawString;
}

void UIList::SetDisabledItemImage(const UIString &image) {
    if( m_ListInfo.diDisabled.sDrawString == image && m_ListInfo.diDisabled.pImageInfo != nullptr ) return;
    m_ListInfo.diDisabled.Clear();
    m_ListInfo.diDisabled.sDrawString = image;
    Invalidate();
}

int UIList::GetItemHLineSize() const {
    return m_ListInfo.iHLineSize;
}

void UIList::SetItemHLineSize(int size) {
    m_ListInfo.iHLineSize = size;
    Invalidate();
}

uint32_t UIList::GetItemHLineColor() const {
    return m_ListInfo.dwHLineColor;
}

void UIList::SetItemHLineColor(uint32_t lineColor) {
    m_ListInfo.dwHLineColor = lineColor;
    Invalidate();
}

uint32_t UIList::GetItemVLineSize() const {
    return m_ListInfo.iVLineSize;
}

void UIList::SetItemVLineSize(int size) {
    m_ListInfo.iVLineSize = size;
    Invalidate();
}

uint32_t UIList::GetItemVLineColor() const {
    return m_ListInfo.dwVLineColor;
}

void UIList::SetItemVLineColor(uint32_t lineColor) {
    m_ListInfo.dwVLineColor = lineColor;
    Invalidate();
}

bool UIList::IsItemShowHtml() const {
    return m_ListInfo.bShowHtml;
}

void UIList::SetItemShowHtml(bool bShowHtml) {
    if( m_ListInfo.bShowHtml == bShowHtml ) return;

    m_ListInfo.bShowHtml = bShowHtml;
    NeedUpdate();
}

void UIList::SetMultiExpanding(bool bMultiExpandable) {
    m_ListInfo.bMultiExpandable = bMultiExpandable;
}

int UIList::GetExpandedItem() const {
    return m_expandedItem;
}

bool UIList::ExpandItem(int iIndex, bool bExpand) {
    if( m_expandedItem >= 0 && !m_ListInfo.bMultiExpandable) {
        UIControl* pControl = GetItemAt(m_expandedItem);
        if( pControl != nullptr ) {
            IListItemUI* pItem = static_cast<IListItemUI*>(pControl->GetInterface(UIString{DUI_CTR_ILISTITEM}));
            if( pItem != nullptr ) pItem->Expand(false);
        }
        m_expandedItem = -1;
    }
    if( bExpand ) {
        UIControl* pControl = GetItemAt(iIndex);
        if( pControl == nullptr ) return false;
        if( !pControl->IsVisible() ) return false;
        IListItemUI* pItem = static_cast<IListItemUI*>(pControl->GetInterface(UIString{DUI_CTR_ILISTITEM}));
        if( pItem == nullptr ) return false;
        m_expandedItem = iIndex;
        if( !pItem->Expand(true) ) {
            m_expandedItem = -1;
            return false;
        }
    }
    NeedUpdate();
    return true;
}

void UIList::SetPos(RECT rc, bool bNeedInvalidate) {
    if( m_pHeader != nullptr ) { // 设置header各子元素x坐标,因为有些listitem的setpos需要用到(临时修复)
        int iLeft = rc.left + m_rcInset.left;
        int iRight = rc.right - m_rcInset.right;

        m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);

        if( !m_pHeader->IsVisible() ) {
            for( int it = m_pHeader->GetCount() - 1; it >= 0; it-- ) {
                static_cast<UIControl*>(m_pHeader->GetItemAt(it))->SetInternVisible(true);
            }
        }
        m_pHeader->SetPos(RECT{iLeft, 0, iRight, 0}, false);
        int iOffset = m_pList->GetScrollPos().cx;
        for( int i = 0; i < m_ListInfo.nColumns; i++ ) {
            UIControl* pControl = static_cast<UIControl*>(m_pHeader->GetItemAt(i));
            if( !pControl->IsVisible() ) continue;
            if( pControl->IsFloat() ) continue;

            RECT rcPos = pControl->GetPos();
            if( iOffset > 0 ) {
                rcPos.left -= iOffset;
                rcPos.right -= iOffset;
                pControl->SetPos(rcPos, false);
            }
            m_ListInfo.rcColumn[i] = pControl->GetPos();
        }
        if( !m_pHeader->IsVisible() ) {
            for( int it = m_pHeader->GetCount() - 1; it >= 0; it-- ) {
                static_cast<UIControl*>(m_pHeader->GetItemAt(it))->SetInternVisible(false);
            }
            m_pHeader->SetInternVisible(false);
        }
    }

    UIVerticalLayout::SetPos(rc, bNeedInvalidate);

    if( m_pHeader == nullptr ) return;

    rc = m_rcItem;
    rc.left += m_rcInset.left;
    rc.top += m_rcInset.top;
    rc.right -= m_rcInset.right;
    rc.bottom -= m_rcInset.bottom;

    if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
        rc.top -= m_pVerticalScrollBar->GetScrollPos();
        rc.bottom -= m_pVerticalScrollBar->GetScrollPos();
        rc.bottom += m_pVerticalScrollBar->GetScrollRange();
        rc.right -= m_pVerticalScrollBar->GetFixedWidth();
    }
    if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
        rc.left -= m_pHorizontalScrollBar->GetScrollPos();
        rc.right -= m_pHorizontalScrollBar->GetScrollPos();
        rc.right += m_pHorizontalScrollBar->GetScrollRange();
        rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
    }

    m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);

    if( !m_pHeader->IsVisible() ) {
        for( int it = m_pHeader->GetCount() - 1; it >= 0; it-- ) {
            static_cast<UIControl*>(m_pHeader->GetItemAt(it))->SetInternVisible(true);
        }
        m_pHeader->SetPos(RECT{rc.left, 0, rc.right, 0}, false);
    }
    int iOffset = m_pList->GetScrollPos().cx;
    for( int i = 0; i < m_ListInfo.nColumns; i++ ) {
        auto* pControl = static_cast<UIControl*>(m_pHeader->GetItemAt(i));
        if( !pControl->IsVisible() ) continue;
        if( pControl->IsFloat() ) continue;

        RECT rcPos = pControl->GetPos();
        if( iOffset > 0 ) {
            rcPos.left -= iOffset;
            rcPos.right -= iOffset;
            pControl->SetPos(rcPos, false);
        }
        m_ListInfo.rcColumn[i] = pControl->GetPos();
    }
    if( !m_pHeader->IsVisible() ) {
        for( int it = m_pHeader->GetCount() - 1; it >= 0; it-- ) {
            static_cast<UIControl*>(m_pHeader->GetItemAt(it))->SetInternVisible(false);
        }
        m_pHeader->SetInternVisible(false);
    }
}

void UIList::Move(SIZE szOffset, bool bNeedInvalidate) {
    UIVerticalLayout::Move(szOffset, bNeedInvalidate);
    if( !m_pHeader->IsVisible() ) m_pHeader->Move(szOffset, false);
}

void UIList::DoEvent(TEventUI &event) {
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_parent != nullptr ) m_parent->DoEvent(event);
        else UIVerticalLayout::DoEvent(event);
        return;
    }

    if( event.Type == UIEVENT_SETFOCUS )
    {
        m_focused = true;
        return;
    }
    if( event.Type == UIEVENT_KILLFOCUS )
    {
        m_focused = false;
        return;
    }

    if( event.Type == UIEVENT_KEYDOWN )
    {
        if (IsKeyboardEnabled() && IsEnabled()) {
            switch( event.chKey ) {
                case VK_UP:
                    SelectItem(FindSelectable(m_curSel - 1, false), true); break;
                case VK_DOWN:
                    SelectItem(FindSelectable(m_curSel + 1, true), true); break;
                case VK_PRIOR:
                    PageUp(); break;
                case VK_NEXT:
                    PageDown(); break;
                case VK_HOME:
                    SelectItem(FindSelectable(0, false), true); break;
                case VK_END:
                    SelectItem(FindSelectable(GetCount() - 1, true), true); break;
                case VK_RETURN:
                    if( m_curSel != -1 ) GetItemAt(m_curSel)->Activate(); break;
            }
            return;
        }
    }

    if( event.Type == UIEVENT_SCROLLWHEEL )
    {
        if (IsEnabled()) {
            switch( LOWORD(event.wParam) ) {
                case SB_LINEUP:
                    if( m_scrollSelect ) SelectItem(FindSelectable(m_curSel - 1, false), true);
                    else LineUp();
                    return;
                case SB_LINEDOWN:
                    if( m_scrollSelect ) SelectItem(FindSelectable(m_curSel + 1, true), true);
                    else LineDown();
                    return;
            }
        }
    }

    UIVerticalLayout::DoEvent(event);
}

void UIList::SetAttribute(const char *pstrName, const char *pstrValue) {
    if( strcasecmp(pstrName, "header") == 0 ) GetHeader()->SetVisible(strcasecmp(pstrValue, "hidden") != 0);
    else if( strcasecmp(pstrName, "headerbkimage") == 0 ) GetHeader()->SetBkImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "scrollselect") == 0 ) SetScrollSelect(strcasecmp(pstrValue, "true") == 0);
    else if( strcasecmp(pstrName, "multiexpanding") == 0 ) SetMultiExpanding(strcasecmp(pstrValue, "true") == 0);
    else if( strcasecmp(pstrName, "itemheight") == 0 ) m_ListInfo.uFixedHeight = atoi(pstrValue);
    else if( strcasecmp(pstrName, "itemfont") == 0 ) m_ListInfo.nFont = atoi(pstrValue);
    else if( strcasecmp(pstrName, "itemalign") == 0 ) {
        if( strstr(pstrValue, "left") != nullptr ) {
            m_ListInfo.textStyle &= ~(DT_CENTER | DT_RIGHT);
            m_ListInfo.textStyle |= DT_LEFT;
        }
        if( strstr(pstrValue, "center") != nullptr ) {
            m_ListInfo.textStyle &= ~(DT_LEFT | DT_RIGHT);
            m_ListInfo.textStyle |= DT_CENTER;
        }
        if( strstr(pstrValue, "right") != nullptr ) {
            m_ListInfo.textStyle &= ~(DT_LEFT | DT_CENTER);
            m_ListInfo.textStyle |= DT_RIGHT;
        }
    }
    else if (strcasecmp(pstrName, "itemvalign") == 0)
    {
        if (strstr(pstrValue, "top") != nullptr) {
            m_ListInfo.textStyle &= ~(DT_BOTTOM | DT_VCENTER);
            m_ListInfo.textStyle |= DT_TOP;
        }
        if (strstr(pstrValue, "vcenter") != nullptr) {
            m_ListInfo.textStyle &= ~(DT_TOP | DT_BOTTOM);
            m_ListInfo.textStyle |= DT_VCENTER;
        }
        if (strstr(pstrValue, "bottom") != nullptr) {
            m_ListInfo.textStyle &= ~(DT_TOP | DT_VCENTER);
            m_ListInfo.textStyle |= DT_BOTTOM;
        }
    }
    else if( strcasecmp(pstrName, "itemendellipsis") == 0 ) {
        if( strcasecmp(pstrValue, "true") == 0 ) m_ListInfo.textStyle |= DT_END_ELLIPSIS;
        else m_ListInfo.textStyle &= ~DT_END_ELLIPSIS;
    }
    else if( strcasecmp(pstrName, "itemmultiline") == 0 ) {
        if (strcasecmp(pstrValue, "true") == 0) {
            m_ListInfo.textStyle &= ~DT_SINGLELINE;
            m_ListInfo.textStyle |= DT_WORDBREAK;
        }
        else m_ListInfo.textStyle |= DT_SINGLELINE;
    }
    else if( strcasecmp(pstrName, "itemtextpadding") == 0 ) {
        RECT rcTextPadding = { 0 };
        char *pstr = nullptr;
        rcTextPadding.left = strtol(pstrValue, &pstr, 10);  assert(pstr);
        rcTextPadding.top = strtol(pstr + 1, &pstr, 10);    assert(pstr);
        rcTextPadding.right = strtol(pstr + 1, &pstr, 10);  assert(pstr);
        rcTextPadding.bottom = strtol(pstr + 1, &pstr, 10); assert(pstr);
        SetItemTextPadding(rcTextPadding);
    }
    else if( strcasecmp(pstrName, "itemtextcolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetItemTextColor(clrColor);
    }
    else if( strcasecmp(pstrName, "itembkcolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetItemBkColor(clrColor);
    }
    else if( strcasecmp(pstrName, "itembkimage") == 0 ) SetItemBkImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "itemaltbk") == 0 ) SetAlternateBk(strcasecmp(pstrValue, "true") == 0);
    else if( strcasecmp(pstrName, "itemselectedtextcolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetSelectedItemTextColor(clrColor);
    }
    else if( strcasecmp(pstrName, "itemselectedbkcolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetSelectedItemBkColor(clrColor);
    }
    else if( strcasecmp(pstrName, "itemselectedimage") == 0 ) SetSelectedItemImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "itemhottextcolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetHotItemTextColor(clrColor);
    }
    else if( strcasecmp(pstrName, "itemhotbkcolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetHotItemBkColor(clrColor);
    }
    else if( strcasecmp(pstrName, "itemhotimage") == 0 ) SetHotItemImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "itemdisabledtextcolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetDisabledItemTextColor(clrColor);
    }
    else if( strcasecmp(pstrName, "itemdisabledbkcolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetDisabledItemBkColor(clrColor);
    }
    else if( strcasecmp(pstrName, "itemdisabledimage") == 0 ) SetDisabledItemImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "itemvlinesize") == 0 ) {
        SetItemVLineSize(atoi(pstrValue));
    }
    else if( strcasecmp(pstrName, "itemvlinecolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetItemVLineColor(clrColor);
    }
    else if( strcasecmp(pstrName, "itemhlinesize") == 0 ) {
        SetItemHLineSize(atoi(pstrValue));
    }
    else if( strcasecmp(pstrName, "itemhlinecolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetItemHLineColor(clrColor);
    }
    else if( strcasecmp(pstrName, "itemshowhtml") == 0 ) SetItemShowHtml(strcasecmp(pstrValue, "true") == 0);
    else UIVerticalLayout::SetAttribute(pstrName, pstrValue);
}

IListCallbackUI *UIList::GetTextCallback() const {
    return m_callback;
}

void UIList::SetTextCallback(IListCallbackUI *callback) {
    m_callback = callback;
}

SIZE UIList::GetScrollPos() const {
    return m_pList->GetScrollPos();
}

SIZE UIList::GetScrollRange() const {
    return m_pList->GetScrollRange();
}

void UIList::SetScrollPos(SIZE pos) {
    m_pList->SetScrollPos(pos);
}

void UIList::LineUp() {
    m_pList->LineUp();
}

void UIList::LineDown() {
    m_pList->LineDown();
}

void UIList::PageUp() {
    m_pList->PageUp();
}

void UIList::PageDown() {
    m_pList->PageDown();
}

void UIList::HomeUp() {
    m_pList->HomeUp();
}

void UIList::EndDown() {
    m_pList->EndDown();
}

void UIList::LineLeft() {
    m_pList->LineLeft();
}

void UIList::LineRight() {
    m_pList->LineRight();
}

void UIList::PageLeft() {
    m_pList->PageLeft();
}

void UIList::PageRight() {
    m_pList->PageRight();
}

void UIList::HomeLeft() {
    m_pList->HomeLeft();
}

void UIList::EndRight() {
    m_pList->EndRight();
}

void UIList::EnableScrollBar(bool bEnableVertical, bool bEnableHorizontal) {
    m_pList->EnableScrollBar(bEnableVertical, bEnableHorizontal);
}

UIScrollBar *UIList::GetVerticalScrollBar() const {
    return m_pList->GetVerticalScrollBar();
}

UIScrollBar *UIList::GetHorizontalScrollBar() const {
    return m_pList->GetHorizontalScrollBar();
}

bool UIList::SortItems(PULVCompareFunc pfnCompare, LPVOID data) {
    if (!m_pList) return false;
    int iCurSel = m_curSel;
    bool bResult = m_pList->SortItems(pfnCompare, data, iCurSel);
    if (bResult) {
        m_curSel = iCurSel;
        EnsureVisible(m_curSel);
        NeedUpdate();
    }
    return bResult;
}


/////////////// UIListBody

UIListBody::UIListBody(UIList *owner)
    :m_owner{owner}
{

}

void UIListBody::SetScrollPos(SIZE szPos) {
    int cx = 0;
    int cy = 0;
    if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
        int iLastScrollPos = m_pVerticalScrollBar->GetScrollPos();
        m_pVerticalScrollBar->SetScrollPos(szPos.cy);
        cy = m_pVerticalScrollBar->GetScrollPos() - iLastScrollPos;
    }

    if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
        int iLastScrollPos = m_pHorizontalScrollBar->GetScrollPos();
        m_pHorizontalScrollBar->SetScrollPos(szPos.cx);
        cx = m_pHorizontalScrollBar->GetScrollPos() - iLastScrollPos;
    }

    if( cx == 0 && cy == 0 ) return;

    for( int it2 = 0; it2 < m_items.GetSize(); it2++ ) {
        auto* pControl = static_cast<UIControl*>(m_items[it2]);
        if( !pControl->IsVisible() ) continue;
        if( pControl->IsFloat() ) continue;
        pControl->Move(SIZE{-cx, -cy}, false);
    }

    Invalidate();

    if( cx != 0 && m_owner ) {
        UIListHeader* pHeader = m_owner->GetHeader();
        if( pHeader == nullptr ) return;
        TListInfoUI* pInfo = m_owner->GetListInfo();
        pInfo->nColumns = MIN(pHeader->GetCount(), UILIST_MAX_COLUMNS);
        for( int i = 0; i < pInfo->nColumns; i++ ) {
            auto* pControl = static_cast<UIControl*>(pHeader->GetItemAt(i));
            if( !pControl->IsVisible() ) continue;
            if( pControl->IsFloat() ) continue;
            pControl->Move(SIZE{-cx, -cy}, false);
            pInfo->rcColumn[i] = pControl->GetPos();
        }
        pHeader->Invalidate();
    }
}

void UIListBody::SetPos(RECT rc, bool needInvalidate) {
    UIControl::SetPos(rc, needInvalidate);
    rc = m_rcItem;

    // Adjust for inset
    rc.left += m_rcInset.left;
    rc.top += m_rcInset.top;
    rc.right -= m_rcInset.right;
    rc.bottom -= m_rcInset.bottom;
    if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
    if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

    // Determine the minimum size
    SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
    if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() )
        szAvailable.cx += m_pHorizontalScrollBar->GetScrollRange();

    int iChildPadding = m_iChildPadding;
    TListInfoUI* pInfo = nullptr;
    if( m_owner ) {
        pInfo = m_owner->GetListInfo();
        if( pInfo != nullptr ) {
            iChildPadding += pInfo->iHLineSize;
            if (pInfo->nColumns > 0) {
                szAvailable.cx = pInfo->rcColumn[pInfo->nColumns - 1].right - pInfo->rcColumn[0].left;
            }
        }
    }

    int cxNeeded = 0;
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
        SIZE sz = pControl->EstimateSize(szAvailable);
        if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
        if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
        cyFixed += sz.cy + pControl->GetPadding().top + pControl->GetPadding().bottom;

        sz.cx = MAX(sz.cx, (long)0);
        if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
        if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();
        cxNeeded = MAX((long)cxNeeded, sz.cx);
        nEstimateNum++;
    }
    cyFixed += (nEstimateNum - 1) * iChildPadding;

    if( m_owner ) {
        UIListHeader* pHeader = m_owner->GetHeader();
        if( pHeader != nullptr && pHeader->GetCount() > 0 ) {
            cxNeeded = MAX((long)0, pHeader->EstimateSize(SIZE{rc.right - rc.left, rc.bottom - rc.top}).cx);
        }
    }

    // Place elements
    int cyNeeded = 0;
    // Position the elements
    SIZE szRemaining = szAvailable;
    int iPosY = rc.top;
    if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
        iPosY -= m_pVerticalScrollBar->GetScrollPos();
    }
    int iPosX = rc.left;
    if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
        iPosX -= m_pHorizontalScrollBar->GetScrollPos();
    }

    int iAdjustable = 0;
    for( int it2 = 0; it2 < m_items.GetSize(); it2++ ) {
        auto* pControl = static_cast<UIControl*>(m_items[it2]);
        if( !pControl->IsVisible() ) continue;
        if( pControl->IsFloat() ) {
            SetFloatPos(it2);
            continue;
        }

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
        SIZE sz = pControl->EstimateSize(szControlAvailable);
        if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
        if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
        sz.cx = pControl->GetMaxWidth();
        if( sz.cx == 0 ) sz.cx = szAvailable.cx - rcPadding.left - rcPadding.right;
        if( sz.cx < 0 ) sz.cx = 0;
        if( sz.cx > szControlAvailable.cx ) sz.cx = szControlAvailable.cx;
        if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();

        RECT rcCtrl = { iPosX + rcPadding.left, iPosY + rcPadding.top, iPosX + rcPadding.left + sz.cx, iPosY + sz.cy + rcPadding.top + rcPadding.bottom };
        pControl->SetPos(rcCtrl, false);

        iPosY += sz.cy + iChildPadding + rcPadding.top + rcPadding.bottom;
        cyNeeded += sz.cy + rcPadding.top + rcPadding.bottom;
        szRemaining.cy -= sz.cy + iChildPadding + rcPadding.bottom;
    }
    cyNeeded += (nEstimateNum - 1) * iChildPadding;

    // Process the scrollbar
    ProcessScrollBar(rc, cxNeeded, cyNeeded);
}

void UIListBody::DoEvent(TEventUI &event) {
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_owner != nullptr ) m_owner->DoEvent(event);
        else UIControl::DoEvent(event);
        return;
    }

    if( m_owner != nullptr ) {
        if (event.Type == UIEVENT_SCROLLWHEEL) {
            if (m_pHorizontalScrollBar != nullptr && m_pHorizontalScrollBar->IsVisible() && m_pHorizontalScrollBar->IsEnabled()) {
                //RECT rcHorizontalScrollBar = m_pHorizontalScrollBar->GetPos();
                UIRect rcHorizontalScrollBar{m_pHorizontalScrollBar->GetPos()};
                if( rcHorizontalScrollBar.IsPtIn(event.ptMouse) )
                {
                    switch( LOWORD(event.wParam) ) {
                        case SB_LINEUP:
                            m_owner->LineLeft();
                            return;
                        case SB_LINEDOWN:
                            m_owner->LineRight();
                            return;
                    }
                }
            }
        }
        m_owner->DoEvent(event); }
    else {
        UIControl::DoEvent(event);
    }
}

bool UIListBody::DoPaint(HANDLE_DC hDC, const RECT &rcPaint, UIControl *stopControl) {
    RECT rcTemp = { 0 };
    if( !UIIntersectRect(&rcTemp, &rcPaint, &m_rcItem) ) return true;

    TListInfoUI* pListInfo = nullptr;
    if( m_owner ) pListInfo = m_owner->GetListInfo();

    UIRenderClip clip;
    UIRenderClip::GenerateClip(hDC, rcTemp, clip);
    UIControl::DoPaint(hDC, rcPaint, stopControl);

    if( m_items.GetSize() > 0 ) {
        RECT rc = m_rcItem;
        rc.left += m_rcInset.left;
        rc.top += m_rcInset.top;
        rc.right -= m_rcInset.right;
        rc.bottom -= m_rcInset.bottom;
        if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
        if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

        if( !UIIntersectRect(&rcTemp, &rcPaint, &rc) ) {
            for( int it = 0; it < m_items.GetSize(); it++ ) {
                auto* pControl = static_cast<UIControl*>(m_items[it]);
                if( pControl == stopControl ) return false;
                if( !pControl->IsVisible() ) continue;
                if( !::UIIntersectRect(&rcTemp, &rcPaint, &pControl->GetPos()) ) continue;
                if( pControl->IsFloat() ) {
                    if( !::UIIntersectRect(&rcTemp, &m_rcItem, &pControl->GetPos()) ) continue;
                    if( !pControl->Paint(hDC, rcPaint, stopControl) ) return false;
                }
            }
        }
        else {
            int iDrawIndex = 0;
            UIRenderClip childClip;
            UIRenderClip::GenerateClip(hDC, rcTemp, childClip);
            for( int it = 0; it < m_items.GetSize(); it++ ) {
                auto* pControl = static_cast<UIControl*>(m_items[it]);
                if( pControl == stopControl ) return false;
                if( !pControl->IsVisible() ) continue;
                if( !pControl->IsFloat() ) {
                    IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(UIString{DUI_CTR_ILISTITEM}));
                    if( pListItem != nullptr ) {
                        pListItem->SetDrawIndex(iDrawIndex);
                        iDrawIndex += 1;
                    }
                    if (pListInfo && pListInfo->iHLineSize > 0) {
                        // 因为没有为最后一个预留分割条长度，如果list铺满，最后一条不会显示
                        RECT rcPadding = pControl->GetPadding();
                        const RECT& rcPos = pControl->GetPos();
                        RECT rcBottomLine = { rcPos.left, rcPos.bottom + rcPadding.bottom, rcPos.right, rcPos.bottom + rcPadding.bottom + pListInfo->iHLineSize };
                        if( ::UIIntersectRect(&rcTemp, &rcPaint, &rcBottomLine) ) {
                            rcBottomLine.top += pListInfo->iHLineSize / 2;
                            rcBottomLine.bottom = rcBottomLine.top;
                            UIRenderEngine::DrawLine(hDC, rcBottomLine, pListInfo->iHLineSize, GetAdjustColor(pListInfo->dwHLineColor),0);
                        }
                    }
                }
                if( !::UIIntersectRect(&rcTemp, &rcPaint, &pControl->GetPos()) ) continue;
                if( pControl->IsFloat() ) {
                    if( !::UIIntersectRect(&rcTemp, &m_rcItem, &pControl->GetPos()) ) continue;
                    UIRenderClip::UseOldClipBegin(hDC, childClip);
                    if( !pControl->Paint(hDC, rcPaint, stopControl) ) return false;
                    UIRenderClip::UseOldClipEnd(hDC, childClip);
                }
                else {
                    if( !::UIIntersectRect(&rcTemp, &rc, &pControl->GetPos()) ) continue;
                    if( !pControl->Paint(hDC, rcPaint, stopControl) ) return false;
                }
            }
        }
    }

    if( m_pVerticalScrollBar != nullptr ) {
        if( m_pVerticalScrollBar == stopControl ) return false;
        if (m_pVerticalScrollBar->IsVisible()) {
            if( ::UIIntersectRect(&rcTemp, &rcPaint, &m_pVerticalScrollBar->GetPos()) ) {
                if( !m_pVerticalScrollBar->Paint(hDC, rcPaint, stopControl) ) return false;
            }
        }
    }

    if( m_pHorizontalScrollBar != nullptr ) {
        if( m_pHorizontalScrollBar == stopControl ) return false;
        if (m_pHorizontalScrollBar->IsVisible()) {
            if( ::UIIntersectRect(&rcTemp, &rcPaint, &m_pHorizontalScrollBar->GetPos()) ) {
                if( !m_pHorizontalScrollBar->Paint(hDC, rcPaint, stopControl) ) return false;
            }
        }
    }
    return true;
}

bool UIListBody::SortItems(PULVCompareFunc pfnCompare, LPVOID data, int curSel) {
    if (!pfnCompare) return false;
    m_compareFunc = pfnCompare;
    m_compareData = data;
    UIControl *pCurSelControl = GetItemAt(curSel);
    auto **pData = (UIControl **)m_items.GetData();
#ifdef WIN32
    qsort_s(m_items.GetData(), m_items.GetSize(), sizeof(UIControl*), UIListBody::ItemCompareFunc, this);
#else
    qsort_r(m_items.GetData(), m_items.GetSize(), sizeof(UIControl*), UIListBody::ItemCompareFunc,this);
#endif
    if (pCurSelControl) curSel = GetItemIndex(pCurSelControl);
    IListItemUI *pItem = nullptr;
    for (int i = 0; i < m_items.GetSize(); ++i)
    {
        pItem = (IListItemUI*)(static_cast<UIControl*>(m_items[i])->GetInterface(UIString{"ListItem"}));
        if (pItem)
        {
            pItem->SetIndex(i);
        }
    }

    return true;
}

#ifdef WIN32
int UIListBody::ItemCompareFunc(void *pvlocale, const void *item1, const void *item2) {
#else
int UIListBody::ItemCompareFunc(const void *item1, const void *item2, void *pvlocale) {
#endif
    auto *pThis = (UIListBody*)pvlocale;
    if (!pThis || !item1 || !item2)
        return 0;
    return pThis->ItemCompareFunc(item1, item2);
}

int UIListBody::ItemCompareFunc(const void *item1, const void *item2) {
    UIControl *pControl1 = *(UIControl**)item1;
    UIControl *pControl2 = *(UIControl**)item2;
    return m_compareFunc((LPVOID)pControl1, (LPVOID)pControl2, m_compareData);
}


////////// UIListHeader

UIListHeader::UIListHeader()=default;

SIZE UIListHeader::EstimateSize(SIZE szAvailable) {
    SIZE cXY = {0, m_cxyFixed.cy};
    if( cXY.cy == 0 && m_manager != nullptr ) {
        for( int it = 0; it < m_items.GetSize(); it++ ) {
            cXY.cy = MAX(cXY.cy,static_cast<UIControl*>(m_items[it])->EstimateSize(szAvailable).cy);
        }
        long nMin = (long)UIResourceMgr::GetInstance().GetDefaultFontHeight(m_manager->GetPaintDC()) + 8;
        cXY.cy = MAX(cXY.cy,nMin);
    }

    for( int it = 0; it < m_items.GetSize(); it++ ) {
        cXY.cx +=  static_cast<UIControl*>(m_items[it])->EstimateSize(szAvailable).cx;
    }

    return cXY;
}

UIString UIListHeader::GetClass() const {
    return UIString{DUI_CTR_LISTHEADER};
}

LPVOID UIListHeader::GetInterface(const UIString &name) {
    if(name == DUI_CTR_LISTHEADER){
        return this;
    }
    return UIHorizontalLayout::GetInterface(name);
}


////////////// UIListHeaderItem

UIListHeaderItem::UIListHeaderItem()
    :m_dragable{true},
    m_buttonState{0},
    m_sepWidth{4},
    m_textStyle{DT_CENTER|DT_VCENTER|DT_SINGLELINE},
    m_textColor{0},
    m_sepColor{0},
    m_font{-1},
    m_showHtml{false},
    m_textPadding({2, 0, 2, 0}),
    ptLastMouse{0,0}
{
    SetMinWidth(16);
}

UIString UIListHeaderItem::GetClass() const {
    return UIString{DUI_CTR_LISTHEADERITEM};
}

LPVOID UIListHeaderItem::GetInterface(const UIString &name) {
    if(name == DUI_CTR_LISTHEADERITEM){
        return this;
    }
    return  UIControl::GetInterface(name);
}

uint32_t UIListHeaderItem::GetControlFlags() const {
    if(IsEnabled() && m_sepWidth != 0){
        return UIFLAG_SETCURSOR;
    }else{
        return 0;
    }
}

void UIListHeaderItem::SetEnabled(bool bEnabled) {
    UIControl::SetEnabled(bEnabled);
    if(!IsEnabled()){
        m_buttonState = 0;
    }
}

bool UIListHeaderItem::IsDraggable() const {
    return m_dragable;
}

void UIListHeaderItem::SetDraggable(bool draggable) {
    if(m_dragable == draggable){
        return;
    }
    m_dragable = draggable;
    if(!m_dragable){
        m_buttonState &= ~UISTATE_CAPTURED;
    }
}

uint32_t UIListHeaderItem::GetSepWidth() const {
    return m_sepWidth;
}

void UIListHeaderItem::SetSepWidth(int width) {
    if(m_sepWidth == width){
        return;
    }
    m_sepWidth = width;
}

uint32_t UIListHeaderItem::GetTextStyle() const {
    return m_textStyle;
}

void UIListHeaderItem::SetTextStyle(uint32_t style) {
    if(m_textStyle == style){
        return;
    }
    m_textStyle = style;
    Invalidate();
}

uint32_t UIListHeaderItem::GetTextColor() const {
    return m_textColor;
}

void UIListHeaderItem::SetTextColor(uint32_t textColor) {
    if(m_textColor == textColor){
        return;
    }
    m_textColor = textColor;
    Invalidate();
}

uint32_t UIListHeaderItem::GetSepColor() const {
    return m_sepColor;
}

void UIListHeaderItem::SetSepColor(uint32_t sepColor) {
    if(m_sepColor == sepColor){
        return;
    }
    m_sepColor = sepColor;
    Invalidate();
}

void UIListHeaderItem::SetTextPadding(RECT rc) {
    m_textPadding = rc;
    Invalidate();
}

RECT UIListHeaderItem::GetTextPadding() const {
    return m_textPadding;
}

void UIListHeaderItem::SetFont(int index) {
    m_font = index;
}

bool UIListHeaderItem::IsShowHtml() const {
    return m_showHtml;
}

void UIListHeaderItem::SetShowHtml(bool showHtml) {
    if( m_showHtml == showHtml ) return;

    m_showHtml = showHtml;
    Invalidate();
}

UIString UIListHeaderItem::GetNormalImage() const {
    return m_diNormal.sDrawString;
}

void UIListHeaderItem::SetNormalImage(const UIString &normalImage) {
    if( m_diNormal.sDrawString == normalImage && m_diNormal.pImageInfo != nullptr ) return;
    m_diNormal.Clear();
    m_diNormal.sDrawString = normalImage;
    Invalidate();
}

UIString UIListHeaderItem::GetHotImage() const {
    return m_diHot.sDrawString;
}

void UIListHeaderItem::SetHotImage(const UIString &hotImage) {
    if( m_diHot.sDrawString == hotImage && m_diHot.pImageInfo != nullptr ) return;
    m_diHot.Clear();
    m_diHot.sDrawString = hotImage;
    Invalidate();
}

UIString UIListHeaderItem::GetPushedImage() const {
    return m_diPushed.sDrawString;
}

void UIListHeaderItem::SetPushedImage(const UIString &pushedImage) {
    if( m_diPushed.sDrawString == pushedImage && m_diPushed.pImageInfo != nullptr ) return;
    m_diPushed.Clear();
    m_diPushed.sDrawString = pushedImage;
    Invalidate();
}

UIString UIListHeaderItem::GetFocusedImage() const {
    return m_diFocused.sDrawString;
}

void UIListHeaderItem::SetFocusedImage(const UIString &focusedImage) {
    if( m_diFocused.sDrawString == focusedImage && m_diFocused.pImageInfo != nullptr ) return;
    m_diFocused.Clear();
    m_diFocused.sDrawString = focusedImage;
    Invalidate();
}

UIString UIListHeaderItem::GetSepImage() const {
    return m_diSep.sDrawString;
}

void UIListHeaderItem::SetSepImage(const UIString &sepImage) {
    if( m_diSep.sDrawString == sepImage && m_diSep.pImageInfo != nullptr ) return;
    m_diSep.Clear();
    m_diSep.sDrawString = sepImage;
    Invalidate();
}

void UIListHeaderItem::DoEvent(TEventUI &event) {
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_parent != nullptr ) m_parent->DoEvent(event);
        else UIControl::DoEvent(event);
        return;
    }

    if( event.Type == UIEVENT_SETFOCUS )
    {
        Invalidate();
    }
    if( event.Type == UIEVENT_KILLFOCUS )
    {
        Invalidate();
    }
    if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK )
    {
        if( !IsEnabled() ) return;
        UIRect rcSeparator{GetThumbRect()};
        if( rcSeparator.IsPtIn(event.ptMouse) ) {
            if( m_dragable ) {
                m_buttonState |= UISTATE_CAPTURED;
                ptLastMouse = event.ptMouse;
            }
        }
        else {
            m_buttonState |= UISTATE_PUSHED;
            m_manager->SendNotify(this, DUI_MSGTYPE_HEADERCLICK);
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_BUTTONUP )
    {
        if( (m_buttonState & UISTATE_CAPTURED) != 0 ) {
            m_buttonState &= ~UISTATE_CAPTURED;
            if( GetParent() )
                GetParent()->NeedParentUpdate();
        }
        else if( (m_buttonState & UISTATE_PUSHED) != 0 ) {
            m_buttonState &= ~UISTATE_PUSHED;
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_MOUSEMOVE )
    {
        if( (m_buttonState & UISTATE_CAPTURED) != 0 ) {
            RECT rc = m_rcItem;
            if( m_sepWidth >= 0 ) {
                rc.right -= ptLastMouse.x - event.ptMouse.x;
            }
            else {
                rc.left -= ptLastMouse.x - event.ptMouse.x;
            }

            if( rc.right - rc.left > GetMinWidth() ) {
                m_cxyFixed.cx = rc.right - rc.left;
                ptLastMouse = event.ptMouse;
                if( GetParent() )
                    GetParent()->NeedParentUpdate();
            }
        }
        return;
    }
    if( event.Type == UIEVENT_SETCURSOR )
    {
        UIRect rcSeparator{GetThumbRect()};
        if( IsEnabled() && m_dragable && rcSeparator.IsPtIn(event.ptMouse) ) {
            UILoadCursor(m_manager, UI_IDC_RESIZEWE);
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
    UIControl::DoEvent(event);
}

SIZE UIListHeaderItem::EstimateSize(SIZE szAvailable) {
    if( m_cxyFixed.cy == 0 ) return SIZE{m_cxyFixed.cx, (long)(UIResourceMgr::GetInstance().GetDefaultFontHeight(m_manager->GetPaintDC()) + 8)};
    return UIControl::EstimateSize(szAvailable);
}

void UIListHeaderItem::SetAttribute(const char *pstrName, const char *pstrValue) {
    if( strcasecmp(pstrName, "dragable") == 0 ) SetDraggable(strcasecmp(pstrValue, "true") == 0);
    else if( strcasecmp(pstrName, "align") == 0 ) {
        if( strstr(pstrValue, "left") != nullptr ) {
            m_textStyle &= ~(DT_CENTER | DT_RIGHT);
            m_textStyle |= DT_LEFT;
        }
        if( strstr(pstrValue, "center") != nullptr ) {
            m_textStyle &= ~(DT_LEFT | DT_RIGHT);
            m_textStyle |= DT_CENTER;
        }
        if( strstr(pstrValue, "right") != nullptr ) {
            m_textStyle &= ~(DT_LEFT | DT_CENTER);
            m_textStyle |= DT_RIGHT;
        }
    }
    else if (strcasecmp(pstrName, "valign") == 0)
    {
        if (strstr(pstrValue, "top") != nullptr) {
            m_textStyle &= ~(DT_BOTTOM | DT_VCENTER);
            m_textStyle |= DT_TOP;
        }
        if (strstr(pstrValue, "vcenter") != nullptr) {
            m_textStyle &= ~(DT_TOP | DT_BOTTOM);
            m_textStyle |= DT_VCENTER;
        }
        if (strstr(pstrValue, "bottom") != nullptr) {
            m_textStyle &= ~(DT_TOP | DT_VCENTER);
            m_textStyle |= DT_BOTTOM;
        }
    }
    else if( strcasecmp(pstrName, "endellipsis") == 0 ) {
        if( strcasecmp(pstrValue, "true") == 0 ) m_textStyle |= DT_END_ELLIPSIS;
        else m_textStyle &= ~DT_END_ELLIPSIS;
    }
    else if( strcasecmp(pstrName, "multiline") == 0 ) {
        if (strcasecmp(pstrValue, "true") == 0) {
            m_textStyle  &= ~DT_SINGLELINE;
            m_textStyle |= DT_WORDBREAK;
        }
        else m_textStyle |= DT_SINGLELINE;
    }
    else if( strcasecmp(pstrName, "font") == 0 ) SetFont(atoi(pstrValue));
    else if( strcasecmp(pstrName, "textcolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetTextColor(clrColor);
    }
    else if( strcasecmp(pstrName, "textpadding") == 0 ) {
        RECT rcTextPadding = { 0 };
        char *pstr = nullptr;
        rcTextPadding.left = strtol(pstrValue, &pstr, 10);  assert(pstr);
        rcTextPadding.top = strtol(pstr + 1, &pstr, 10);    assert(pstr);
        rcTextPadding.right = strtol(pstr + 1, &pstr, 10);  assert(pstr);
        rcTextPadding.bottom = strtol(pstr + 1, &pstr, 10); assert(pstr);
        SetTextPadding(rcTextPadding);
    }
    else if( strcasecmp(pstrName, "showhtml") == 0 ) SetShowHtml(strcasecmp(pstrValue, "true") == 0);
    else if( strcasecmp(pstrName, "normalimage") == 0 ) SetNormalImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "hotimage") == 0 ) SetHotImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "pushedimage") == 0 ) SetPushedImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "focusedimage") == 0 ) SetFocusedImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "sepwidth") == 0 ) SetSepWidth(atoi(pstrValue));
    else if( strcasecmp(pstrName, "sepcolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetSepColor(clrColor);
    }
    else if( strcasecmp(pstrName, "sepimage") == 0 ) SetSepImage(UIString{pstrValue});
    else UIControl::SetAttribute(pstrName, pstrValue);
}

RECT UIListHeaderItem::GetThumbRect() const {
    if( m_sepWidth >= 0 ) return RECT{m_rcItem.right - m_sepWidth, m_rcItem.top, m_rcItem.right, m_rcItem.bottom};
    else return RECT{m_rcItem.left, m_rcItem.top, m_rcItem.left - m_sepWidth, m_rcItem.bottom};
}

void UIListHeaderItem::PaintText(HANDLE_DC hDC) {
    if( m_textColor == 0 ) m_textColor = m_manager->GetDefaultFontColor();

    RECT rcText = m_rcItem;
    rcText.left += m_textPadding.left;
    rcText.top += m_textPadding.top;
    rcText.right -= m_textPadding.right;
    rcText.bottom -= m_textPadding.bottom;

    if( m_text.IsEmpty() ) return;
    int nLinks = 0;
    if( m_showHtml )
        UIRenderEngine::DrawHtmlText(hDC, m_manager, rcText, m_text, m_textColor, \
        nullptr, nullptr, nLinks, m_font, m_textStyle);
    else
        UIRenderEngine::DrawText(hDC, m_manager, rcText, m_text, m_textColor, \
        m_font, m_textStyle);
}

void UIListHeaderItem::PaintStatusImage(HANDLE_DC hDC) {
    if( IsFocused() ) m_buttonState |= UISTATE_FOCUSED;
    else m_buttonState &= ~ UISTATE_FOCUSED;

    if( (m_buttonState & UISTATE_PUSHED) != 0 ) {
        if( !DrawImage(hDC, m_diPushed) )  DrawImage(hDC, m_diNormal);
    }
    else if( (m_buttonState & UISTATE_HOT) != 0 ) {
        if( !DrawImage(hDC, m_diHot) )  DrawImage(hDC, m_diNormal);
    }
    else if( (m_buttonState & UISTATE_FOCUSED) != 0 ) {
        if( !DrawImage(hDC, m_diFocused) )  DrawImage(hDC, m_diNormal);
    }
    else {
        DrawImage(hDC, m_diNormal);
    }

    if (m_sepWidth > 0) {
        RECT rcThumb = GetThumbRect();
        m_diSep.rcDestOffset.left = rcThumb.left - m_rcItem.left;
        m_diSep.rcDestOffset.top = rcThumb.top - m_rcItem.top;
        m_diSep.rcDestOffset.right = rcThumb.right - m_rcItem.left;
        m_diSep.rcDestOffset.bottom = rcThumb.bottom - m_rcItem.top;
        if( !DrawImage(hDC, m_diSep) ) {
            if (m_sepColor != 0) {
                RECT rcSepLine = { rcThumb.left + m_sepWidth/2, rcThumb.top, rcThumb.left + m_sepWidth/2, rcThumb.bottom};
                UIRenderEngine::DrawLine(hDC, rcSepLine, m_sepWidth, GetAdjustColor(m_sepColor),0);
            }
        }
    }
}

//////////////////// UIListElement


UIListElement::UIListElement()
    :m_index{-1},
    m_drawIndex{0},
    m_owner{nullptr},
    m_selected{false},
    m_buttonState{0}
{

}

UIString UIListElement::GetClass() const {
    return UIString{DUI_CTR_LISTELEMENT};
}

LPVOID UIListElement::GetInterface(const UIString &name) {
    if(name == DUI_CTR_ILISTITEM){
        return static_cast<IListItemUI*>(this);
    }
    if(name == DUI_CTR_LISTELEMENT){
        return static_cast<UIListElement*>(this);
    }
    return UIControl::GetInterface(name);
}

uint32_t UIListElement::GetControlFlags() const {
    return UIFLAG_WANTRETURN;
}

void UIListElement::SetEnabled(bool bEnabled) {
    UIControl::SetEnabled(bEnabled);
    if(!IsEnabled()){
        m_buttonState = 0;
    }
}

int UIListElement::GetIndex() const {
    return m_index;
}

void UIListElement::SetIndex(int index) {
    m_index = index;
}

int UIListElement::GetDrawIndex() const {
    return m_drawIndex;
}

void UIListElement::SetDrawIndex(int index) {
    m_drawIndex = index;
}

IListOwnerUI *UIListElement::GetOwner() {
    return m_owner;
}

void UIListElement::SetOwner(UIControl *owner) {
    if(owner != nullptr){
        m_owner = static_cast<IListOwnerUI*>(owner->GetInterface(UIString{DUI_CTR_ILISTOWNER}));
    }
}

void UIListElement::SetVisible(bool visible) {
    UIControl::SetVisible(visible);
    if(!IsVisible() && m_selected){
        m_selected = false;
        if(m_owner != nullptr){
            m_owner->SelectItem(-1);
        }
    }
}

bool UIListElement::IsSelected() const {
    return m_selected;
}

bool UIListElement::Select(bool select, bool bTriggerEvent) {
    if( !IsEnabled() ) return false;
    //if( select == m_selected ) return true;
    m_selected = select;
    if( select && m_owner != nullptr ) m_owner->SelectItem(m_index, bTriggerEvent);
    Invalidate();

    return true;
}

bool UIListElement::IsExpanded() const {
    return false;
}

bool UIListElement::Expand(bool expand) {
    return false;
}

void UIListElement::Invalidate() {
    if( !IsVisible() ) return;

    if( GetParent() ) {
        auto* pParentContainer = static_cast<UIContainer*>(GetParent()->GetInterface(UIString{DUI_CTR_CONTAINER}));
        if( pParentContainer ) {
            RECT rc = pParentContainer->GetPos();
            RECT rcInset = pParentContainer->GetInset();
            rc.left += rcInset.left;
            rc.top += rcInset.top;
            rc.right -= rcInset.right;
            rc.bottom -= rcInset.bottom;
            UIScrollBar* pVerticalScrollBar = pParentContainer->GetVerticalScrollBar();
            if( pVerticalScrollBar && pVerticalScrollBar->IsVisible() ) rc.right -= pVerticalScrollBar->GetFixedWidth();
            UIScrollBar* pHorizontalScrollBar = pParentContainer->GetHorizontalScrollBar();
            if( pHorizontalScrollBar && pHorizontalScrollBar->IsVisible() ) rc.bottom -= pHorizontalScrollBar->GetFixedHeight();

            RECT invalidateRc = m_rcItem;
            if( !::UIIntersectRect(&invalidateRc, &m_rcItem, &rc) )
            {
                return;
            }

            UIControl* pParent = GetParent();
            RECT rcTemp;
            RECT rcParent;
            while( (pParent = pParent->GetParent()) && pParent!=nullptr )
            {
                rcTemp = invalidateRc;
                rcParent = pParent->GetPos();
                if( !::UIIntersectRect(&invalidateRc, &rcTemp, &rcParent) )
                {
                    return;
                }
            }

            if( m_manager != nullptr ) m_manager->Invalidate(invalidateRc);
        }
        else {
            UIControl::Invalidate();
        }
    }
    else {
        UIControl::Invalidate();
    }
}

bool UIListElement::Activate() {
    if( !UIControl::Activate() ) return false;
    if( m_manager != nullptr ) m_manager->SendNotify(this, DUI_MSGTYPE_ITEMACTIVATE);
    return true;
}

void UIListElement::DoEvent(TEventUI &event) {
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_owner != nullptr ) m_owner->DoEvent(event);
        else UIControl::DoEvent(event);
        return;
    }

    if( event.Type == UIEVENT_DBLCLICK )
    {
        if( IsEnabled() ) {
            Activate();
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_KEYDOWN )
    {
        if (IsKeyboardEnabled() && IsEnabled()) {
            if( event.chKey == VK_RETURN ) {
                Activate();
                Invalidate();
                return;
            }
        }
    }
    // An important twist: The list-item will send the event not to its immediate
    // parent but to the "attached" list. A list may actually embed several components
    // in its path to the item, but key-presses etc. needs to go to the actual list.
    if( m_owner != nullptr ) m_owner->DoEvent(event); else UIControl::DoEvent(event);
}

void UIListElement::SetAttribute(const char *pstrName, const char *pstrValue) {
    if( strcasecmp(pstrName, "selected") == 0 ) Select();
    else UIControl::SetAttribute(pstrName, pstrValue);
}

void UIListElement::DrawItemBk(HANDLE_DC hDC, const RECT &rcItem) {
    assert(m_owner);
    if( m_owner == nullptr ) return;
    TListInfoUI* pInfo = m_owner->GetListInfo();
    if( pInfo == nullptr ) return;
    uint32_t iBackColor = 0;
    if( !pInfo->bAlternateBk || m_drawIndex % 2 == 0 ) iBackColor = pInfo->dwBkColor;
    if( (m_buttonState & UISTATE_HOT) != 0 ) {
        iBackColor = pInfo->dwHotBkColor;
    }
    if( IsSelected() ) {
        iBackColor = pInfo->dwSelectedBkColor;
    }
    if( !IsEnabled() ) {
        iBackColor = pInfo->dwDisabledBkColor;
    }

    if ( iBackColor != 0 ) {
        UIRenderEngine::DrawColor(hDC, rcItem, GetAdjustColor(iBackColor));
    }

    if( !IsEnabled() ) {
        if( DrawImage(hDC, pInfo->diDisabled) ) return;
    }
    if( IsSelected() ) {
        if( DrawImage(hDC, pInfo->diSelected) ) return;
    }
    if( (m_buttonState & UISTATE_HOT) != 0 ) {
        if( DrawImage(hDC, pInfo->diHot) ) return;
    }

    if( !DrawImage(hDC, m_diBk) ) {
        if( !pInfo->bAlternateBk || m_drawIndex % 2 == 0 ) {
            if( DrawImage(hDC, pInfo->diBk) ) return;
        }
    }
}


////////////////////////// UIListLabelElement

UIListLabelElement::UIListLabelElement()
    :m_needEstimateSize{true},
    m_fixedHeightLast{0},
    m_fontLast{-1},
    m_textStyleLast{0},
    m_szAvailableLast{0,0},
    m_cxyFixedLast{0,0},
    m_textPaddingLast{0,0,0,0}
{

}

UIString UIListLabelElement::GetClass() const {
    return UIString{DUI_CTR_LISTLABELELEMENT};
}

LPVOID UIListLabelElement::GetInterface(const UIString &name) {
    if(name == DUI_CTR_LISTLABELELEMENT){
        return static_cast<UIListLabelElement*>(this);
    }
    return UIListElement::GetInterface(name);
}

void UIListLabelElement::SetOwner(UIControl *owner) {
    m_needEstimateSize = true;
    UIListElement::SetOwner(owner);
}

void UIListLabelElement::SetFixedWidth(int cx) {
    m_needEstimateSize = true;
    UIControl::SetFixedWidth(cx);
}

void UIListLabelElement::SetFixedHeight(int cy) {
    m_needEstimateSize = true;
    UIControl::SetFixedHeight(cy);
}

void UIListLabelElement::SetText(const UIString &text) {
    m_needEstimateSize = true;
    UIControl::SetText(text);
}

void UIListLabelElement::DoEvent(TEventUI &event) {
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_owner != nullptr ) m_owner->DoEvent(event);
        else UIListElement::DoEvent(event);
        return;
    }

    if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_RBUTTONDOWN )
    {
        if( IsEnabled() ) {
            m_manager->SendNotify(this, DUI_MSGTYPE_ITEMCLICK);
            Select();
            Invalidate();
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
    UIListElement::DoEvent(event);
}

SIZE UIListLabelElement::EstimateSize(SIZE szAvailable) {
    if( m_owner == nullptr ) return SIZE{0, 0};
    TListInfoUI* pInfo = m_owner->GetListInfo();
    if (pInfo == nullptr) return SIZE{0, 0};
    if (m_cxyFixed.cx > 0) {
        if (m_cxyFixed.cy > 0) return m_cxyFixed;
        else if (pInfo->uFixedHeight > 0) return SIZE{m_cxyFixed.cx, (long)pInfo->uFixedHeight};
    }

    if ((pInfo->textStyle & DT_SINGLELINE) == 0 &&
        (szAvailable.cx != m_szAvailableLast.cx || szAvailable.cy != m_szAvailableLast.cy)) {
        m_needEstimateSize = true;
    }
    if (m_fixedHeightLast != pInfo->uFixedHeight || m_fontLast != pInfo->nFont ||
        m_textStyleLast != pInfo->textStyle ||
        m_textPaddingLast.left != pInfo->rcTextPadding.left || m_textPaddingLast.right != pInfo->rcTextPadding.right ||
        m_textPaddingLast.top != pInfo->rcTextPadding.top || m_textPaddingLast.bottom != pInfo->rcTextPadding.bottom) {
        m_needEstimateSize = true;
    }

    if (m_needEstimateSize) {
        m_needEstimateSize = false;
        m_szAvailableLast = szAvailable;
        m_fixedHeightLast = pInfo->uFixedHeight;
        m_fontLast = pInfo->nFont;
        m_textStyleLast = pInfo->textStyle;
        m_textPaddingLast = pInfo->rcTextPadding;

        m_cxyFixedLast = m_cxyFixed;
        if (m_cxyFixedLast.cy == 0) {
            m_cxyFixedLast.cy = (long)pInfo->uFixedHeight;
        }

        if ((pInfo->textStyle & DT_SINGLELINE) != 0) {
            if( m_cxyFixedLast.cy == 0 ) {
                uint32_t height = UIResourceMgr::GetInstance().GetFontHeight(pInfo->nFont,m_manager->GetPaintDC());
                m_cxyFixedLast.cy = (long)(height) + 8;
                m_cxyFixedLast.cy += pInfo->rcTextPadding.top + pInfo->rcTextPadding.bottom;
            }
            if (m_cxyFixedLast.cx == 0) {
                RECT rcText = { 0, 0, 9999, m_cxyFixedLast.cy };
                if( pInfo->bShowHtml ) {
                    int nLinks = 0;
                    UIRenderEngine::DrawHtmlText(m_manager->GetPaintDC(), m_manager, rcText, m_text, 0, nullptr, nullptr, nLinks, pInfo->nFont, DT_CALCRECT | pInfo->textStyle & ~DT_RIGHT & ~DT_CENTER);
                }
                else {
                    UIRenderEngine::DrawText(m_manager->GetPaintDC(), m_manager, rcText, m_text, 0, pInfo->nFont, DT_CALCRECT | pInfo->textStyle & ~DT_RIGHT & ~DT_CENTER);
                }
                m_cxyFixedLast.cx = rcText.right - rcText.left + pInfo->rcTextPadding.left + pInfo->rcTextPadding.right;
            }
        }
        else {
            if( m_cxyFixedLast.cx == 0 ) {
                m_cxyFixedLast.cx = szAvailable.cx;
            }
            RECT rcText = { 0, 0, m_cxyFixedLast.cx, 9999 };
            rcText.left += pInfo->rcTextPadding.left;
            rcText.right -= pInfo->rcTextPadding.right;
            if( pInfo->bShowHtml ) {
                int nLinks = 0;
                UIRenderEngine::DrawHtmlText(m_manager->GetPaintDC(), m_manager, rcText, m_text, 0, nullptr, nullptr, nLinks, pInfo->nFont, DT_CALCRECT | pInfo->textStyle & ~DT_RIGHT & ~DT_CENTER);
            }
            else {
                UIRenderEngine::DrawText(m_manager->GetPaintDC(), m_manager, rcText, m_text, 0, pInfo->nFont, DT_CALCRECT | pInfo->textStyle & ~DT_RIGHT & ~DT_CENTER);
            }
            m_cxyFixedLast.cy = rcText.bottom - rcText.top + pInfo->rcTextPadding.top + pInfo->rcTextPadding.bottom;
        }
    }
    return m_cxyFixedLast;
}

bool UIListLabelElement::DoPaint(HANDLE_DC hDC, const RECT &rcPaint, UIControl *stopControl) {
    DrawItemBk(hDC, m_rcItem);
    DrawItemText(hDC, m_rcItem);
    return true;
}

void UIListLabelElement::DrawItemText(HANDLE_DC hDC, const RECT &rcItem) {
    if( m_text.IsEmpty() ) return;

    if( m_owner == nullptr ) return;
    TListInfoUI* pInfo = m_owner->GetListInfo();
    if( pInfo == nullptr ) return;
    uint32_t iTextColor = pInfo->dwTextColor;
    if( (m_buttonState & UISTATE_HOT) != 0 ) {
        iTextColor = pInfo->dwHotTextColor;
    }
    if( IsSelected() ) {
        iTextColor = pInfo->dwSelectedTextColor;
    }
    if( !IsEnabled() ) {
        iTextColor = pInfo->dwDisabledTextColor;
    }
    int nLinks = 0;
    RECT rcText = rcItem;
    rcText.left += pInfo->rcTextPadding.left;
    rcText.right -= pInfo->rcTextPadding.right;
    rcText.top += pInfo->rcTextPadding.top;
    rcText.bottom -= pInfo->rcTextPadding.bottom;

    if( pInfo->bShowHtml )
        UIRenderEngine::DrawHtmlText(hDC, m_manager, rcText, m_text, iTextColor, \
        nullptr, nullptr, nLinks, pInfo->nFont, pInfo->textStyle);
    else
        UIRenderEngine::DrawText(hDC, m_manager, rcText, m_text, iTextColor, \
        pInfo->nFont, pInfo->textStyle);
}

///////////////// UIListTextElement

UIListTextElement::UIListTextElement()
    :m_links{0},
    m_hoverLink{-1},
     m_ListOwner{nullptr},
    m_rcLinks{0}
{

}

UIListTextElement::~UIListTextElement() {
    UIString* pText;
    for( int it = 0; it < m_texts.GetSize(); it++ ) {
        pText = static_cast<UIString*>(m_texts[it]);
        delete pText;
    }
    m_texts.Empty();
}

uint32_t UIListTextElement::GetControlFlags() const {
    return UIFLAG_WANTRETURN | ( (IsEnabled() && m_links > 0) ? UIFLAG_SETCURSOR : 0);
}

UIString UIListTextElement::GetClass() const {
    return UIString{DUI_CTR_LISTTEXTELEMENT};
}

LPVOID UIListTextElement::GetInterface(const UIString &name) {
    if( name == DUI_CTR_LISTTEXTELEMENT ) return static_cast<UIListTextElement*>(this);
    return UIListLabelElement::GetInterface(name);
}

UIString UIListTextElement::GetText(int index) const {
    auto* pText = static_cast<UIString*>(m_texts.GetAt(index));
    if( pText ) return *pText;
    return UIString{""};
}

void UIListTextElement::SetText(int index, const UIString &text) {
    if( m_owner == nullptr ) return;
    TListInfoUI* pInfo = m_owner->GetListInfo();
    if( index < 0 || index >= pInfo->nColumns ) return;
    m_needEstimateSize = true;

    while( m_texts.GetSize() < pInfo->nColumns ) { m_texts.Add(nullptr); }

    auto* pText = static_cast<UIString*>(m_texts[index]);
    if( (pText == nullptr && text.IsEmpty()) || (pText && *pText == text) ) return;

    if ( pText ) //by cddjr 2011/10/20
        pText->Assign(text.GetData());
    else
        m_texts.SetAt(index, new UIString(text));
    Invalidate();
}

void UIListTextElement::SetOwner(UIControl *owner) {
    if (owner != nullptr) {
        m_needEstimateSize = true;
        UIListLabelElement::SetOwner(owner);
        m_ListOwner = static_cast<IListUI*>(owner->GetInterface(UIString{DUI_CTR_ILIST}));
    }
}

UIString *UIListTextElement::GetLinkContent(int index) {
    if( index >= 0 && index < m_links ) return &m_sLinks[index];
    return nullptr;
}

void UIListTextElement::DoEvent(TEventUI &event) {
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_owner != nullptr ) m_owner->DoEvent(event);
        else UIListLabelElement::DoEvent(event);
        return;
    }

    // When you hover over a link
    if( event.Type == UIEVENT_SETCURSOR ) {
        for( int i = 0; i < m_links; i++ ) {
            UIRect rcLink{m_rcLinks[i]};
            if( rcLink.IsPtIn(event.ptMouse) ) {
                UILoadCursor(m_manager, UI_IDC_HAND);
                return;
            }
        }
    }
    if( event.Type == UIEVENT_BUTTONUP && IsEnabled() ) {
        for( int i = 0; i < m_links; i++ ) {
            UIRect rcLink{m_rcLinks[i]};
            if( rcLink.IsPtIn(event.ptMouse) ) {
                m_manager->SendNotify(this, DUI_MSGTYPE_LINK, (WPARAM)(long)i);
                return;
            }
        }
    }
    if( m_links > 0 && event.Type == UIEVENT_MOUSEMOVE ) {
        int nHoverLink = -1;
        for( int i = 0; i < m_links; i++ ) {
            UIRect rcLink{m_rcLinks[i]};
            if( rcLink.IsPtIn(event.ptMouse) ) {
                nHoverLink = i;
                break;
            }
        }

        if(m_hoverLink != nHoverLink) {
            Invalidate();
            m_hoverLink = nHoverLink;
        }
    }
    if( m_links > 0 && event.Type == UIEVENT_MOUSELEAVE ) {
        if(m_hoverLink != -1) {
            UIRect rcLink{m_rcLinks[m_hoverLink]};
            if( !rcLink.IsPtIn(event.ptMouse) ) {
                m_hoverLink = -1;
                Invalidate();
                if (m_manager) m_manager->RemoveMouseLeaveNeeded(this);
            }
            else {
                if (m_manager) m_manager->AddMouseLeaveNeeded(this);
                return;
            }
        }
    }
    UIListLabelElement::DoEvent(event);
}

SIZE UIListTextElement::EstimateSize(SIZE szAvailable) {
    if( m_owner == nullptr ) return SIZE{0, 0};
    TListInfoUI* pInfo = m_owner->GetListInfo();
    if (pInfo == nullptr) return SIZE{0, 0};
    SIZE cxyFixed = m_cxyFixed;
    if (cxyFixed.cx == 0 && pInfo->nColumns > 0) {
        cxyFixed.cx = pInfo->rcColumn[pInfo->nColumns - 1].right - pInfo->rcColumn[0].left;
        if (m_cxyFixedLast.cx != cxyFixed.cx) m_needEstimateSize = true;
    }
    if (cxyFixed.cx > 0) {
        if (cxyFixed.cy > 0) return cxyFixed;
        else if (pInfo->uFixedHeight > 0) return SIZE{cxyFixed.cx, (long)pInfo->uFixedHeight};
    }

    if ((pInfo->textStyle & DT_SINGLELINE) == 0 &&
        (szAvailable.cx != m_szAvailableLast.cx || szAvailable.cy != m_szAvailableLast.cy)) {
        m_needEstimateSize = true;
    }
    if (m_fixedHeightLast != pInfo->uFixedHeight || m_fontLast != pInfo->nFont ||
        m_textStyleLast != pInfo->textStyle ||
        m_textPaddingLast.left != pInfo->rcTextPadding.left || m_textPaddingLast.right != pInfo->rcTextPadding.right ||
            m_textPaddingLast.top != pInfo->rcTextPadding.top || m_textPaddingLast.bottom != pInfo->rcTextPadding.bottom) {
        m_needEstimateSize = true;
    }

    UIString strText;
    IListCallbackUI* pCallback = m_ListOwner->GetTextCallback();
    if( pCallback ) strText = pCallback->GetItemText(this, m_index, 0);
    else if (m_texts.GetSize() > 0) strText.Assign(GetText(0).GetData());
    else strText = m_text;
    if (m_textLast != strText) m_needEstimateSize = true;

    if (m_needEstimateSize) {
        m_needEstimateSize = false;
        m_szAvailableLast = szAvailable;
        m_fixedHeightLast = pInfo->uFixedHeight;
        m_fontLast = pInfo->nFont;
        m_textStyleLast = pInfo->textStyle;
        m_textPaddingLast = pInfo->rcTextPadding;
        m_textLast = strText;

        m_cxyFixedLast = m_cxyFixed;
        if (m_cxyFixedLast.cx == 0 && pInfo->nColumns > 0) {
            m_cxyFixedLast.cx = pInfo->rcColumn[pInfo->nColumns - 1].right - pInfo->rcColumn[0].left;
        }
        if (m_cxyFixedLast.cy == 0) {
            m_cxyFixedLast.cy = pInfo->uFixedHeight;
        }

        if ((pInfo->textStyle & DT_SINGLELINE) != 0) {
            if( m_cxyFixedLast.cy == 0 ) {
                long height = (long)UIResourceMgr::GetInstance().GetFontHeight(pInfo->nFont, m_manager->GetPaintDC());
                m_cxyFixedLast.cy = height + 8;
                m_cxyFixedLast.cy += pInfo->rcTextPadding.top + pInfo->rcTextPadding.bottom;
            }
            if (m_cxyFixedLast.cx == 0) {
                RECT rcText = { 0, 0, 9999, m_cxyFixedLast.cy };
                if( pInfo->bShowHtml ) {
                    int nLinks = 0;
                    UIRenderEngine::DrawHtmlText(m_manager->GetPaintDC(), m_manager, rcText, strText, 0, nullptr, nullptr, nLinks, pInfo->nFont, DT_CALCRECT | pInfo->textStyle & ~DT_RIGHT & ~DT_CENTER);
                }
                else {
                    UIRenderEngine::DrawText(m_manager->GetPaintDC(), m_manager, rcText, strText, 0, pInfo->nFont, DT_CALCRECT | pInfo->textStyle & ~DT_RIGHT & ~DT_CENTER);
                }
                m_cxyFixedLast.cx = rcText.right - rcText.left + pInfo->rcTextPadding.left + pInfo->rcTextPadding.right;
            }
        }
        else {
            if( m_cxyFixedLast.cx == 0 ) {
                m_cxyFixedLast.cx = szAvailable.cx;
            }
            RECT rcText = { 0, 0, m_cxyFixedLast.cx, 9999 };
            rcText.left += pInfo->rcTextPadding.left;
            rcText.right -= pInfo->rcTextPadding.right;
            if( pInfo->bShowHtml ) {
                int nLinks = 0;
                UIRenderEngine::DrawHtmlText(m_manager->GetPaintDC(), m_manager, rcText, strText, 0, nullptr, nullptr, nLinks, pInfo->nFont, DT_CALCRECT | pInfo->textStyle & ~DT_RIGHT & ~DT_CENTER);
            }
            else {
                UIRenderEngine::DrawText(m_manager->GetPaintDC(), m_manager, rcText, strText, 0, pInfo->nFont, DT_CALCRECT | pInfo->textStyle & ~DT_RIGHT & ~DT_CENTER);
            }
            m_cxyFixedLast.cy = rcText.bottom - rcText.top + pInfo->rcTextPadding.top + pInfo->rcTextPadding.bottom;
        }
    }
    return m_cxyFixedLast;
}

void UIListTextElement::DrawItemText(HANDLE_DC hDC, const RECT &rc) {
    if( m_owner == nullptr ) return;
    TListInfoUI* pInfo = m_owner->GetListInfo();
    if (pInfo == nullptr) return;
    uint32_t iTextColor = pInfo->dwTextColor;

    if( (m_buttonState & UISTATE_HOT) != 0 ) {
        iTextColor = pInfo->dwHotTextColor;
    }
    if( IsSelected() ) {
        iTextColor = pInfo->dwSelectedTextColor;
    }
    if( !IsEnabled() ) {
        iTextColor = pInfo->dwDisabledTextColor;
    }
    IListCallbackUI* pCallback = m_ListOwner->GetTextCallback();

    m_links = 0;
    int nLinks = (sizeof(m_rcLinks)/sizeof(*m_rcLinks));
    if (pInfo->nColumns > 0) {
        for( int i = 0; i < pInfo->nColumns; i++ )
        {
            RECT rcItem = { pInfo->rcColumn[i].left, m_rcItem.top, pInfo->rcColumn[i].right, m_rcItem.bottom };
            if (pInfo->iVLineSize > 0 && i < pInfo->nColumns - 1) {
                RECT rcLine = { rcItem.right - pInfo->iVLineSize / 2, rcItem.top, rcItem.right - pInfo->iVLineSize / 2, rcItem.bottom};
                UIRenderEngine::DrawLine(hDC, rcLine, pInfo->iVLineSize, GetAdjustColor(pInfo->dwVLineColor),0);
                rcItem.right -= pInfo->iVLineSize;
            }

            rcItem.left += pInfo->rcTextPadding.left;
            rcItem.right -= pInfo->rcTextPadding.right;
            rcItem.top += pInfo->rcTextPadding.top;
            rcItem.bottom -= pInfo->rcTextPadding.bottom;

            UIString strText;//不使用LPCTSTR，否则限制太多 by cddjr 2011/10/20
            if( pCallback ) strText = pCallback->GetItemText(this, m_index, i);
            else strText.Assign(GetText(i).GetData());
            if( pInfo->bShowHtml )
                UIRenderEngine::DrawHtmlText(hDC, m_manager, rcItem, strText, iTextColor, \
                &m_rcLinks[m_links], &m_sLinks[m_links], nLinks, pInfo->nFont, pInfo->textStyle);
            else
                UIRenderEngine::DrawText(hDC, m_manager, rcItem, strText, iTextColor, \
                pInfo->nFont, pInfo->textStyle);

            m_links += nLinks;
            nLinks = (int)(sizeof(m_rcLinks)/sizeof(*m_rcLinks)) - m_links;
        }
    }
    else {
        RECT rcItem = m_rcItem;
        rcItem.left += pInfo->rcTextPadding.left;
        rcItem.right -= pInfo->rcTextPadding.right;
        rcItem.top += pInfo->rcTextPadding.top;
        rcItem.bottom -= pInfo->rcTextPadding.bottom;

        UIString strText;
        if( pCallback ) strText = pCallback->GetItemText(this, m_index, 0);
        else if (m_texts.GetSize() > 0) strText.Assign(GetText(0).GetData());
        else strText = m_text;
        if( pInfo->bShowHtml )
            UIRenderEngine::DrawHtmlText(hDC, m_manager, rcItem, strText, iTextColor, \
            &m_rcLinks[m_links], &m_sLinks[m_links], nLinks, pInfo->nFont, pInfo->textStyle);
        else
            UIRenderEngine::DrawText(hDC, m_manager, rcItem, strText, iTextColor, \
            pInfo->nFont, pInfo->textStyle);

        m_links += nLinks;
        nLinks = (int)(sizeof(m_rcLinks)/sizeof(*m_rcLinks)) - m_links;
    }

    for( int i = m_links; i < (int)(sizeof(m_rcLinks)/sizeof(*m_rcLinks)); i++ ) {
        memset(m_rcLinks+i, 0, sizeof(RECT));
        ((UIString*)(m_sLinks + i))->Empty();
    }
}

////////  UIListContainerElement

UIListContainerElement::UIListContainerElement()
    :m_index{-1},
    m_drawIndex{0},
    m_owner{nullptr},
    m_selected{false},
    m_expandable{false},
    m_expand{false},
    m_buttonState{0}
{

}

UIString UIListContainerElement::GetClass() const {
    return UIString{DUI_CTR_LISTCONTAINERELEMENT};
}

uint32_t UIListContainerElement::GetControlFlags() const {
    return UIFLAG_WANTRETURN;
}

LPVOID UIListContainerElement::GetInterface(const UIString &name) {
    if(name == DUI_CTR_ILISTITEM){
        return static_cast<IListItemUI*>(this);
    }
    if(name == DUI_CTR_LISTCONTAINERELEMENT){
        return static_cast<UIListContainerElement*>(this);
    }
    return UIContainer::GetInterface(name);
}

int UIListContainerElement::GetIndex() const {
    return m_index;
}

void UIListContainerElement::SetIndex(int index) {
    m_index = index;
}

int UIListContainerElement::GetDrawIndex() const {
    return m_drawIndex;
}

void UIListContainerElement::SetDrawIndex(int index) {
    m_drawIndex = index;
}

IListOwnerUI *UIListContainerElement::GetOwner() {
    return m_owner;
}

void UIListContainerElement::SetOwner(UIControl *owner) {
    if(owner){
        m_owner = static_cast<IListOwnerUI*>(owner->GetInterface(UIString{DUI_CTR_ILISTOWNER}));
    }
}

void UIListContainerElement::SetVisible(bool visible) {
    UIContainer::SetVisible(visible);
    if(!IsVisible() && m_selected){
        m_selected = false;
        if(m_owner != nullptr){
            m_owner->SelectItem(-1);
        }
    }
}

void UIListContainerElement::SetEnabled(bool enable) {
    UIControl::SetEnabled(enable);
    if(!IsEnabled()){
        m_buttonState = 0;
    }
}

bool UIListContainerElement::IsSelected() const {
    return m_selected;
}

bool UIListContainerElement::Select(bool select, bool triggerEvent) {
    if( !IsEnabled() ) return false;
    if( select == m_selected ) return true;
    m_selected = select;
    if( select && m_owner != nullptr ) m_owner->SelectItem(m_index, triggerEvent);
    Invalidate();

    return true;
}

bool UIListContainerElement::IsExpandable() const {
    return m_expandable;
}

void UIListContainerElement::SetExpandable(bool expandable) {
    m_expandable = expandable;
}

bool UIListContainerElement::IsExpanded() const {
    return m_expand;
}

bool UIListContainerElement::Expand(bool expand) {
    assert(m_owner);
    if( m_owner == nullptr ) return false;
    if( expand == m_expand ) return true;
    m_expand = expand;
    if( m_expandable ) {
        if( !m_owner->ExpandItem(m_index, expand) ) return false;
        if( m_manager != nullptr ) {
            if( expand ) m_manager->SendNotify(this, DUI_MSGTYPE_ITEMEXPAND, (WPARAM)(long)false);
            else m_manager->SendNotify(this, DUI_MSGTYPE_ITEMCOLLAPSE, (WPARAM)(long)false);
        }
    }

    return true;
}

void UIListContainerElement::Invalidate() {
    if( !IsVisible() ) return;

    if( GetParent() ) {
        auto* pParentContainer = static_cast<UIContainer*>(GetParent()->GetInterface(UIString{DUI_CTR_CONTAINER}));
        if( pParentContainer ) {
            RECT rc = pParentContainer->GetPos();
            RECT rcInset = pParentContainer->GetInset();
            rc.left += rcInset.left;
            rc.top += rcInset.top;
            rc.right -= rcInset.right;
            rc.bottom -= rcInset.bottom;
            UIScrollBar* pVerticalScrollBar = pParentContainer->GetVerticalScrollBar();
            if( pVerticalScrollBar && pVerticalScrollBar->IsVisible() ) rc.right -= pVerticalScrollBar->GetFixedWidth();
            UIScrollBar* pHorizontalScrollBar = pParentContainer->GetHorizontalScrollBar();
            if( pHorizontalScrollBar && pHorizontalScrollBar->IsVisible() ) rc.bottom -= pHorizontalScrollBar->GetFixedHeight();

            RECT invalidateRc = m_rcItem;
            if( !::UIIntersectRect(&invalidateRc, &m_rcItem, &rc) )
            {
                return;
            }

            UIControl* pParent = GetParent();
            RECT rcTemp;
            RECT rcParent;
            while( (pParent = pParent->GetParent()) && pParent!=nullptr )
            {
                rcTemp = invalidateRc;
                rcParent = pParent->GetPos();
                if( !::UIIntersectRect(&invalidateRc, &rcTemp, &rcParent) )
                {
                    return;
                }
            }

            if( m_manager != nullptr ) m_manager->Invalidate(invalidateRc);
        }
        else {
            UIContainer::Invalidate();
        }
    }
    else {
        UIContainer::Invalidate();
    }
}

bool UIListContainerElement::Activate() {
    if( !UIContainer::Activate() ) return false;
    if( m_manager != nullptr ) m_manager->SendNotify(this, DUI_MSGTYPE_ITEMACTIVATE);
    return true;
}

void UIListContainerElement::DoEvent(TEventUI &event) {
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_owner != nullptr ) m_owner->DoEvent(event);
        else UIContainer::DoEvent(event);
        return;
    }

    if( event.Type == UIEVENT_DBLCLICK )
    {
        if( IsEnabled() ) {
            Activate();
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_KEYDOWN )
    {
        if (IsKeyboardEnabled() && IsEnabled()) {
            if( event.chKey == VK_RETURN ) {
                Activate();
                Invalidate();
                return;
            }
        }
    }
    if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_RBUTTONDOWN )
    {
        if( IsEnabled() ) {
            m_manager->SendNotify(this, DUI_MSGTYPE_ITEMCLICK);
            Select();
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_BUTTONUP )
    {
        return;
    }
    if( event.Type == UIEVENT_MOUSEMOVE )
    {
        return;
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

    // An important twist: The list-item will send the event not to its immediate
    // parent but to the "attached" list. A list may actually embed several components
    // in its path to the item, but key-presses etc. needs to go to the actual list.
    if( m_owner != nullptr ) m_owner->DoEvent(event); else UIControl::DoEvent(event);
}

void UIListContainerElement::SetAttribute(const char *pstrName, const char *pstrValue) {
    if( strcasecmp(pstrName, "selected") == 0 ) Select();
    else if( strcasecmp(pstrName, "expandable") == 0 ) SetExpandable(strcasecmp(pstrValue, "true") == 0);
    else UIContainer::SetAttribute(pstrName, pstrValue);
}

bool UIListContainerElement::DoPaint(HANDLE_DC hDC, const RECT &rcPaint, UIControl *stopControl) {
    DrawItemBk(hDC, m_rcItem);
    return UIContainer::DoPaint(hDC, rcPaint, stopControl);
}

void UIListContainerElement::DrawItemText(HANDLE_DC hDC, const RECT &rcItem) {

}

void UIListContainerElement::DrawItemBk(HANDLE_DC hDC, const RECT &rcItem) {
    assert(m_owner);
    if( m_owner == nullptr ) return;
    TListInfoUI* pInfo = m_owner->GetListInfo();
    if( pInfo == nullptr ) return;
    uint32_t iBackColor = 0;
    if( !pInfo->bAlternateBk || m_drawIndex % 2 == 0 ) iBackColor = pInfo->dwBkColor;

    if( (m_buttonState & UISTATE_HOT) != 0 ) {
        iBackColor = pInfo->dwHotBkColor;
    }
    if( IsSelected() ) {
        iBackColor = pInfo->dwSelectedBkColor;
    }
    if( !IsEnabled() ) {
        iBackColor = pInfo->dwDisabledBkColor;
    }
    if ( iBackColor != 0 ) {
        UIRenderEngine::DrawColor(hDC, m_rcItem, GetAdjustColor(iBackColor));
    }

    if( !IsEnabled() ) {
        if( DrawImage(hDC, pInfo->diDisabled) ) return;
    }
    if( IsSelected() ) {
        if( DrawImage(hDC, pInfo->diSelected) ) return;
    }
    if( (m_buttonState & UISTATE_HOT) != 0 ) {
        if( DrawImage(hDC, pInfo->diHot) ) return;
    }
    if( !DrawImage(hDC, m_diBk) ) {
        if( !pInfo->bAlternateBk || m_drawIndex % 2 == 0 ) {
            if( DrawImage(hDC, pInfo->diBk) ) return;
        }
    }
}

SIZE UIListContainerElement::EstimateSize(SIZE szAvailable) {
    TListInfoUI* pInfo = nullptr;
    if( m_owner ) pInfo = m_owner->GetListInfo();

    SIZE cXY = m_cxyFixed;

    if( cXY.cy == 0 ) {
        cXY.cy = (long)(pInfo->uFixedHeight);
    }

    return cXY;
}


//////////////////////// UIListHBoxElement

UIListHBoxElement::UIListHBoxElement() = default;

UIString UIListHBoxElement::GetClass() const {
    return UIString{DUI_CTR_LISTHBOXELEMENT};
}

LPVOID UIListHBoxElement::GetInterface(const UIString &name) {
    if(name == DUI_CTR_LISTHBOXELEMENT){
        return static_cast<UIListHBoxElement*>(this);
    }
    return UIListContainerElement::GetInterface(name);
}

void UIListHBoxElement::SetPos(RECT rc, bool needInvalidate) {
    if( m_owner == nullptr ) return UIListContainerElement::SetPos(rc, needInvalidate);

    UIControl::SetPos(rc, needInvalidate);
    rc = m_rcItem;

    TListInfoUI* pInfo = m_owner->GetListInfo();
    if (pInfo == nullptr) return;
    if (pInfo->nColumns > 0) {
        int iColumnIndex = 0;
        for( int it2 = 0; it2 < m_items.GetSize(); it2++ ) {
            auto* pControl = static_cast<UIControl*>(m_items[it2]);
            if( !pControl->IsVisible() ) continue;
            if( pControl->IsFloat() ) {
                SetFloatPos(it2);
                continue;
            }
            if( iColumnIndex >= pInfo->nColumns ) continue;

            RECT rcPadding = pControl->GetPadding();
            RECT rcItem = { pInfo->rcColumn[iColumnIndex].left + rcPadding.left, m_rcItem.top + rcPadding.top,
                            pInfo->rcColumn[iColumnIndex].right - rcPadding.right, m_rcItem.bottom - rcPadding.bottom };
            if (pInfo->iVLineSize > 0 && iColumnIndex < pInfo->nColumns - 1) {
                rcItem.right -= pInfo->iVLineSize;
            }
            pControl->SetPos(rcItem, false);
            iColumnIndex += 1;
        }
    }
    else {
        for( int it2 = 0; it2 < m_items.GetSize(); it2++ ) {
            auto* pControl = static_cast<UIControl*>(m_items[it2]);
            if( !pControl->IsVisible() ) continue;
            if( pControl->IsFloat() ) {
                SetFloatPos(it2);
                continue;
            }

            RECT rcPadding = pControl->GetPadding();
            RECT rcItem = { m_rcItem.left + rcPadding.left, m_rcItem.top + rcPadding.top,
                            m_rcItem.right - rcPadding.right, m_rcItem.bottom - rcPadding.bottom };
            pControl->SetPos(rcItem, false);
        }
    }
}

bool UIListHBoxElement::DoPaint(HANDLE_DC hDC, const RECT &rcPaint, UIControl *stopControl) {
    assert(m_owner);
    if( m_owner == nullptr ) return true;
    TListInfoUI* pInfo = m_owner->GetListInfo();
    if( pInfo == nullptr ) return true;

    DrawItemBk(hDC, m_rcItem);
    for( int i = 0; i < pInfo->nColumns; i++ ) {
        RECT rcItem = { pInfo->rcColumn[i].left, m_rcItem.top, pInfo->rcColumn[i].right, m_rcItem.bottom };
        if (pInfo->iVLineSize > 0 && i < pInfo->nColumns - 1) {
            RECT rcLine = { rcItem.right - pInfo->iVLineSize / 2, rcItem.top, rcItem.right - pInfo->iVLineSize / 2, rcItem.bottom};
            UIRenderEngine::DrawLine(hDC, rcLine, pInfo->iVLineSize, GetAdjustColor(pInfo->dwVLineColor),0);
        }
    }
    return UIContainer::DoPaint(hDC, rcPaint, stopControl);
}
