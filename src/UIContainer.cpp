#include <UIContainer.h>
#include <UIPaintManager.h>
#include <UIScrollBar.h>
#include <cassert>
#include <UIRect.h>
#include <cstring>
#include <UIRenderClip.h>
#include <iostream>

using namespace std;

UIContainer::UIContainer()
    :m_rcInset{0,0,0,0},
     m_iChildPadding{0},
     m_iChildAlign{DT_LEFT},
     m_iChildVAlign{DT_TOP},
     m_bAutoDestroy{true},
     m_bDelayedDestroy{true},
     m_bMouseChildEnabled{true},
     m_pVerticalScrollBar{nullptr},
     m_pHorizontalScrollBar{nullptr},
     m_bScrollProcess{false}
{

}

UIContainer::~UIContainer() {
    m_bDelayedDestroy = false;
    RemoveAll();
    if(m_pVerticalScrollBar){
        m_pVerticalScrollBar->Delete();
    }
    if(m_pHorizontalScrollBar){
        m_pHorizontalScrollBar->Delete();
    }
}

UIString UIContainer::GetClass() const {
    return UIString{DUI_CTR_CONTAINER};
}

LPVOID UIContainer::GetInterface(const UIString &name) {
    if(name == DUI_CTR_ICONTAINER){
        return static_cast<IContainerUI*>(this);
    }
    if(name == DUI_CTR_CONTAINER){
        return static_cast<UIContainer*>(this);
    }
    return UIControl::GetInterface(name);
}

UIControl *UIContainer::GetItemAt(int iIndex) const {
    if(iIndex < 0 || iIndex >=m_items.GetSize()){
        return nullptr;
    }
    return static_cast<UIControl*>(m_items[iIndex]);
}

int UIContainer::GetItemIndex(UIControl *pControl) const {
    for(int it = 0; it<m_items.GetSize(); ++it){
        if(static_cast<UIControl*>(m_items[it])==pControl){
            return it;
        }
    }
    return -1;
}

bool UIContainer::SetItemIndex(UIControl *pControl, int iNewIndex) {
    for(int it=0; it<m_items.GetSize(); ++it){
        if(static_cast<UIControl*>(m_items[it]) == pControl){
            NeedUpdate();
            m_items.Remove(it);
            return m_items.InsertAt(iNewIndex, pControl);
        }
    }
    return false;
}

bool UIContainer::SetMultiItemIndex(UIControl *pStartControl, int iCount, int iNewStartIndex) {
    if (pStartControl == NULL || iCount < 0 || iNewStartIndex < 0) return false;
    int iStartIndex = GetItemIndex(pStartControl);
    if (iStartIndex == iNewStartIndex) return true;
    if (iStartIndex + iCount > GetCount()) return false;
    if (iNewStartIndex + iCount > GetCount()) return false;

    UIPtrArray pControls(iCount);
    pControls.Resize(iCount);
    memmove(pControls.GetData(), m_items.GetData() + iStartIndex, iCount * sizeof(LPVOID));
    m_items.Remove(iStartIndex, iCount);

    for( int it3 = 0; it3 < pControls.GetSize(); it3++ ) {
        if (!pControls.InsertAt(iNewStartIndex + it3, pControls[it3])) return false;
    }

    NeedUpdate();
    return true;
}

int UIContainer::GetCount() const {
    return m_items.GetSize();
}

bool UIContainer::Add(UIControl *pControl) {
    if( pControl == nullptr) return false;

    if( m_manager != nullptr ) m_manager->InitControls(pControl, this);
    if( IsVisible() ) NeedUpdate();
    else pControl->SetInternVisible(false);
    return m_items.Add(pControl);
}

bool UIContainer::AddAt(UIControl *pControl, int iIndex) {
    if( pControl == nullptr) return false;

    if( m_manager != nullptr ) m_manager->InitControls(pControl, this);
    if( IsVisible() ) NeedUpdate();
    else pControl->SetInternVisible(false);
    return m_items.InsertAt(iIndex, pControl);
}

bool UIContainer::Remove(UIControl *pControl, bool bDoNotDestroy) {
    if( pControl == nullptr) return false;

    for( int it = 0; it < m_items.GetSize(); it++ ) {
        if( static_cast<UIControl*>(m_items[it]) == pControl ) {
            NeedUpdate();
            if( !bDoNotDestroy && m_bAutoDestroy ) {
                if( m_bDelayedDestroy && m_manager ) m_manager->AddDelayedCleanup(pControl);
                else pControl->Delete();
            }
            return m_items.Remove(it);
        }
    }
    return false;
}

bool UIContainer::RemoveAt(int iIndex, bool bDoNotDestroy) {
    UIControl* pControl = GetItemAt(iIndex);
    if (pControl != NULL) {
        return UIContainer::Remove(pControl, bDoNotDestroy);
    }

    return false;
}

void UIContainer::RemoveAll() {
    for( int it = 0; m_bAutoDestroy && it < m_items.GetSize(); it++ ) {
        if( m_bDelayedDestroy && m_manager ) m_manager->AddDelayedCleanup(static_cast<UIControl*>(m_items[it]));
        else static_cast<UIControl*>(m_items[it])->Delete();
    }
    m_items.Empty();
    NeedUpdate();
}

