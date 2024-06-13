#include <UIPaintManager.h>
#include <UIDefine.h>
#include <cassert>
#include <UIRect.h>
#include <cstring>
#include <iostream>

using namespace std;

bool  UIPaintManager::m_useHSL = false;
short UIPaintManager::m_H = 180;
short UIPaintManager::m_S = 100;
short UIPaintManager::m_L = 100;

typedef struct tagFINDTABINFO
{
    UIControl   *pFocus;
    UIControl   *pLast;
    bool        bForward;
    bool        bNextIsIt;
}FINDTABINFO;

typedef struct tagFINDSHORTCUT
{
    char    ch;
    bool    bPickNext;
}FINDSHORTCUT;


tagTDrawInfo::tagTDrawInfo() {
    Clear();
}

tagTDrawInfo::tagTDrawInfo(const char *lpsz) {
    Clear();
    sDrawString = lpsz;
}

void tagTDrawInfo::Clear()
{
    sDrawString.Empty();
    sImageName.Empty();
    memset(&bLoaded, 0, sizeof(tagTDrawInfo) - offsetof(tagTDrawInfo, bLoaded));
    uFade = 255;
}

UIPaintManager::~UIPaintManager(){
    for(int i=0;i<m_aDelayedCleanup.GetSize();i++){
        static_cast<UIControl*>(m_aDelayedCleanup[i])->Delete();
    }
    m_aDelayedCleanup.Empty();
    m_mNameHash.Resize(0);
	if(m_pRoot != nullptr){
		m_pRoot->Delete();
	}
    this->RemoveAllDefaultAttributeList();
    this->RemoveAllOptionGroups();
    this->RemoveAllTimers();
}

void UIPaintManager::ReapObjects(UIControl *control) {

}

void UIPaintManager::AddDelayedCleanup(UIControl *pControl) {
    if (pControl == nullptr) return;
    pControl->SetManager(this, nullptr, false);
    m_aDelayedCleanup.Add(pControl);
    //PostAsyncNotify();
}

void UIPaintManager::AddMouseLeaveNeeded(UIControl *pControl) {
    if (pControl == nullptr) return;
    for( int i = 0; i < m_aNeedMouseLeaveNeeded.GetSize(); i++ ) {
        if( static_cast<UIControl*>(m_aNeedMouseLeaveNeeded[i]) == pControl ) {
            return;
        }
    }
    m_aNeedMouseLeaveNeeded.Add(pControl);
}

bool UIPaintManager::RemoveMouseLeaveNeeded(UIControl *control) {
    if (control == nullptr) return false;
    for( int i = 0; i < m_aNeedMouseLeaveNeeded.GetSize(); i++ ) {
        if( static_cast<UIControl*>(m_aNeedMouseLeaveNeeded[i]) == control ) {
            return m_aNeedMouseLeaveNeeded.Remove(i);
        }
    }
    return false;
}

bool UIPaintManager::RenameControl(UIControl *control, const UIString &name) {
    assert(control);
    if( control == nullptr || control->GetManager() != this || name.IsEmpty()) return false;
    if (control->GetName() == name) return true;
    if (nullptr != FindControl(name.GetData())) return false;
    m_mNameHash.Remove(control->GetName());
    bool bResult = m_mNameHash.Insert(name, control);
    if (bResult) control->SetName(name);
    return bResult;
}

UIControl *UIPaintManager::GetFocus() const {
    return m_pFocus;
}

