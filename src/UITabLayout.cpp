#include <UITabLayout.h>
#include <UIPaintManager.h>

UITabLayout::UITabLayout()
    :m_curSel{-1}
{

}

UIString UITabLayout::GetClass() const {
    return UIString{DUI_CTR_TABLAYOUT};
}

LPVOID UITabLayout::GetInterface(const UIString &name) {
    if(name == DUI_CTR_TABLAYOUT){
        return static_cast<UITabLayout*>(this);
    }
    return UIContainer::GetInterface(name);
}

bool UITabLayout::Add(UIControl *pControl) {
    bool ret = UIContainer::Add(pControl);
    if( !ret ) return ret;

    if(m_curSel == -1 && pControl->IsVisible())
    {
        m_curSel = GetItemIndex(pControl);
    }
    else
    {
        pControl->SetVisible(false);
    }

    return ret;
}

bool UITabLayout::AddAt(UIControl *pControl, int iIndex) {
    bool ret = UIContainer::AddAt(pControl, iIndex);
    if( !ret ) return ret;

    if(m_curSel == -1 && pControl->IsVisible())
    {
        m_curSel = GetItemIndex(pControl);
    }
    else if( m_curSel != -1 && iIndex <= m_curSel )
    {
        m_curSel += 1;
    }
    else
    {
        pControl->SetVisible(false);
    }

    return ret;
}

bool UITabLayout::Remove(UIControl *pControl, bool bDoNotDestroy) {
    if( pControl == nullptr) return false;

    int index = GetItemIndex(pControl);
    bool ret = UIContainer::Remove(pControl, bDoNotDestroy);
    if( !ret ) return false;

    if( m_curSel == index)
    {
        if( GetCount() > 0 )
        {
            m_curSel=0;
            GetItemAt(m_curSel)->SetVisible(true);
        }
        else
            m_curSel=-1;
        NeedParentUpdate();
    }
    else if( m_curSel > index )
    {
        m_curSel -= 1;
    }

    return ret;
}

void UITabLayout::RemoveAll() {
    m_curSel = -1;
    UIContainer::RemoveAll();
    NeedParentUpdate();
}

int UITabLayout::GetCurSel() const {
    return m_curSel;
}

bool UITabLayout::SelectItem(int iIndex, bool triggerEvent) {
    if( iIndex < 0 || iIndex >= m_items.GetSize() ) return false;
    if( iIndex == m_curSel ) return true;

    int iOldSel = m_curSel;
    m_curSel = iIndex;
    for( int it = 0; it < m_items.GetSize(); it++ )
    {
        if( it == iIndex ) {
            GetItemAt(it)->SetVisible(true);
            GetItemAt(it)->SetFocus();
        }
        else GetItemAt(it)->SetVisible(false);
    }
    NeedParentUpdate();

    if( m_manager != NULL ) {
        //TODO SetNextTabControl
        //m_manager->SetNextTabControl();
        if (triggerEvent) m_manager->SendNotify(this, DUI_MSGTYPE_TABSELECT, (WPARAM)(long)m_curSel, (LPARAM)(long)iOldSel);
    }
    return true;
}

bool UITabLayout::SelectItem(UIControl *control, bool triggerEvent) {
    int iIndex=GetItemIndex(control);
    if (iIndex==-1)
        return false;
    else
        return SelectItem(iIndex, triggerEvent);
}

void UITabLayout::SetPos(RECT rc, bool bNeedInvalidate) {
    UIControl::SetPos(rc, bNeedInvalidate);
    rc = m_rcItem;

    // Adjust for inset
    rc.left += m_rcInset.left;
    rc.top += m_rcInset.top;
    rc.right -= m_rcInset.right;
    rc.bottom -= m_rcInset.bottom;

    for( int it = 0; it < m_items.GetSize(); it++ ) {
        auto* pControl = static_cast<UIControl*>(m_items[it]);
        if( !pControl->IsVisible() ) continue;
        if( pControl->IsFloat() ) {
            SetFloatPos(it);
            continue;
        }

        if( it != m_curSel ) continue;

        RECT rcPadding = pControl->GetPadding();
        rc.left += rcPadding.left;
        rc.top += rcPadding.top;
        rc.right -= rcPadding.right;
        rc.bottom -= rcPadding.bottom;

        SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };

        SIZE sz = pControl->EstimateSize(szAvailable);
        if( sz.cx == 0 ) {
            sz.cx = MAX((long)0, szAvailable.cx);
        }
        if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
        if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();

        if(sz.cy == 0) {
            sz.cy = MAX((long)0, szAvailable.cy);
        }
        if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
        if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();

        RECT rcCtrl = { rc.left, rc.top, rc.left + sz.cx, rc.top + sz.cy};
        pControl->SetPos(rcCtrl, false);
    }
}

void UITabLayout::SetAttribute(const char *pstrName, const char *pstrValue) {
    if( strcasecmp(pstrName, "selectedid") == 0 ) SelectItem(atoi(pstrValue));
    return UIContainer::SetAttribute(pstrName, pstrValue);
}