void UIContainer::DoEvent(TEventUI &event) {
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_parent != NULL ) m_parent->DoEvent(event);
        else UIControl::DoEvent(event);
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
            if( m_pVerticalScrollBar != NULL && m_pVerticalScrollBar->IsVisible() && m_pVerticalScrollBar->IsEnabled() )
            {
                switch( event.chKey ) {
                    case VK_DOWN:
                        LineDown();
                        return;
                    case VK_UP:
                        LineUp();
                        return;
                    case VK_NEXT:
                        PageDown();
                        return;
                    case VK_PRIOR:
                        PageUp();
                        return;
                    case VK_HOME:
                        HomeUp();
                        return;
                    case VK_END:
                        EndDown();
                        return;
                }
            }
            else if (m_pHorizontalScrollBar != NULL && m_pHorizontalScrollBar->IsVisible() && m_pHorizontalScrollBar->IsEnabled())
            {
                switch( event.chKey ) {
                    case VK_DOWN:
                        LineRight();
                        return;
                    case VK_UP:
                        LineLeft();
                        return;
                    case VK_NEXT:
                        PageRight();
                        return;
                    case VK_PRIOR:
                        PageLeft();
                        return;
                    case VK_HOME:
                        HomeLeft();
                        return;
                    case VK_END:
                        EndRight();
                        return;
                }
            }
        }
    }
    else if (event.Type == UIEVENT_SCROLLWHEEL)
    {
        if (m_pHorizontalScrollBar != NULL && m_pHorizontalScrollBar->IsVisible() && m_pHorizontalScrollBar->IsEnabled())
        {
            UIRect rcHorizontalScrollBar {m_pHorizontalScrollBar->GetPos()};
            if(rcHorizontalScrollBar.IsPtIn(event.ptMouse))
            {
                switch( LOWORD(event.wParam) ) {
                    case SB_LINEUP:
                        LineLeft();
                        return;
                    case SB_LINEDOWN:
                        LineRight();
                        return;
                }
            }
        }
        if (m_pVerticalScrollBar != NULL && m_pVerticalScrollBar->IsVisible() && m_pVerticalScrollBar->IsEnabled())
        {
            switch( LOWORD(event.wParam) ) {
                case SB_LINEUP:
                    LineUp();
                    return;
                case SB_LINEDOWN:
                    LineDown();
                    return;
            }
        }
        if (m_pHorizontalScrollBar != NULL && m_pHorizontalScrollBar->IsVisible() && m_pHorizontalScrollBar->IsEnabled())
        {
            switch( LOWORD(event.wParam) ) {
                case SB_LINEUP:
                    LineLeft();
                    return;
                case SB_LINEDOWN:
                    LineRight();
                    return;
            }
        }
    }
    UIControl::DoEvent(event);
}

void UIContainer::SetVisible(bool bVisible) {
    if( m_visible == bVisible ) return;
    UIControl::SetVisible(bVisible);
    for( int it = 0; it < m_items.GetSize(); it++ ) {
        static_cast<UIControl*>(m_items[it])->SetInternVisible(IsVisible());
    }
}

// 逻辑上，对于Container控件不公开此方法
// 调用此方法的结果是，内部子控件隐藏，控件本身依然显示，背景等效果存在
void UIContainer::SetInternVisible(bool bVisible) {
    UIControl::SetInternVisible(bVisible);
    if( m_items.IsEmpty() ) return;
    for( int it = 0; it < m_items.GetSize(); it++ ) {
        // 控制子控件显示状态
        // InternVisible状态应由子控件自己控制
        static_cast<UIControl*>(m_items[it])->SetInternVisible(IsVisible());
    }
}

void UIContainer::SetMouseEnabled(bool bEnable) {
    if( m_pVerticalScrollBar != NULL ) m_pVerticalScrollBar->SetMouseEnabled(bEnable);
    if( m_pHorizontalScrollBar != NULL ) m_pHorizontalScrollBar->SetMouseEnabled(bEnable);
    UIControl::SetMouseEnabled(bEnable);
}

RECT UIContainer::GetInset() const {
    return m_rcInset;
}

void UIContainer::SetInset(RECT rcInset) {
    m_rcInset = rcInset;
    NeedUpdate();
}

int UIContainer::GetChildPadding() const {
    return m_iChildPadding;
}

void UIContainer::SetChildPadding(int iPadding) {
    m_iChildPadding = iPadding;
    if(m_iChildPadding < 0){
        m_iChildPadding = 0;
    }
    NeedUpdate();
}

uint32_t UIContainer::GetChildAlign() const {
    return m_iChildAlign;
}

void UIContainer::SetChildAlign(uint32_t iAlign) {
    m_iChildAlign = iAlign;
    NeedUpdate();
}

uint32_t UIContainer::GetChildVAlign() const {
    return m_iChildVAlign;
}

void UIContainer::SetChildVAlign(uint32_t iVAlign) {
    m_iChildVAlign = iVAlign;
    NeedUpdate();
}

bool UIContainer::IsAutoDestroy() const {
    return m_bAutoDestroy;
}

void UIContainer::SetAutoDestroy(bool bAuto) {
    m_bAutoDestroy = bAuto;
}

bool UIContainer::IsDelayedDestroy() const {
    return m_bDelayedDestroy;
}

void UIContainer::SetDelayedDestroy(bool bDelayed) {
    m_bDelayedDestroy = bDelayed;
}

bool UIContainer::IsMouseChildEnabled() const {
    return m_bMouseChildEnabled;
}

void UIContainer::SetMouseChildEnabled(bool bEnable) {
    m_bMouseChildEnabled = bEnable;
}