void UIPaintManager::SetFocus(UIControl *pControl, bool bFocusWnd) {
// Paint manager window has focus?
#ifdef WIN32
    HWND hFocusWnd = ::GetFocus();
    if( bFocusWnd && hFocusWnd != m_paintWnd && pControl != m_pFocus && !m_bNoActivate) ::SetFocus(m_paintWnd);
#else
    GtkWidget *focusWidget = gtk_window_get_focus(GTK_WINDOW(m_paintWnd));
    if( bFocusWnd && focusWidget != m_paintWnd && pControl != m_pFocus && !m_bNoActivate) {
        gtk_window_set_focus(GTK_WINDOW(m_paintWnd), m_paintWnd);
    }
#endif
    // Already has focus?
    if( pControl == m_pFocus ) return;
    // Remove focus from old control
    if( m_pFocus != nullptr )
    {
        TEventUI event = { 0 };
        event.Type = UIEVENT_KILLFOCUS;
        event.pSender = pControl;
        event.dwTimestamp = UIGetTickCount();
        m_pFocus->Event(event);
        SendNotify(m_pFocus, DUI_MSGTYPE_KILLFOCUS);
        m_pFocus = nullptr;
    }
    if( pControl == nullptr ) return;
    // Set focus to new control
    if( pControl->GetManager() == this
        && pControl->IsVisible()
        && pControl->IsEnabled() )
    {
        m_pFocus = pControl;
        TEventUI event = { 0 };
        event.Type = UIEVENT_SETFOCUS;
        event.pSender = pControl;
        event.dwTimestamp = UIGetTickCount();
        m_pFocus->Event(event);
        SendNotify(m_pFocus, DUI_MSGTYPE_SETFOCUS);
    }
}

void UIPaintManager::NeedUpdate() {
    m_bUpdateNeeded = true;
}

bool UIPaintManager::GetHSL(short *H, short *S, short *L) {
    *H = m_H;
    *S = m_S;
    *L = m_L;
    return m_useHSL;
}

bool UIPaintManager::AddNotifier(INotifyUI *notifier) {
    if(notifier == nullptr){
        return false;
    }
    assert(m_aNotifiers.Find(notifier)<0);
    return m_aNotifiers.Add(notifier);
}

bool UIPaintManager::RemoveNotifier(INotifyUI *notifier) {
    for( int i = 0; i < m_aNotifiers.GetSize(); i++ ) {
        if( static_cast<INotifyUI*>(m_aNotifiers[i]) == notifier ) {
            return m_aNotifiers.Remove(i);
        }
    }
    return false;
}

void UIPaintManager::SendNotify(TNotifyUI &Msg, bool bAsync, bool bEnableRepeat) {

    Msg.ptMouse = m_ptLastMousePos;
    Msg.dwTimestamp = UIGetTickCount();
    //TODO virtualWnd
    /*if( m_bUsedVirtualWnd )
    {
        Msg.sVirtualWnd = Msg.pSender->GetVirtualWnd();
    }*/

    if( !bAsync ) {
        // Send to all listeners
        if( Msg.pSender != NULL ) {
            if( Msg.pSender->OnNotify ) Msg.pSender->OnNotify(&Msg);
        }
        for( int i = 0; i < m_aNotifiers.GetSize(); i++ ) {
            static_cast<INotifyUI*>(m_aNotifiers[i])->Notify(Msg);
        }
    }
    else {
        //TODO asynchronous
#if 0
        if( !bEnableRepeat ) {
            for( int i = 0; i < m_aAsyncNotify.GetSize(); i++ ) {
                TNotifyUI* pMsg = static_cast<TNotifyUI*>(m_aAsyncNotify[i]);
                if( pMsg->pSender == Msg.pSender && pMsg->sType == Msg.sType) {
                    if (m_bUsedVirtualWnd) pMsg->sVirtualWnd = Msg.sVirtualWnd;
                    pMsg->wParam = Msg.wParam;
                    pMsg->lParam = Msg.lParam;
                    pMsg->ptMouse = Msg.ptMouse;
                    pMsg->dwTimestamp = Msg.dwTimestamp;
                    return;
                }
            }
        }

        TNotifyUI *pMsg = new TNotifyUI;
        if (m_bUsedVirtualWnd) pMsg->sVirtualWnd = Msg.sVirtualWnd;
        pMsg->pSender = Msg.pSender;
        pMsg->sType = Msg.sType;
        pMsg->wParam = Msg.wParam;
        pMsg->lParam = Msg.lParam;
        pMsg->ptMouse = Msg.ptMouse;
        pMsg->dwTimestamp = Msg.dwTimestamp;
        m_aAsyncNotify.Add(pMsg);

        PostAsyncNotify();
#endif
    }
}