int UIContainer::FindSelectable(int iIndex, bool bForward) const {
    // NOTE: This is actually a helper-function for the list/combo/ect controls
    //       that allow them to find the next enabled/available selectable item
    if( GetCount() == 0 ) return -1;
    iIndex = CLAMP(iIndex, 0, GetCount() - 1);
    if( bForward ) {
        for( int i = iIndex; i < GetCount(); i++ ) {
            if( GetItemAt(i)->GetInterface(UIString{DUI_CTR_ILISTITEM}) != NULL
                && GetItemAt(i)->IsVisible()
                && GetItemAt(i)->IsEnabled() ) return i;
        }
        return -1;
    }
    else {
        for( int i = iIndex; i >= 0; --i ) {
            if( GetItemAt(i)->GetInterface(UIString{DUI_CTR_ILISTITEM}) != NULL
                && GetItemAt(i)->IsVisible()
                && GetItemAt(i)->IsEnabled() ) return i;
        }
        return FindSelectable(0, true);
    }
}

RECT UIContainer::GetClientPos() const {
    RECT rc = m_rcItem;
    rc.left += m_rcInset.left;
    rc.top += m_rcInset.top;
    rc.right -= m_rcInset.right;
    rc.bottom -= m_rcInset.bottom;

    if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
        rc.right -= m_pVerticalScrollBar->GetFixedWidth();
    }
    if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
        rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
    }
    return rc;
}

void UIContainer::SetPos(RECT rc, bool bNeedInvalidate) {
    UIControl::SetPos(rc, bNeedInvalidate);
    //CControlUI::SetPos(rc, bNeedInvalidate);
    if( m_items.IsEmpty() ) return;

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

    for( int it = 0; it < m_items.GetSize(); it++ ) {
        UIControl* pControl = static_cast<UIControl*>(m_items[it]);
        if( !pControl->IsVisible() ) continue;
        if( pControl->IsFloat() ) {
            SetFloatPos(it);
        }
        else {
            SIZE sz = { rc.right - rc.left, rc.bottom - rc.top };
            if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
            if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();
            if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
            if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
            RECT rcCtrl = { rc.left, rc.top, rc.left + sz.cx, rc.top + sz.cy };
            pControl->SetPos(rcCtrl, false);
        }
    }
}

void UIContainer::Move(SIZE szOffset, bool bNeedInvalidate) {
    UIControl::Move(szOffset, bNeedInvalidate);
    if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) m_pVerticalScrollBar->Move(szOffset, false);
    if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) m_pHorizontalScrollBar->Move(szOffset, false);
    for( int it = 0; it < m_items.GetSize(); it++ ) {
        UIControl* pControl = static_cast<UIControl*>(m_items[it]);
        if( pControl != NULL && pControl->IsVisible() ) pControl->Move(szOffset, false);
    }
}

bool UIContainer::DoPaint(HANDLE_DC hDC, const RECT &rcPaint, UIControl *pStopControl) {
    RECT rcTemp = { 0 };
    if( !UIIntersectRect(&rcTemp, &rcPaint, &m_rcItem) ) return true;

    UIRenderClip clip;
    UIRenderClip::GenerateClip(hDC, rcTemp, clip);

    UIControl::DoPaint(hDC, rcPaint, pStopControl);

    if( m_items.GetSize() > 0 ) {
        RECT rc = m_rcItem;
        rc.left += m_rcInset.left;
        rc.top += m_rcInset.top;
        rc.right -= m_rcInset.right;
        rc.bottom -= m_rcInset.bottom;
        if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
        if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

        if( !::UIIntersectRect(&rcTemp, &rcPaint, &rc) ) {
            for( int it = 0; it < m_items.GetSize(); it++ ) {
                UIControl* pControl = static_cast<UIControl*>(m_items[it]);
                if( pControl == pStopControl ) return false;
                if( !pControl->IsVisible() ) continue;
                if( !::UIIntersectRect(&rcTemp, &rcPaint, &pControl->GetPos()) ) continue;
                if( pControl->IsFloat() ) {
                    if( !::UIIntersectRect(&rcTemp, &m_rcItem, &pControl->GetPos()) ) continue;
                    if( !pControl->Paint(hDC, rcPaint, pStopControl) ) return false;
                }
            }
        }
        else {
            UIRenderClip childClip;
            UIRenderClip::GenerateClip(hDC, rcTemp, childClip);

            for( int it = 0; it < m_items.GetSize(); it++ ) {
                UIControl* pControl = static_cast<UIControl*>(m_items[it]);
                if( pControl == pStopControl ) return false;
                if( !pControl->IsVisible() ) continue;
                if( !::UIIntersectRect(&rcTemp, &rcPaint, &pControl->GetPos()) ) continue;
                if( pControl->IsFloat() ) {
                    if( !::UIIntersectRect(&rcTemp, &m_rcItem, &pControl->GetPos()) ) continue;
                    UIRenderClip::UseOldClipBegin(hDC, childClip);

                    if( !pControl->Paint(hDC, rcPaint, pStopControl) ) {
                        UIRenderClip::UseOldClipEnd(hDC, childClip);
                        return false;
                    }
                    UIRenderClip::UseOldClipEnd(hDC, childClip);
                }
                else {
                    if( !::UIIntersectRect(&rcTemp, &rc, &pControl->GetPos()) ) continue;
                    if( !pControl->Paint(hDC, rcPaint, pStopControl) ) return false;
                }
            }
        }
    }

    if( m_pVerticalScrollBar != NULL ) {
        if( m_pVerticalScrollBar == pStopControl ) return false;
        if (m_pVerticalScrollBar->IsVisible()) {
            if( ::UIIntersectRect(&rcTemp, &rcPaint, &m_pVerticalScrollBar->GetPos()) ) {
                if( !m_pVerticalScrollBar->Paint(hDC, rcPaint, pStopControl) ) return false;
            }
        }
    }

    if( m_pHorizontalScrollBar != NULL ) {
        if( m_pHorizontalScrollBar == pStopControl ) return false;
        if (m_pHorizontalScrollBar->IsVisible()) {
            if( ::UIIntersectRect(&rcTemp, &rcPaint, &m_pHorizontalScrollBar->GetPos()) ) {
                if( !m_pHorizontalScrollBar->Paint(hDC, rcPaint, pStopControl) ) return false;
            }
        }
    }
    return true;
}

void UIContainer::SetAttribute(const char* pstrName, const char* pstrValue) {
    if( strcasecmp(pstrName, "inset") == 0 ) {
        RECT rcInset = { 0 };
        char *pstr = NULL;
        rcInset.left = strtol(pstrValue, &pstr, 10);  assert(pstr);
        rcInset.top = strtol(pstr + 1, &pstr, 10);    assert(pstr);
        rcInset.right = strtol(pstr + 1, &pstr, 10);  assert(pstr);
        rcInset.bottom = strtol(pstr + 1, &pstr, 10); assert(pstr);
        SetInset(rcInset);
    }
    else if( strcasecmp(pstrName, "mousechild") == 0 ) SetMouseChildEnabled(strcasecmp(pstrValue, "true") == 0);
    else if( strcasecmp(pstrName, "vscrollbar") == 0 ) {
        EnableScrollBar(strcasecmp(pstrValue, "true") == 0, GetHorizontalScrollBar() != NULL);
    }
    else if( strcasecmp(pstrName, "vscrollbarstyle") == 0 ) {
        EnableScrollBar(true, GetHorizontalScrollBar() != NULL);
        if( GetVerticalScrollBar() ) GetVerticalScrollBar()->SetAttributeList(pstrValue);
    }
    else if( strcasecmp(pstrName, "hscrollbar") == 0 ) {
        EnableScrollBar(GetVerticalScrollBar() != NULL, strcasecmp(pstrValue, "true") == 0);
    }
    else if( strcasecmp(pstrName, "hscrollbarstyle") == 0 ) {
        EnableScrollBar(GetVerticalScrollBar() != NULL, true);
        if( GetHorizontalScrollBar() ) GetHorizontalScrollBar()->SetAttributeList(pstrValue);
    }
    else if( strcasecmp(pstrName, "childpadding") == 0 ) SetChildPadding(atoi(pstrValue));
    else if( strcasecmp(pstrName, "childalign") == 0 ) {
        if( strcasecmp(pstrValue, "left") == 0 ) m_iChildAlign = DT_LEFT;
        else if( strcasecmp(pstrValue, "center") == 0 ) m_iChildAlign = DT_CENTER;
        else if( strcasecmp(pstrValue, "right") == 0 ) m_iChildAlign = DT_RIGHT;
    }
    else if( strcasecmp(pstrName, "childvalign") == 0 ) {
        if( strcasecmp(pstrValue, "top") == 0 ) m_iChildVAlign = DT_TOP;
        else if( strcasecmp(pstrValue, "vcenter") == 0 ) m_iChildVAlign = DT_VCENTER;
        else if( strcasecmp(pstrValue, "bottom") == 0 ) m_iChildVAlign = DT_BOTTOM;
    }
    else UIControl::SetAttribute(pstrName, pstrValue);

}

void UIContainer::SetManager(UIPaintManager *pManager, UIControl *pParent, bool bInit) {
    for( int it = 0; it < m_items.GetSize(); it++ ) {
        static_cast<UIControl*>(m_items[it])->SetManager(pManager, this, bInit);
    }

    if( m_pVerticalScrollBar != NULL ) m_pVerticalScrollBar->SetManager(pManager, this, bInit);
    if( m_pHorizontalScrollBar != NULL ) m_pHorizontalScrollBar->SetManager(pManager, this, bInit);
    UIControl::SetManager(pManager, pParent, bInit);
}