void UIPaintManager::SendNotify(UIControl *pControl, const char *pstrMessage, WPARAM wParam, LPARAM lParam, bool bAsync,
                                bool bEnableRepeat) {
    TNotifyUI Msg;
    Msg.pSender = pControl;
    Msg.sType = pstrMessage;
    Msg.wParam = wParam;
    Msg.lParam = lParam;
    SendNotify(Msg, bAsync, bEnableRepeat);
}

void UIPaintManager::UsedVirtualWnd(bool used) {

}

RECT &UIPaintManager::GetCaptionRect() {
    return m_rcCaption;
}

void UIPaintManager::SetCaptionRect(RECT &rcCaption) {
    m_rcCaption = rcCaption;
}

void UIPaintManager::SetRoundCorner(SIZE roundCorner) {
    m_roundCorner = roundCorner;
}

SIZE UIPaintManager::GetRoundCorner() const {
    return m_roundCorner;
}

SIZE UIPaintManager::GetInitSize()const {
    return m_szInitWindowSize;
}

RECT UIPaintManager::GetSizeBox()const {
    return m_rcSizeBox;
}

void UIPaintManager::SetSizeBox(RECT &rcSizeBox) {
    m_rcSizeBox = rcSizeBox;
}

SIZE UIPaintManager::GetMinInfo() const {
    return m_szMinWindow;
}

void UIPaintManager::SetMinInfo(int cx, int cy) {
    assert(cx>=0 && cy>=0);
    m_szMinWindow.cx = cx;
    m_szMinWindow.cy = cy;
}

SIZE UIPaintManager::GetMaxInfo() const {
    return m_szMaxWindow;
}

void UIPaintManager::SetMaxInfo(int cx, int cy) {
    assert(cx>=0 && cy>=0);
    m_szMaxWindow.cx = cx;
    m_szMaxWindow.cy = cy;
}

bool UIPaintManager::AttachDialog(UIControl *pControl) {
    m_pRoot = pControl;
    m_bUpdateNeeded = true;
    return InitControls(pControl);
}

UIPtrArray *UIPaintManager::GetFoundControls() {
    return &m_aFoundControls;
}

UIControl *UIPaintManager::GetRoot() const {
    return m_pRoot;
}

UIControl *UIPaintManager::FindControl(POINT pt) const {
    //assert(m_pRoot);
    return m_pRoot->FindControl(__FindControlFromPoint, &pt, UIFIND_VISIBLE|UIFIND_HITTEST|UIFIND_TOP_FIRST);
}

UIControl *UIPaintManager::FindControl(const char *pstrName) const {
    assert(m_pRoot);
    return static_cast<UIControl*>(m_mNameHash.Find(UIString{pstrName}));
}

UIControl *UIPaintManager::FindSubControlByPoint(UIControl *parent, POINT pt) const {
    if(parent == nullptr){
        parent = GetRoot();
    }
    assert(parent);
    return parent->FindControl(__FindControlFromPoint, &pt, UIFIND_VISIBLE|UIFIND_HITTEST|UIFIND_TOP_FIRST);
}

UIControl *UIPaintManager::FindSubControlByName(UIControl *parent, const char *pstrName) const {
    if(parent == nullptr){
        parent = GetRoot();
    }
    assert(parent);
    return parent->FindControl(__FindControlFromName, (LPVOID)pstrName, UIFIND_ALL);
}

UIControl *UIPaintManager::FindSubControlByClass(UIControl *parent, const char *pstrClass, int iIndex) {
    if(parent == nullptr){
        parent = GetRoot();
    }
    assert(parent);
    m_aFoundControls.Resize(iIndex+1);
    return parent->FindControl(__FindControlFromClass, (LPVOID)pstrClass, UIFIND_ALL);
}

UIPtrArray *UIPaintManager::FindSubControlsByClass(UIControl *parent, const char *pstrClass) {
    if(parent == nullptr){
        parent = GetRoot();
    }
    assert(parent);
    m_aFoundControls.Empty();
    parent->FindControl(__FindControlsFromClass, (LPVOID)pstrClass, UIFIND_ALL);
    return &m_aFoundControls;
}