UIControl *UIContainer::FindControl(FindControlProc Proc, LPVOID pData, uint32_t uFlags) {
    UIRect  rcItem{m_rcItem};
    // Check if this guy is valid
    if( (uFlags & UIFIND_VISIBLE) != 0 && !IsVisible() ) return NULL;
    if( (uFlags & UIFIND_ENABLED) != 0 && !IsEnabled() ) return NULL;
    if( (uFlags & UIFIND_HITTEST) != 0 && !rcItem.IsPtIn(*(static_cast<LPPOINT>(pData))) ) return NULL;
    if( (uFlags & UIFIND_UPDATETEST) != 0 && Proc(this, pData) != NULL ) return NULL;

    UIControl* pResult = NULL;
    if( (uFlags & UIFIND_ME_FIRST) != 0 ) {
        if( (uFlags & UIFIND_HITTEST) == 0 || IsMouseEnabled() ) pResult = Proc(this, pData);
    }
    if( pResult == NULL && m_cover != NULL ) {
        if( (uFlags & UIFIND_HITTEST) == 0 || IsMouseChildEnabled() ) pResult = m_cover->FindControl(Proc, pData, uFlags);
    }
    if( pResult == NULL && m_pVerticalScrollBar != NULL ) {
        if( (uFlags & UIFIND_HITTEST) == 0 || IsMouseEnabled() ) pResult = m_pVerticalScrollBar->FindControl(Proc, pData, uFlags);
    }
    if( pResult == NULL && m_pHorizontalScrollBar != NULL ) {
        if( (uFlags & UIFIND_HITTEST) == 0 || IsMouseEnabled() ) pResult = m_pHorizontalScrollBar->FindControl(Proc, pData, uFlags);
    }
    if( pResult != NULL ) return pResult;

    if( (uFlags & UIFIND_HITTEST) == 0 || IsMouseChildEnabled() ) {
        RECT rc = m_rcItem;
        rc.left += m_rcInset.left;
        rc.top += m_rcInset.top;
        rc.right -= m_rcInset.right;
        rc.bottom -= m_rcInset.bottom;
        if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
        if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
        if( (uFlags & UIFIND_TOP_FIRST) != 0 ) {
            for( int it = m_items.GetSize() - 1; it >= 0; it-- ) {
                pResult = static_cast<UIControl*>(m_items[it])->FindControl(Proc, pData, uFlags);
                if( pResult != NULL ) {
                    UIRect   rcRect{rc};
                    if( (uFlags & UIFIND_HITTEST) != 0 && !pResult->IsFloat() && !rcRect.IsPtIn(*(static_cast<LPPOINT>(pData))) )
                        continue;
                    else
                        return pResult;
                }
            }
        }
        else {
            for( int it = 0; it < m_items.GetSize(); it++ ) {
                pResult = static_cast<UIControl*>(m_items[it])->FindControl(Proc, pData, uFlags);
                if( pResult != NULL ) {
                    UIRect  rcRect {rc};
                    if( (uFlags & UIFIND_HITTEST) != 0 && !pResult->IsFloat() && !rcRect.IsPtIn(*(static_cast<LPPOINT>(pData))) )
                        continue;
                    else
                        return pResult;
                }
            }
        }
    }

    pResult = NULL;
    if( pResult == NULL && (uFlags & UIFIND_ME_FIRST) == 0 ) {
        if( (uFlags & UIFIND_HITTEST) == 0 || IsMouseEnabled() ) pResult = Proc(this, pData);
    }
    return pResult;
}

bool UIContainer::SetSubControlText(const char* pstrSubControlName, const char* pstrText) {
    UIControl* pSubControl=NULL;
    pSubControl=this->FindSubControl(pstrSubControlName);
    if (pSubControl!=NULL)
    {
        pSubControl->SetText(UIString{pstrText});
        return true;
    }
    return false;
}

bool UIContainer::SetSubControlFixedHeight(const char* pstrSubControlName, int cy) {
    UIControl* pSubControl=NULL;
    pSubControl=this->FindSubControl(pstrSubControlName);
    if (pSubControl!=NULL)
    {
        pSubControl->SetFixedHeight(cy);
        return true;
    }
    return false;
}

bool UIContainer::SetSubControlFixedWidth(const char* pstrSubControlName, int cx) {
    UIControl* pSubControl=NULL;
    pSubControl=this->FindSubControl(pstrSubControlName);
    if (pSubControl!=NULL)
    {
        pSubControl->SetFixedWidth(cx);
        return true;
    }
    return false;
}

bool UIContainer::SetSubControlUserData(const char* pstrSubControlName, const char* pstrText) {
    UIControl* pSubControl=NULL;
    pSubControl=this->FindSubControl(pstrSubControlName);
    if (pSubControl!=NULL)
    {
        pSubControl->SetUserData(pstrText);
        return true;
    }
    return false;
}

UIString UIContainer::GetSubControlText(const char* pstrSubControlName) {
    UIControl* pSubControl=NULL;
    pSubControl=this->FindSubControl(pstrSubControlName);
    if (pSubControl==NULL)
    {
        return UIString{""};
    }
    return pSubControl->GetText();
}

int UIContainer::GetSubControlFixedHeight(const char* pstrSubControlName) {
    UIControl* pSubControl=NULL;
    pSubControl=this->FindSubControl(pstrSubControlName);
    if (pSubControl==NULL)
    {
        return -1;
    }
    return pSubControl->GetFixedHeight();
}

int UIContainer::GetSubControlFixedWidth(const char* pstrSubControlName) {
    UIControl* pSubControl=NULL;
    pSubControl=this->FindSubControl(pstrSubControlName);
    if (pSubControl==NULL)
    {
        return -1;
    }
    return pSubControl->GetFixedWidth();
}

const UIString UIContainer::GetSubControlUserData(const char* pstrSubControlName) {
    UIControl* pSubControl=NULL;
    pSubControl=this->FindSubControl(pstrSubControlName);
    if (pSubControl==NULL)
        return UIString{""};
    else
        return pSubControl->GetUserData();
}

UIControl *UIContainer::FindSubControl(const char* pstrSubControlName) {
    UIControl* pSubControl=NULL;
    pSubControl=static_cast<UIControl*>(GetManager()->FindSubControlByName(this,pstrSubControlName));
    return pSubControl;
}

SIZE UIContainer::GetScrollPos() const {
    SIZE sz = {0, 0};
    if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) sz.cy = m_pVerticalScrollBar->GetScrollPos();
    if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) sz.cx = m_pHorizontalScrollBar->GetScrollPos();
    return sz;
}

SIZE UIContainer::GetScrollRange() const {
    SIZE sz = {0, 0};
    if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) sz.cy = m_pVerticalScrollBar->GetScrollRange();
    if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) sz.cx = m_pHorizontalScrollBar->GetScrollRange();
    return sz;
}

void UIContainer::SetScrollPos(SIZE szPos) {
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
        UIControl* pControl = static_cast<UIControl*>(m_items[it2]);
        if( !pControl->IsVisible() ) continue;
        if( pControl->IsFloat() ) continue;
        pControl->Move(SIZE{-cx, -cy}, false);
    }

    Invalidate();
}

void UIContainer::LineUp() {
    int cyLine = SCROLLBAR_LINESIZE;
    if( m_manager ) {
        //TODO Default Font Height;
        //cyLine = m_manager->GetDefaultFontInfo()->tm.tmHeight + 8;
        if (m_pVerticalScrollBar && m_pVerticalScrollBar->GetScrollUnit() > 1)
            cyLine = m_pVerticalScrollBar->GetScrollUnit();
    }

    SIZE sz = GetScrollPos();
    sz.cy -= cyLine;
    SetScrollPos(sz);
}

void UIContainer::LineDown() {
    int cyLine = SCROLLBAR_LINESIZE;
    if( m_manager ) {
        //TODO Default Font Height;
        //cyLine = m_manager->GetDefaultFontInfo()->tm.tmHeight + 8;
        if (m_pVerticalScrollBar && m_pVerticalScrollBar->GetScrollUnit() > 1)
            cyLine = m_pVerticalScrollBar->GetScrollUnit();
    }

    SIZE sz = GetScrollPos();
    sz.cy += cyLine;
    SetScrollPos(sz);
}

void UIContainer::PageUp() {
    SIZE sz = GetScrollPos();
    int iOffset = m_rcItem.bottom - m_rcItem.top - m_rcInset.top - m_rcInset.bottom;
    if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) iOffset -= m_pHorizontalScrollBar->GetFixedHeight();
    sz.cy -= iOffset;
    SetScrollPos(sz);
}

void UIContainer::PageDown() {
    SIZE sz = GetScrollPos();
    int iOffset = m_rcItem.bottom - m_rcItem.top - m_rcInset.top - m_rcInset.bottom;
    if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) iOffset -= m_pHorizontalScrollBar->GetFixedHeight();
    sz.cy += iOffset;
    SetScrollPos(sz);
}

void UIContainer::HomeUp() {
    SIZE sz = GetScrollPos();
    sz.cy = 0;
    SetScrollPos(sz);
}

void UIContainer::EndDown() {
    SIZE sz = GetScrollPos();
    sz.cy = GetScrollRange().cy;
    SetScrollPos(sz);
}

void UIContainer::LineLeft() {
    int cxLine = SCROLLBAR_LINESIZE;
    if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->GetScrollUnit() > 1)
        cxLine = m_pHorizontalScrollBar->GetScrollUnit();

    SIZE sz = GetScrollPos();
    sz.cx -= cxLine;
    SetScrollPos(sz);
}

void UIContainer::LineRight() {
    int cxLine = SCROLLBAR_LINESIZE;
    if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->GetScrollUnit() > 1)
        cxLine = m_pHorizontalScrollBar->GetScrollUnit();

    SIZE sz = GetScrollPos();
    sz.cx += cxLine;
    SetScrollPos(sz);
}

void UIContainer::PageLeft() {
    SIZE sz = GetScrollPos();
    int iOffset = m_rcItem.right - m_rcItem.left - m_rcInset.left - m_rcInset.right;
    if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) iOffset -= m_pVerticalScrollBar->GetFixedWidth();
    sz.cx -= iOffset;
    SetScrollPos(sz);
}

void UIContainer::PageRight() {
    SIZE sz = GetScrollPos();
    int iOffset = m_rcItem.right - m_rcItem.left - m_rcInset.left - m_rcInset.right;
    if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) iOffset -= m_pVerticalScrollBar->GetFixedWidth();
    sz.cx += iOffset;
    SetScrollPos(sz);
}

void UIContainer::HomeLeft() {
    SIZE sz = GetScrollPos();
    sz.cx = 0;
    SetScrollPos(sz);
}

void UIContainer::EndRight() {
    SIZE sz = GetScrollPos();
    sz.cx = GetScrollRange().cx;
    SetScrollPos(sz);
}

void UIContainer::EnableScrollBar(bool bEnableVertical, bool bEnableHorizontal) {
    if( bEnableVertical && !m_pVerticalScrollBar ) {
        m_pVerticalScrollBar = new UIScrollBar;
        m_pVerticalScrollBar->SetScrollRange(0);
        m_pVerticalScrollBar->SetOwner(this);
        m_pVerticalScrollBar->SetManager(m_manager, NULL, false);
        if ( m_manager ) {
            const char* pDefaultAttributes = m_manager->GetDefaultAttributeList("VScrollBar");
            //const char* pDefaultAttributes = m_manager->GetDefaultAttributeList("VScrollBar");
            if( pDefaultAttributes ) {
                m_pVerticalScrollBar->SetAttributeList(pDefaultAttributes);
            }
        }
    }
    else if( !bEnableVertical && m_pVerticalScrollBar ) {
        m_pVerticalScrollBar->Delete();
        m_pVerticalScrollBar = NULL;
    }

    if( bEnableHorizontal && !m_pHorizontalScrollBar ) {
        m_pHorizontalScrollBar = new UIScrollBar;
        m_pHorizontalScrollBar->SetScrollRange(0);
        m_pHorizontalScrollBar->SetHorizontal(true);
        m_pHorizontalScrollBar->SetOwner(this);
        m_pHorizontalScrollBar->SetManager(m_manager, nullptr, false);
        if ( m_manager ) {
            const char* pDefaultAttributes = m_manager->GetDefaultAttributeList("HScrollBar");
            if( pDefaultAttributes ) {
                m_pHorizontalScrollBar->SetAttributeList(pDefaultAttributes);
            }
        }
    }
    else if( !bEnableHorizontal && m_pHorizontalScrollBar ) {
        m_pHorizontalScrollBar->Delete();
        m_pHorizontalScrollBar = nullptr;
    }

    NeedUpdate();
}