UIControl *UIPaintManager::__FindControlFromNameHash(UIControl *pThis, LPVOID pData) {
    auto* pManager = static_cast<UIPaintManager*>(pData);
    const UIString sName = pThis->GetName();
    if( sName.IsEmpty() ) return nullptr;
    // Add this control to the hash list
    pManager->m_mNameHash.Set(sName, pThis);
    return nullptr; // Attempt to add all controls
}

UIControl *UIPaintManager::__FindControlFromPoint(UIControl *pThis, LPVOID pData) {
    auto point = static_cast<LPPOINT>(pData);
    UIRect posTemp {pThis->GetPos()};
    return posTemp.IsPtIn(*point) ? pThis:nullptr;
}

UIControl *UIPaintManager::__FindControlFromTab(UIControl *pThis, LPVOID pData) {
    auto* pInfo = static_cast<FINDTABINFO*>(pData);
    if( pInfo->pFocus == pThis ) {
        if( pInfo->bForward ) pInfo->bNextIsIt = true;
        return pInfo->bForward ? nullptr : pInfo->pLast;
    }
    if( (pThis->GetControlFlags() & UIFLAG_TABSTOP) == 0 ) return NULL;
    pInfo->pLast = pThis;
    if( pInfo->bNextIsIt ) return pThis;
    if( pInfo->pFocus == nullptr ) return pThis;
    return nullptr;  // Examine all controls
}

UIControl *UIPaintManager::__FindControlFromShortcut(UIControl *pThis, LPVOID pData) {
    if( !pThis->IsVisible() ) return nullptr;
    auto* pFS = static_cast<FINDSHORTCUT*>(pData);
    if( pFS->ch == toupper(pThis->GetShortcut()) ) pFS->bPickNext = true;
    if( pThis->GetClass().Find( DUI_CTR_LABEL) != -1 ) return nullptr;   // Labels never get focus!
    return pFS->bPickNext ? pThis : nullptr;
}

UIControl *UIPaintManager::__FindControlFromName(UIControl *pThis, LPVOID pData) {
    const char *pstrName = static_cast<const char *>(pData);
    const UIString& sName = pThis->GetName();
    if( sName.IsEmpty() ) return nullptr;
    return (sName==pstrName) ? pThis : nullptr;
}

UIControl *UIPaintManager::__FindControlFromClass(UIControl *pThis, LPVOID pData) {
    const char *pstrType = static_cast<const char *>(pData);
    UIString pType = pThis->GetClass();
    UIPtrArray* pFoundControls = pThis->GetManager()->GetFoundControls();
    if( strcmp(pstrType, "*") == 0 || pType == pstrType ) {
        int iIndex = -1;
        while( pFoundControls->GetAt(++iIndex) != nullptr ) ;
        if( iIndex < pFoundControls->GetSize() ) pFoundControls->SetAt(iIndex, pThis);
    }
    if( pFoundControls->GetAt(pFoundControls->GetSize() - 1) != nullptr ) return pThis;
    return nullptr;
}

UIControl *UIPaintManager::__FindControlsFromClass(UIControl *pThis, LPVOID pData) {
    const char* pstrType = static_cast<const char*>(pData);
    UIString pType = pThis->GetClass();
    if( strcmp(pstrType, "*") == 0 || pType==pstrType )
        pThis->GetManager()->GetFoundControls()->Add((LPVOID)pThis);
    return nullptr;
}

UIControl *UIPaintManager::__FindControlsFromUpdate(UIControl *pThis, LPVOID pData) {
    if( pThis->IsUpdateNeeded() ) {
        pThis->GetManager()->GetFoundControls()->Add((LPVOID)pThis);
        return pThis;
    }
    return nullptr;
}

UIControl *UIPaintManager::__FindControlFromUpdate(UIControl *pThis, LPVOID pData) {
    return pThis->IsUpdateNeeded()?pThis:nullptr;
}

bool UIPaintManager::InitControls(UIControl *control, UIControl *parent) {
    assert(control);
    if( control == nullptr ) return false;
    control->SetManager(this, parent != NULL ? parent : control->GetParent(), true);
    control->FindControl(__FindControlFromNameHash, this, UIFIND_ALL);
    return true;
}

bool UIPaintManager::IsCaptured() const {
    return m_bMouseCapture;
}