UIScrollBar *UIContainer::GetVerticalScrollBar() const {
    return m_pVerticalScrollBar;
}

UIScrollBar *UIContainer::GetHorizontalScrollBar() const {
    return m_pHorizontalScrollBar;
}

void UIContainer::SetFloatPos(int iIndex) {
// 因为CControlUI::SetPos对float的操作影响，这里不能对float组件添加滚动条的影响
    if( iIndex < 0 || iIndex >= m_items.GetSize() ) return;

    UIControl* pControl = static_cast<UIControl*>(m_items[iIndex]);

    if( !pControl->IsVisible() ) return;
    if( !pControl->IsFloat() ) return;

    SIZE szXY = pControl->GetFixedXY();
    SIZE sz = {pControl->GetFixedWidth(), pControl->GetFixedHeight()};
    TPercentInfo rcPercent = pControl->GetFloatPercent();
    long width = m_rcItem.right - m_rcItem.left;
    long height = m_rcItem.bottom - m_rcItem.top;
    RECT rcCtrl = { 0 };
    rcCtrl.left = (long)(width*rcPercent.left) + szXY.cx;
    rcCtrl.top = (long)(height*rcPercent.top) + szXY.cy;
    rcCtrl.right = (long)(width*rcPercent.right) + szXY.cx + sz.cx;
    rcCtrl.bottom = (long)(height*rcPercent.bottom) + szXY.cy + sz.cy;
    pControl->SetPos(rcCtrl, false);
}