void UIPaintManager::SetWindowAttribute(const char *pstrName, const char *pstrValue) {
    if( strcasecmp(pstrName, "size") == 0 ) {
        char *pstr = nullptr;
        int cx = strtol(pstrValue, &pstr, 10);  assert(pstr);
        int cy = strtol(pstr + 1, &pstr, 10);    assert(pstr);
        SetInitSize(cx, cy);
    }
    else if( strcasecmp(pstrName, "sizebox") == 0 ) {
        RECT rcSizeBox = { 0 };
        char *pstr = nullptr;
        rcSizeBox.left = strtol(pstrValue, &pstr, 10);  assert(pstr);
        rcSizeBox.top = strtol(pstr + 1, &pstr, 10);    assert(pstr);
        rcSizeBox.right = strtol(pstr + 1, &pstr, 10);  assert(pstr);
        rcSizeBox.bottom = strtol(pstr + 1, &pstr, 10); assert(pstr);
        SetSizeBox(rcSizeBox);
    }
    else if( strcasecmp(pstrName, "caption") == 0 ) {
        RECT rcCaption = { 0 };
        char *pstr = nullptr;
        rcCaption.left = strtol(pstrValue, &pstr, 10);  assert(pstr);
        rcCaption.top = strtol(pstr + 1, &pstr, 10);    assert(pstr);
        rcCaption.right = strtol(pstr + 1, &pstr, 10);  assert(pstr);
        rcCaption.bottom = strtol(pstr + 1, &pstr, 10); assert(pstr);
        SetCaptionRect(rcCaption);
    }
    else if( strcasecmp(pstrName, "roundcorner") == 0 ) {
        char *pstr = nullptr;
        int cx = strtol(pstrValue, &pstr, 10);  assert(pstr);
        int cy = strtol(pstr + 1, &pstr, 10);    assert(pstr);
        SetRoundCorner(SIZE{cx, cy});
    }
    else if( strcasecmp(pstrName, "mininfo") == 0 ) {
        char *pstr = nullptr;
        int cx = strtol(pstrValue, &pstr, 10);  assert(pstr);
        int cy = strtol(pstr + 1, &pstr, 10);    assert(pstr);
        SetMinInfo(cx, cy);
    }
    else if( strcasecmp(pstrName, "maxinfo") == 0 ) {
        char *pstr = nullptr;
        int cx = strtol(pstrValue, &pstr, 10);  assert(pstr);
        int cy = strtol(pstr + 1, &pstr, 10);    assert(pstr);
        SetMaxInfo(cx, cy);
    }
    else if( strcasecmp(pstrName, "disabledfontcolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtol(pstrValue, &pstr, 16);
        SetDefaultDisabledColor(clrColor);
    }
    else if( strcasecmp(pstrName, "defaultfontcolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtol(pstrValue, &pstr, 16);
        SetDefaultFontColor(clrColor);
    }
    else if( strcasecmp(pstrName, "linkfontcolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtol(pstrValue, &pstr, 16);
        SetDefaultLinkFontColor(clrColor);
    }
    else if( strcasecmp(pstrName, "linkhoverfontcolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtol(pstrValue, &pstr, 16);
        SetDefaultLinkHoverFontColor(clrColor);
    }
    else if( strcasecmp(pstrName, "selectedcolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtol(pstrValue, &pstr, 16);
        SetDefaultSelectedBkColor(clrColor);
    }
    //else
    //    AddWindowCustomAttribute(pstrName, pstrValue);
}

void UIPaintManager::SetDefaultDisabledColor(uint32_t disabledColor) {
    m_defaultDisabledColor = disabledColor;
}

uint32_t UIPaintManager::GetDefaultDisabledColor() const {
    return m_defaultDisabledColor;
}

void UIPaintManager::SetDefaultFontColor(uint32_t fontColor) {
    m_defaultFontColor = fontColor;
}

uint32_t UIPaintManager::GetDefaultFontColor() const {
    return m_defaultFontColor;
}

void UIPaintManager::SetDefaultLinkFontColor(uint32_t linkFontColor) {
    m_defaultLinkFontColor = linkFontColor;
}

uint32_t UIPaintManager::GetDefaultLinkFontColor() const {
    return m_defaultLinkFontColor;
}

void UIPaintManager::SetDefaultLinkHoverFontColor(uint32_t linkHoverFontColor) {
    m_defaultLinkHoverFontColor = linkHoverFontColor;
}

uint32_t UIPaintManager::GetDefaultLinkHoverFontColor() const {
    return m_defaultLinkHoverFontColor;
}

void UIPaintManager::SetDefaultSelectedBkColor(uint32_t selectedBkColor) {
    m_defaultSelectedBkColor = selectedBkColor;
}

uint32_t UIPaintManager::GetDefaultSelectedBkColor() const {
    return m_defaultSelectedBkColor;
}

UIPtrArray *UIPaintManager::GetPlugins() {
    return nullptr;
}

void UIPaintManager::AddDefaultAttributeList(const char *pStrControlName, const char *pStrControlAttrList, bool shared) {
    auto *pDefaultAttribute = new UIString{pStrControlAttrList};
    UIString *oldDefaultAttr = static_cast<UIString*>(
            m_defaultAttributesMapping.Set(UIString{pStrControlName},
                                           pDefaultAttribute));
    delete oldDefaultAttr;
}

const char *UIPaintManager::GetDefaultAttributeList(const char *pStrControlName) const {
    UIString *attribute = static_cast<UIString*>(m_defaultAttributesMapping.Find(UIString{pStrControlName}));
    if(attribute == nullptr){
        return "";
    }
    return attribute->GetData();
}

void UIPaintManager::RemoveAllDefaultAttributeList() {
    for(int i=0;i<m_defaultAttributesMapping.GetSize();i++){
        UIString key = m_defaultAttributesMapping.GetAt(i);
        auto *attribute = static_cast<UIString*>(m_defaultAttributesMapping.Find(key));
        delete attribute;
    }
    m_defaultAttributesMapping.RemoveAll();
}

POINT UIPaintManager::GetMousePos() const {
    return m_ptLastMousePos;
}

bool UIPaintManager::AddOptionGroup(const UIString &groupName, UIControl *control) {
    if (control == NULL || groupName.IsEmpty()) return false;

    LPVOID lp = m_optionGroup.Find(groupName);
    if( lp ) {
        auto* aOptionGroup = static_cast<UIPtrArray*>(lp);
        for( int i = 0; i < aOptionGroup->GetSize(); i++ ) {
            if( static_cast<UIControl*>(aOptionGroup->GetAt(i)) == control ) {
                return false;
            }
        }
        aOptionGroup->Add(control);
    }
    else {
        auto* aOptionGroup = new UIPtrArray(6);
        aOptionGroup->Add(control);
        m_optionGroup.Insert(groupName, aOptionGroup);
    }
    return true;
}

UIPtrArray *UIPaintManager::GetOptionGroup(const UIString &groupName) {
    LPVOID lp = m_optionGroup.Find(groupName);
    if( lp ) return static_cast<UIPtrArray*>(lp);
    return nullptr;
}

void UIPaintManager::RemoveOptionGroup(const UIString &groupName, UIControl *control) {
    LPVOID lp = m_optionGroup.Find(groupName);
    if( lp ) {
        auto* aOptionGroup = static_cast<UIPtrArray*>(lp);
        for( int i = 0; i < aOptionGroup->GetSize(); i++ ) {
            if( static_cast<UIControl*>(aOptionGroup->GetAt(i)) == control ) {
                aOptionGroup->Remove(i);
                break;
            }
        }
        if( aOptionGroup->IsEmpty() ) {
            delete aOptionGroup;
            m_optionGroup.Remove(groupName);
        }
    }
}

void UIPaintManager::RemoveAllOptionGroups() {
    UIPtrArray* aOptionGroup;
    for( int i = 0; i< m_optionGroup.GetSize(); i++ ) {
        UIString key = m_optionGroup.GetAt(i);
        if(!key.IsEmpty()) {
            aOptionGroup = static_cast<UIPtrArray*>(m_optionGroup.Find(key));
            delete aOptionGroup;
        }
    }
    m_optionGroup.RemoveAll();
}