void UIContainer::ProcessScrollBar(RECT rc, int cxRequired, int cyRequired) {
    if (m_pHorizontalScrollBar != NULL) {
        if( m_pVerticalScrollBar == NULL ) {
            if( cxRequired > rc.right - rc.left && !m_pHorizontalScrollBar->IsVisible() ) {
                m_pHorizontalScrollBar->SetVisible(true);
                m_pHorizontalScrollBar->SetScrollRange(cxRequired - (rc.right - rc.left));
                m_pHorizontalScrollBar->SetScrollPos(0);
                m_bScrollProcess = true;
                if( !IsFloat() ) SetPos(GetPos());
                else SetPos(GetRelativePos());
                m_bScrollProcess = false;
                return;
            }
            // No scrollbar required
            if( !m_pHorizontalScrollBar->IsVisible() ) return;

            // Scroll not needed anymore?
            int cxScroll = cxRequired - (rc.right - rc.left);
            if( cxScroll <= 0 && !m_bScrollProcess) {
                m_pHorizontalScrollBar->SetVisible(false);
                m_pHorizontalScrollBar->SetScrollPos(0);
                m_pHorizontalScrollBar->SetScrollRange(0);
                if( !IsFloat() ) SetPos(GetPos());
                else SetPos(GetRelativePos());
            }
            else
            {
                RECT rcScrollBarPos = { rc.left, rc.bottom, rc.right, rc.bottom + m_pHorizontalScrollBar->GetFixedHeight()};
                m_pHorizontalScrollBar->SetPos(rcScrollBarPos, false);

                if( m_pHorizontalScrollBar->GetScrollRange() != cxScroll ) {
                    int iScrollPos = m_pHorizontalScrollBar->GetScrollPos();
                    m_pHorizontalScrollBar->SetScrollRange(::abs(cxScroll));
                    if( m_pHorizontalScrollBar->GetScrollRange() == 0 ) {
                        m_pHorizontalScrollBar->SetVisible(false);
                        m_pHorizontalScrollBar->SetScrollPos(0);
                    }
                    if( iScrollPos > m_pHorizontalScrollBar->GetScrollPos() ) {
                        if( !IsFloat() ) SetPos(GetPos(), false);
                        else SetPos(GetRelativePos(), false);
                    }
                }
            }
            return;
        }
        else {
            bool bNeedSetPos = false;
            if (cxRequired > rc.right - rc.left) {
                if (!m_pHorizontalScrollBar->IsVisible()) {
                    m_pHorizontalScrollBar->SetVisible(true);
                    m_pHorizontalScrollBar->SetScrollPos(0);
                    rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
                }
                RECT rcScrollBarPos = { rc.left, rc.bottom, rc.right, rc.bottom + m_pHorizontalScrollBar->GetFixedHeight()};
                m_pHorizontalScrollBar->SetPos(rcScrollBarPos, false);
                if (m_pHorizontalScrollBar->GetScrollRange() != cxRequired - (rc.right - rc.left)) {
                    m_pHorizontalScrollBar->SetScrollRange(cxRequired - (rc.right - rc.left));
                    bNeedSetPos = true;
                }
            }
            else {
                if (m_pHorizontalScrollBar->IsVisible()) {
                    m_pHorizontalScrollBar->SetVisible(false);
                    rc.bottom += m_pHorizontalScrollBar->GetFixedHeight();
                }
            }

            if( cyRequired > rc.bottom - rc.top && !m_pVerticalScrollBar->IsVisible() ) {
                m_pVerticalScrollBar->SetVisible(true);
                m_pVerticalScrollBar->SetScrollRange(cyRequired - (rc.bottom - rc.top));
                m_pVerticalScrollBar->SetScrollPos(0);
                rc.right -= m_pVerticalScrollBar->GetFixedWidth();
                if (m_pHorizontalScrollBar->IsVisible()) {
                    RECT rcScrollBarPos = { rc.left, rc.bottom, rc.right, rc.bottom + m_pHorizontalScrollBar->GetFixedHeight()};
                    m_pHorizontalScrollBar->SetPos(rcScrollBarPos, false);
                    m_pHorizontalScrollBar->SetScrollRange(cxRequired - (rc.right - rc.left));
                }
                m_bScrollProcess = true;
                if( !IsFloat() ) SetPos(GetPos());
                else SetPos(GetRelativePos());
                m_bScrollProcess = false;
                return;
            }
            // No scrollbar required
            if( !m_pVerticalScrollBar->IsVisible() ) {
                if (bNeedSetPos) {
                    if( !IsFloat() ) SetPos(GetPos());
                    else SetPos(GetRelativePos());
                }
                return;
            }

            // Scroll not needed anymore?
            int cyScroll = cyRequired - (rc.bottom - rc.top);
            if( cyScroll <= 0 && !m_bScrollProcess) {
                m_pVerticalScrollBar->SetVisible(false);
                m_pVerticalScrollBar->SetScrollPos(0);
                m_pVerticalScrollBar->SetScrollRange(0);
                rc.right += m_pVerticalScrollBar->GetFixedWidth();
                if (m_pHorizontalScrollBar->IsVisible()) {
                    RECT rcScrollBarPos = { rc.left, rc.bottom, rc.right, rc.bottom + m_pHorizontalScrollBar->GetFixedHeight()};
                    m_pHorizontalScrollBar->SetPos(rcScrollBarPos, false);
                    m_pHorizontalScrollBar->SetScrollRange(cxRequired - (rc.right - rc.left));
                }
                if( !IsFloat() ) SetPos(GetPos());
                else SetPos(GetRelativePos());
            }
            else
            {
                RECT rcScrollBarPos = { rc.right, rc.top, rc.right + m_pVerticalScrollBar->GetFixedWidth(), rc.bottom };
                m_pVerticalScrollBar->SetPos(rcScrollBarPos, false);

                if( m_pVerticalScrollBar->GetScrollRange() != cyScroll ) {
                    int iScrollPos = m_pVerticalScrollBar->GetScrollPos();
                    m_pVerticalScrollBar->SetScrollRange(::abs(cyScroll));
                    if( m_pVerticalScrollBar->GetScrollRange() == 0 ) {
                        m_pVerticalScrollBar->SetVisible(false);
                        m_pVerticalScrollBar->SetScrollPos(0);
                    }
                    if( iScrollPos > m_pVerticalScrollBar->GetScrollPos() || bNeedSetPos) {
                        if( !IsFloat() ) SetPos(GetPos(), false);
                        else SetPos(GetRelativePos(), false);
                    }
                }
            }
        }
    }
    else {
        if( m_pVerticalScrollBar == NULL ) return;

        if( cyRequired > rc.bottom - rc.top && !m_pVerticalScrollBar->IsVisible() ) {
            m_pVerticalScrollBar->SetVisible(true);
            m_pVerticalScrollBar->SetScrollRange(cyRequired - (rc.bottom - rc.top));
            m_pVerticalScrollBar->SetScrollPos(0);
            m_bScrollProcess = true;
            if( !IsFloat() ) SetPos(GetPos());
            else SetPos(GetRelativePos());
            m_bScrollProcess = false;
            return;
        }
        // No scrollbar required
        if( !m_pVerticalScrollBar->IsVisible() ) return;

        // Scroll not needed anymore?
        int cyScroll = cyRequired - (rc.bottom - rc.top);
        if( cyScroll <= 0 && !m_bScrollProcess) {
            m_pVerticalScrollBar->SetVisible(false);
            m_pVerticalScrollBar->SetScrollPos(0);
            m_pVerticalScrollBar->SetScrollRange(0);
            if( !IsFloat() ) SetPos(GetPos());
            else SetPos(GetRelativePos());
        }
        else
        {
            RECT rcScrollBarPos = { rc.right, rc.top, rc.right + m_pVerticalScrollBar->GetFixedWidth(), rc.bottom };
            m_pVerticalScrollBar->SetPos(rcScrollBarPos, false);

            if( m_pVerticalScrollBar->GetScrollRange() != cyScroll ) {
                int iScrollPos = m_pVerticalScrollBar->GetScrollPos();
                m_pVerticalScrollBar->SetScrollRange(::abs(cyScroll));
                if( m_pVerticalScrollBar->GetScrollRange() == 0 ) {
                    m_pVerticalScrollBar->SetVisible(false);
                    m_pVerticalScrollBar->SetScrollPos(0);
                }
                if( iScrollPos > m_pVerticalScrollBar->GetScrollPos() ) {
                    if( !IsFloat() ) SetPos(GetPos(), false);
                    else SetPos(GetRelativePos(), false);
                }
            }
        }
    }
}
