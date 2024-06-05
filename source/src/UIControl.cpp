#include <UIControl.h>
#include <UIPaintManager.h>
#include <UIRenderEngine.h>
#include <UIRect.h>
#include <cassert>
#include <cstring>
#include "UIRenderClip.h"

UIControl::UIControl()
    :m_manager{nullptr},
    m_cover{nullptr},
    m_parent {nullptr},
    m_updateNeeded {true},
    m_menuUsed {false},
    m_asyncNotify{false},
    m_visible {true},
    m_internVisible{true},
    m_focused {false},
    m_enabled{true},
    m_mouseEnabled {true},
    m_keyboardEnabled{true},
    m_float { false},
    m_setPos {false},
    m_shortCut {'\0'},
    m_tag {nullptr},
    m_backColor {0},
    m_backColor2 {0},
    m_backColor3 {0},
    m_borderColor {0},
    m_focusBorderColor {0},
    m_colorHSL {false},
    m_borderStyle {0}
{
    m_cXY.cx = m_cXY.cy = 0;
    m_cxyFixed.cx = m_cxyFixed.cy = 0;
    m_cxyMin.cx = m_cxyMin.cy = 0;
    m_cxyMax.cx = m_cxyMax.cy = 9999;
    m_cxyBorderRound.cx = m_cxyBorderRound.cy = 0;

    memset(&m_rcPadding, 0, sizeof(RECT));
    memset(&m_rcItem, 0, sizeof(RECT));
    memset(&m_rcPaint, 0, sizeof(RECT));
    memset(&m_rcBorderSize,0, sizeof(RECT));
    m_piFloatPercent.left = m_piFloatPercent.top = m_piFloatPercent.right = m_piFloatPercent.bottom = 0.0f;
}

UIControl::~UIControl() {
    if(m_cover != nullptr){
        m_cover->Delete();
        m_cover = nullptr;
    }
    RemoveAllCustomeAttribute();
    if(OnDestroy){
        OnDestroy(this);
    }
    if(m_manager != nullptr){
        m_manager->ReapObjects(this);
    }
}

void UIControl::Delete() {
    if(m_manager){
        m_manager->RemoveMouseLeaveNeeded(this);
    }
    delete this;
}

UIString UIControl::GetName() const {
    return m_name;
}

void UIControl::SetName(const UIString &name) {
    if(m_name != name){
        m_name = name;
        if(m_manager != nullptr){
            m_manager->RenameControl(this, name);
        }
    }
}

UIString UIControl::GetClass() const {
    return UIString{DUI_CTR_CONTROL};
}

LPVOID UIControl::GetInterface(const UIString &name) {
    if(strcmp(name.GetData(), DUI_CTR_CONTROL)==0){
        return this;
    }
    return nullptr;
}

uint32_t UIControl::GetControlFlags() const {
    return 0;
}

HANDLE_WND UIControl::GetNativeWindow() const {
    return nullptr;
}

bool UIControl::Activate() {
    if(!IsVisible()){
        return false;
    }
    if(!IsEnabled()){
        return false;
    }
    return true;
}

UIPaintManager *UIControl::GetManager() const {
    return m_manager;
}

void UIControl::SetManager(UIPaintManager *manager, UIControl *parent, bool bInit) {
    if( m_cover != nullptr ) m_cover->SetManager(manager, this, bInit);
    m_manager = manager;
    m_parent = parent;
    if(bInit && m_parent){
        Init();
    }
}

UIControl *UIControl::GetParent() const {
    return m_parent;
}

UIControl *UIControl::GetCover() const {
    return m_cover;
}

void UIControl::SetCover(UIControl *pControl) {
    if( m_cover == pControl ) return;
    if( m_cover != nullptr ) m_cover->Delete();
    m_cover = pControl;
    if( m_cover != nullptr ) {
        m_manager->InitControls(m_cover, this);
        if( IsVisible() ) NeedUpdate();
        else pControl->SetInternVisible(false);
    }
}

UIString UIControl::GetText() const {
    return m_text;
}

void UIControl::SetText(const UIString &text) {
    if(m_text == text){
        return;
    }
    m_text =text;
    Invalidate();
}

uint32_t UIControl::GetBkColor() const {
    return m_backColor;
}

void UIControl::SetBkColor(uint32_t backColor) {
    if(m_backColor == backColor){
        return;
    }
    m_backColor = backColor;
    Invalidate();
}

uint32_t UIControl::GetBkColor2() const {
    return m_backColor2;
}

void UIControl::SetBkColor2(uint32_t backColor) {
    if(m_backColor2 == backColor){
        return;
    }
    m_backColor2 = backColor;
    Invalidate();
}

uint32_t UIControl::GetBkColor3() const {
    return m_backColor3;
}

void UIControl::SetBkColor3(uint32_t backColor) {
    if(m_backColor3 == backColor){
        return;
    }
    m_backColor3 = backColor;
    Invalidate();
}

UIString UIControl::GetBkImage() const {
    return m_diBk.sDrawString;
}

void UIControl::SetBkImage(const UIString &bkImage) {
    if(m_diBk.sDrawString == bkImage && m_diBk.pImageInfo != nullptr){
        return;
    }
    m_diBk.Clear();
    m_diBk.sDrawString = bkImage;
    DrawImage(nullptr, m_diBk);
    if(m_float && m_cxyFixed.cx==0 && m_cxyFixed.cy==0 && m_diBk.pImageInfo){
        m_cxyFixed.cx = m_diBk.pImageInfo->nX;
        m_cxyFixed.cy = m_diBk.pImageInfo->nY;
    }
    Invalidate();
}

uint32_t UIControl::GetFocusBorderColor() const {
    return m_focusBorderColor;
}

void UIControl::SetFocusBorderColor(uint32_t borderColor) {
    if(m_focusBorderColor == borderColor){
        return;
    }
    m_focusBorderColor = borderColor;
    Invalidate();
}

bool UIControl::IsColorHSL() const {
    return m_colorHSL;
}

void UIControl::SetColorHSL(bool colorHSL) {
    if(m_colorHSL == colorHSL){
        return;
    }
    m_colorHSL = colorHSL;
    Invalidate();
}

SIZE UIControl::GetBorderRound() const {
    return m_cxyBorderRound;
}

void UIControl::SetBorderRound(SIZE cxyRound) {
    if(m_cXY.cx == cxyRound.cx && m_cXY.cy == cxyRound.cy){
        return;
    }
    m_cxyBorderRound = cxyRound;
    Invalidate();
}

bool UIControl::DrawImage(HANDLE_DC hDC, TDrawInfo &drawInfo) {
    return UIRenderEngine::DrawImage(hDC, m_manager, m_rcItem, m_rcPaint, drawInfo);
}

uint32_t UIControl::GetBorderColor() const {
    return m_borderColor;
}

void UIControl::SetBorderColor(uint32_t borderColor) {
    if(m_borderColor == borderColor){
        return;
    }
    m_borderColor = borderColor;
    Invalidate();
}

RECT UIControl::GetBorderSize() const {
    return m_rcBorderSize;
}

void UIControl::SetBorderSize(RECT rc) {
    m_rcBorderSize = rc;
    Invalidate();
}

void UIControl::SetBorderSize(int iSize) {
    m_rcBorderSize.left = m_rcBorderSize.top = m_rcBorderSize.right = m_rcBorderSize.bottom = iSize;
}

int UIControl::GetBorderStyle() const {
    return m_borderStyle;
}

void UIControl::SetBorderStyle(int nStyle) {
    if(m_borderStyle == nStyle){
        return;
    }
    m_borderStyle = nStyle;
    Invalidate();
}

const RECT &UIControl::GetPos() const {
    return m_rcItem;
}

RECT UIControl::GetRelativePos() const {
    UIControl* pParent = GetParent();
    if( pParent != nullptr ) {
        RECT rcParentPos = pParent->GetPos();
        UIRect rcRelativePos(m_rcItem);
        rcRelativePos.Offset(-rcParentPos.left, -rcParentPos.top);
        return rcRelativePos;
    }
    return UIRect(0, 0, 0, 0);
}

RECT UIControl::GetClientPos() const {
    return m_rcItem;
}

void UIControl::SetPos(RECT rc, bool bNeedInvalidate) {
    if( rc.right < rc.left ) rc.right = rc.left;
    if( rc.bottom < rc.top ) rc.bottom = rc.top;

    UIRect invalidateRc { m_rcItem };
    if( invalidateRc.IsEmpty() ) invalidateRc = UIRect{rc};

    if( m_float ) {
        UIControl* parent = GetParent();
        if( parent != nullptr ) {
            RECT rcParentPos = parent->GetPos();
            RECT rcCtrl = {rcParentPos.left + rc.left, rcParentPos.top + rc.top,
                           rcParentPos.left + rc.right, rcParentPos.top + rc.bottom};
            m_rcItem = rcCtrl;

            long width = rcParentPos.right - rcParentPos.left;
            long height = rcParentPos.bottom - rcParentPos.top;
            RECT rcPercent = {(long)(width*m_piFloatPercent.left), (long)(height*m_piFloatPercent.top),
                              (long)(width*m_piFloatPercent.right), (long)(height*m_piFloatPercent.bottom)};
            m_cXY.cx = rc.left - rcPercent.left;
            m_cXY.cy = rc.top - rcPercent.top;
            m_cxyFixed.cx = rc.right - rcPercent.right - m_cXY.cx;
            m_cxyFixed.cy = rc.bottom - rcPercent.bottom - m_cXY.cy;
        }
    }
    else {
        m_rcItem = rc;
    }
    if( m_manager == nullptr ) return;

    if( !m_setPos ) {
        m_setPos = true;
        if( OnSize ) OnSize(this);
        m_setPos = false;
    }

    m_updateNeeded = false;

    if( bNeedInvalidate && IsVisible() ) {
        invalidateRc.Join(m_rcItem);
        UIControl* parent = this;
        RECT rcTemp;
        RECT rcParent;
        while( (parent = parent->GetParent()) != nullptr ) {
            if( !parent->IsVisible() ) return;
            rcTemp = invalidateRc;
            rcParent = parent->GetPos();
            if( !::UIIntersectRect(&invalidateRc, &rcTemp, &rcParent) ) return;
        }
        m_manager->Invalidate(invalidateRc);
    }

    if( m_cover != nullptr && m_cover->IsVisible() ) {
        if( m_cover->IsFloat() ) {
            SIZE szXY = m_cover->GetFixedXY();
            SIZE sz = {m_cover->GetFixedWidth(), m_cover->GetFixedHeight()};
            TPercentInfo rcPercent = m_cover->GetFloatPercent();
            long width = m_rcItem.right - m_rcItem.left;
            long height = m_rcItem.bottom - m_rcItem.top;
            RECT rcCtrl = { 0 };
            rcCtrl.left = (long)(width*rcPercent.left) + szXY.cx;
            rcCtrl.top = (long)(height*rcPercent.top) + szXY.cy;
            rcCtrl.right = (long)(width*rcPercent.right) + szXY.cx + sz.cx;
            rcCtrl.bottom = (long)(height*rcPercent.bottom) + szXY.cy + sz.cy;
            m_cover->SetPos(rcCtrl, false);
        }
        else {
            SIZE sz = { rc.right - rc.left, rc.bottom - rc.top };
            if( sz.cx < m_cover->GetMinWidth() ) sz.cx = m_cover->GetMinWidth();
            if( sz.cx > m_cover->GetMaxWidth() ) sz.cx = m_cover->GetMaxWidth();
            if( sz.cy < m_cover->GetMinHeight() ) sz.cy = m_cover->GetMinHeight();
            if( sz.cy > m_cover->GetMaxHeight() ) sz.cy = m_cover->GetMaxHeight();
            RECT rcCtrl = { rc.left, rc.top, rc.left + sz.cx, rc.top + sz.cy };
            m_cover->SetPos(rcCtrl, false);
        }
    }
}

void UIControl::Move(SIZE szOffset, bool bNeedInvalidate) {
    UIRect invalidateRc {m_rcItem};
    m_rcItem.left += szOffset.cx;
    m_rcItem.top += szOffset.cy;
    m_rcItem.right += szOffset.cx;
    m_rcItem.bottom += szOffset.cy;

    if( bNeedInvalidate && m_manager != nullptr && IsVisible() ) {
        invalidateRc.Join(m_rcItem);
        UIControl* parent = this;
        RECT rcTemp;
        RECT rcParent;
        while( (parent = parent->GetParent()) != nullptr ) {
            if( !parent->IsVisible() ) return;
            rcTemp = invalidateRc;
            rcParent = parent->GetPos();
            if( !::UIIntersectRect(&invalidateRc, &rcTemp, &rcParent) ) return;
        }
        m_manager->Invalidate(invalidateRc);
    }

    if( m_cover != nullptr && m_cover->IsVisible() ) m_cover->Move(szOffset, false);
}

int UIControl::GetWidth() const {
    return m_rcItem.right - m_rcItem.left;
}

int UIControl::GetHeight() const {
    return m_rcItem.bottom - m_rcItem.top;
}

int UIControl::GetX() const {
    return m_rcItem.left;
}

int UIControl::GetY() const {
    return m_rcItem.top;
}

RECT UIControl::GetPadding() const {
    return m_rcPadding;
}

void UIControl::SetPadding(RECT rcPadding) {
    m_rcPadding = rcPadding;
    NeedParentUpdate();
}

SIZE UIControl::GetFixedXY() const {
    return m_cXY;
}

void UIControl::SetFixedXY(SIZE szXY) {
    if(m_cXY.cx == szXY.cx && m_cXY.cy == szXY.cy){
        return;
    }
    m_cXY.cx = szXY.cx;
    m_cXY.cy = szXY.cy;
    NeedParentUpdate();
}

TPercentInfo UIControl::GetFloatPercent() const {
    return m_piFloatPercent;
}

void UIControl::SetFloatPercent(TPercentInfo piFloatPercent) {
    m_piFloatPercent = piFloatPercent;
    NeedParentUpdate();
}

int UIControl::GetFixedWidth() const {
    return m_cxyFixed.cx;
}

void UIControl::SetFixedWidth(int cx) {
    if(m_cxyFixed.cx == cx || cx<0){
        return;
    }
    m_cxyFixed.cx = cx;
    NeedParentUpdate();
}

int UIControl::GetFixedHeight() const {
    return m_cxyFixed.cy;
}

void UIControl::SetFixedHeight(int cy) {
    if(m_cxyFixed.cy==cy || cy<0){
        return;
    }
    m_cxyFixed.cy = cy;
    NeedParentUpdate();
}

int UIControl::GetMinWidth() const {
    return m_cxyMin.cx;
}

void UIControl::SetMinWidth(int cx) {
    if(m_cxyMin.cx == cx || cx<0){
        return;
    }
    m_cxyMin.cx = cx;
    NeedParentUpdate();
}

int UIControl::GetMaxWidth() const {
    if(m_cxyMax.cx < m_cxyMin.cx){
        return m_cxyMin.cx;
    }
    return m_cxyMax.cx;
}

void UIControl::SetMaxWidth(int cx) {
    if(m_cxyMax.cx == cx || cx < 0){
        return;
    }
    m_cxyMax.cx = cx;
    NeedParentUpdate();
}

int UIControl::GetMinHeight() const {
    return m_cxyMin.cy;
}

void UIControl::SetMinHeight(int cy) {
    if(m_cxyMin.cy == cy || cy<0){
        return;
    }
    m_cxyMin.cy = cy;
    NeedParentUpdate();
}

int UIControl::GetMaxHeight() const {
    if(m_cxyMax.cy < m_cxyMin.cy){
        return m_cxyMin.cy;
    }
    return m_cxyMax.cy;
}

void UIControl::SetMaxHeight(int cy) {
    if(m_cxyMax.cy==cy || cy<0){
        return;
    }
    m_cxyMax.cy = cy;
    NeedParentUpdate();
}

char UIControl::GetShortcut() const {
    return m_shortCut;
}

void UIControl::SetShortcut(char ch) {
    m_shortCut = ch;
}

bool UIControl::IsContextMenuUsed() const {
    return m_menuUsed;
}

void UIControl::SetContextMenuUsed(bool bMenuUsed) {
    m_menuUsed = bMenuUsed;
}

const UIString &UIControl::GetUserData() {
    return m_userData;
}

void UIControl::SetUserData(const char *pstrText) {
    m_userData = pstrText;
}

void *UIControl::GetTag() const {
    return m_tag;
}

void UIControl::SetTag(void *pTag) {
    m_tag = pTag;
}

bool UIControl::IsVisible() const {
    return m_visible && m_internVisible;
}

void UIControl::SetVisible(bool bVisible) {
    if(m_visible == bVisible){
        return;
    }
    bool v = IsVisible();
    m_visible = bVisible;
    if(m_focused){
        m_focused = false;
    }
    if(!bVisible && m_manager && m_manager->GetFocus()==this)
    {
        m_manager->SetFocus(nullptr);
    }
    if(IsVisible() != v){
        NeedParentUpdate();
    }
    if( m_cover != nullptr ) m_cover->SetInternVisible(IsVisible());
}

void UIControl::SetInternVisible(bool bVisible) {
    m_internVisible = bVisible;
    if (!bVisible && m_manager && m_manager->GetFocus() == this) {
        m_manager->SetFocus(nullptr) ;
    }
    if( m_cover != nullptr ) m_cover->SetInternVisible(IsVisible());
}

bool UIControl::IsEnabled() const {
    return m_enabled;
}

void UIControl::SetEnabled(bool bEnabled) {
    if(m_enabled == bEnabled){
        return;
    }
    m_enabled = bEnabled;
    Invalidate();
}

bool UIControl::IsMouseEnabled() const {
    return m_mouseEnabled;
}

void UIControl::SetMouseEnabled(bool bEnabled) {
    m_mouseEnabled = bEnabled;
}

bool UIControl::IsKeyboardEnabled() const {
    return m_keyboardEnabled;
}

void UIControl::SetKeyboardEnabled(bool bEnabled) {
    m_keyboardEnabled = bEnabled;
}

bool UIControl::IsFocused() const {
    return m_focused;
}

void UIControl::SetFocus() {
    if(m_manager){
        m_manager->SetFocus(this, false);
    }
}

bool UIControl::IsFloat() const {
    return m_float;
}

void UIControl::SetFloat(bool bFloat) {
    if(m_float == bFloat){
        return;
    }
    m_float = bFloat;
    NeedParentUpdate();
}

void UIControl::AddCustomAttribute(const char *name, const char *value) {
    if( name == nullptr || name[0] == '\0' || value == nullptr || value[0] == '\0' ) return;
    auto* pCostomAttr = new UIString(value);
    if (m_customAttrHash.Find(UIString{name}) == nullptr)
        m_customAttrHash.Set(UIString{name}, (LPVOID)pCostomAttr);
    else
        delete pCostomAttr;
}

const char *UIControl::GetCustomAttribute(const char *name) const {
    if( name == nullptr || name[0] == '\0' ) return nullptr;
    UIString* pCostomAttr = static_cast<UIString*>(m_customAttrHash.Find(UIString{name}));
    if( pCostomAttr ) return pCostomAttr->GetData();
    return nullptr;
}

bool UIControl::RemoveCustomAttribute(const char *name) {
    if( name == nullptr || name[0] == '\0' ) return false;
    UIString* pCostomAttr = static_cast<UIString*>(m_customAttrHash.Find(UIString{name}));
    if( !pCostomAttr ) return false;

    delete pCostomAttr;
    return m_customAttrHash.Remove(UIString{name});
}

void UIControl::RemoveAllCustomeAttribute() {
    UIString* pCostomAttr;
    for( int i = 0; i< m_customAttrHash.GetSize(); i++ ) {
        UIString key = m_customAttrHash.GetAt(i);
        if(!key.IsEmpty()) {
            pCostomAttr = static_cast<UIString*>(m_customAttrHash.Find(key));
            delete pCostomAttr;
        }
    }
    m_customAttrHash.Resize();
}

UIControl *UIControl::FindControl(FindControlProc Proc, LPVOID pData, uint32_t uFlags) {
    UIRect  rcRect {m_rcItem};
    if( (uFlags & UIFIND_VISIBLE) != 0 && !IsVisible() ) return nullptr;
    if( (uFlags & UIFIND_ENABLED) != 0 && !IsEnabled() ) return nullptr;
    if( (uFlags & UIFIND_HITTEST) != 0 && (!rcRect.IsPtIn(* static_cast<LPPOINT>(pData))) ) return nullptr;
    if( (uFlags & UIFIND_UPDATETEST) != 0 && Proc(this, pData) != nullptr ) return nullptr;

    UIControl* pResult = nullptr;
    if( (uFlags & UIFIND_ME_FIRST) != 0 ) {
        if( (uFlags & UIFIND_HITTEST) == 0 || IsMouseEnabled() ) pResult = Proc(this, pData);
    }
    if( pResult == nullptr && m_cover != nullptr ) {
        /*if( (uFlags & UIFIND_HITTEST) == 0 || true)*/ pResult = m_cover->FindControl(Proc, pData, uFlags);
    }
    if( pResult == nullptr && (uFlags & UIFIND_ME_FIRST) == 0 ) {
        if( (uFlags & UIFIND_HITTEST) == 0 || IsMouseEnabled() ) pResult = Proc(this, pData);
    }
    return pResult;
}

void UIControl::Invalidate() {
    if( !IsVisible() ) return;

    RECT invalidateRc = m_rcItem;

    UIControl* pParent = this;
    RECT rcTemp;
    RECT rcParent;
    while( (pParent = pParent->GetParent()) != nullptr )
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

bool UIControl::IsUpdateNeeded() const {
    return m_updateNeeded;
}

void UIControl::NeedUpdate() {
    if(!IsVisible()){
        return;
    }
    m_updateNeeded = true;
    Invalidate();
    if(m_manager != nullptr){
        m_manager->NeedUpdate();
    }
}

void UIControl::NeedParentUpdate() {
    if(GetParent()){
        GetParent()->NeedUpdate();
        GetParent()->Invalidate();
    }else{
        NeedUpdate();
    }
    if(m_manager != nullptr){
        m_manager->NeedUpdate();
    }
}

uint32_t UIControl::GetAdjustColor(uint32_t color) {
    if(!m_colorHSL){
        return color;
    }
    short H, S, L;
    UIPaintManager::GetHSL(&H, &S, &L);
    return UIRenderEngine::AdjustColor(color, H, S, L);
}

void UIControl::Init() {
    DoInit();
    if(OnInit){
        OnInit(this);
    }
}

void UIControl::DoInit() {

}

void UIControl::Event(TEventUI &event) {
    if(OnEvent(&event)){
        DoEvent(event);
    }
}

void UIControl::DoEvent(TEventUI &event) {
    if(event.Type == UIEVENT_SETCURSOR){
        UILoadCursor(m_manager, UI_IDC_ARROW)
        return;
    }
    if(event.Type == UIEVENT_SETFOCUS)
    {
        m_focused = true;
        Invalidate();
        return;
    }
    if(event.Type == UIEVENT_KILLFOCUS)
    {
        m_focused = false;
        Invalidate();
        return;
    }
    if(event.Type == UIEVENT_TIMER)
    {
        m_manager->SendNotify(this, DUI_MSGTYPE_TIMER, event.wParam, event.lParam);
        return;
    }
    if(event.Type == UIEVENT_CONTEXTMENU)
    {
        if(IsContextMenuUsed()){
            m_manager->SendNotify(this, DUI_MSGTYPE_MENU, event.wParam,event.lParam);
            return;
        }
    }
    if(m_parent != nullptr){
        m_parent->DoEvent(event);
    }
}

UIString UIControl::GetAttribute(const char *name) {
    return UIString {""};
}

void UIControl::SetAttribute(const char *name, const char *value) {
    if( strcasecmp(name, "pos") == 0 ) {
        RECT rcPos = { 0 };
        char *pstr = nullptr;
        rcPos.left = strtol(value, &pstr, 10);  assert(pstr);
        rcPos.top = strtol(pstr + 1, &pstr, 10);    assert(pstr);
        rcPos.right = strtol(pstr + 1, &pstr, 10);  assert(pstr);
        rcPos.bottom = strtol(pstr + 1, &pstr, 10); assert(pstr);
        SIZE szXY = {rcPos.left, rcPos.top};
        SetFixedXY(szXY);
        //ASSERT(rcPos.right - rcPos.left >= 0);
        //ASSERT(rcPos.bottom - rcPos.top >= 0);
        SetFixedWidth(rcPos.right - rcPos.left);
        SetFixedHeight(rcPos.bottom - rcPos.top);
    }
    else if(strcasecmp(name, "padding") == 0 ) {
        RECT rcPadding = { 0 };
        char *pstr = nullptr;
        rcPadding.left = strtol(value, &pstr, 10);  assert(pstr);
        rcPadding.top = strtol(pstr + 1, &pstr, 10);    assert(pstr);
        rcPadding.right = strtol(pstr + 1, &pstr, 10);  assert(pstr);
        rcPadding.bottom = strtol(pstr + 1, &pstr, 10); assert(pstr);
        SetPadding(rcPadding);
    }
    else if( strcasecmp(name, "bkcolor") == 0 || strcasecmp(name, "bkcolor1") == 0 ) {
        while( *value > '\0' && *value <= ' ' ) value = CharNext(value);
        if( *value == '#') value = CharNext(value);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(value, &pstr, 16);
        SetBkColor(clrColor);
    }
    else if( strcasecmp(name, "bkcolor2") == 0 ) {
        while( *value > '\0' && *value <= ' ' ) value = CharNext(value);
        if( *value == '#') value = CharNext(value);
        char    *pstr = nullptr;
        uint32_t clrColor = strtoul(value, &pstr, 16);
        SetBkColor2(clrColor);
    }
    else if( strcasecmp(name, "bkcolor3") == 0 ) {
        while( *value > '\0' && *value <= ' ' ) value = CharNext(value);
        if( *value == '#') value = CharNext(value);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(value, &pstr, 16);
        SetBkColor3(clrColor);
    }
    else if( strcasecmp(name, "bordercolor") == 0 ) {
        if( *value == '#') value = CharNext(value);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(value, &pstr, 16);
        SetBorderColor(clrColor);
    }
    else if( strcasecmp(name, "focusbordercolor") == 0 ) {
        if( *value == '#') value = CharNext(value);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(value, &pstr, 16);
        SetFocusBorderColor(clrColor);
    }
    else if( strcasecmp(name, "colorhsl") == 0 ) SetColorHSL(strcasecmp(value, "true") == 0);
    else if( strcasecmp(name, "bordersize") == 0 ) {
        UIString nValue {value};
        if(nValue.Find(',') < 0)
        {
            SetBorderSize(atoi(value));
        }
        else
        {
            RECT rcBorder = { 0 };
            char *pstr = nullptr;
            rcBorder.left = strtol(value, &pstr, 10);  assert(pstr);
            rcBorder.top = strtol(pstr + 1, &pstr, 10);    assert(pstr);
            rcBorder.right = strtol(pstr + 1, &pstr, 10);  assert(pstr);
            rcBorder.bottom = strtol(pstr + 1, &pstr, 10); assert(pstr);
            SetBorderSize(rcBorder);
        }
    }
    else if( strcasecmp(name, "borderstyle") == 0 ) SetBorderStyle(atoi(value));
    else if( strcasecmp(name, "borderround") == 0 ) {
        SIZE cxyRound = { 0 };
        char *pstr = nullptr;
        cxyRound.cx = strtol(value, &pstr, 10);  assert(pstr);
        cxyRound.cy = strtol(pstr + 1, &pstr, 10);    assert(pstr);
        SetBorderRound(cxyRound);
    }
    else if( strcasecmp(name, "bkimage") == 0 ) SetBkImage(UIString{value});
    else if( strcasecmp(name, "width") == 0 ) SetFixedWidth(atoi(value));
    else if( strcasecmp(name, "height") == 0 ) SetFixedHeight(atoi(value));
    else if( strcasecmp(name, "minwidth") == 0 ) SetMinWidth(atoi(value));
    else if( strcasecmp(name, "minheight") == 0 ) SetMinHeight(atoi(value));
    else if( strcasecmp(name, "maxwidth") == 0 ) SetMaxWidth(atoi(value));
    else if( strcasecmp(name, "maxheight") == 0 ) SetMaxHeight(atoi(value));
    else if( strcasecmp(name, "name") == 0 ) SetName(UIString{value});
    else if( strcasecmp(name, "text") == 0 ) SetText(UIString{value});
    //else if( _tcscmp(name, _T("tooltip")) == 0 ) SetToolTip(value);
    else if( strcasecmp(name, "userdata") == 0 ) SetUserData(value);
    else if( strcasecmp(name, "tag") == 0 ) SetTag(reinterpret_cast<void *>(atoi(value)));
    else if( strcasecmp(name, "enabled") == 0 ) SetEnabled(strcasecmp(value, "true") == 0);
    else if( strcasecmp(name, "mouse") == 0 ) SetMouseEnabled(strcasecmp(value, "true") == 0);
    else if( strcasecmp(name, "keyboard") == 0 ) SetKeyboardEnabled(strcasecmp(value, "true") == 0);
    else if( strcasecmp(name, "visible") == 0 ) SetVisible(strcasecmp(value, "true") == 0);
    else if( strcasecmp(name, "float") == 0 ) {
        UIString nValue {value};
        if(nValue.Find(',') < 0) {
            SetFloat(strcasecmp(value, "true") == 0);
        }
        else {
            TPercentInfo piFloatPercent = { 0 };
            char *pstr = nullptr;
            piFloatPercent.left = strtod(value, &pstr);  assert(pstr);
            piFloatPercent.top = strtod(pstr + 1, &pstr);    assert(pstr);
            piFloatPercent.right = strtod(pstr + 1, &pstr);  assert(pstr);
            piFloatPercent.bottom = strtod(pstr + 1, &pstr); assert(pstr);
            SetFloatPercent(piFloatPercent);
            SetFloat(true);
        }
    }
    else if( strcasecmp(name, "shortcut") == 0 ) SetShortcut(value[0]);
    else if( strcasecmp(name, "menu") == 0 ) SetContextMenuUsed(strcasecmp(value, "true") == 0);
    else if( strcasecmp(name, "virtualwnd") == 0 ) SetVirtualWnd(value);
    else {
        AddCustomAttribute(name, value);
    }
}

UIString UIControl::GetAttributeList(bool bIgnoreDefault) {
    return UIString();
}

void UIControl::SetAttributeList(const char *pstrList) {
    UIString sItem;
    UIString sValue;
    while( *pstrList != '\0' ) {
        sItem.Empty();
        sValue.Empty();
        while( *pstrList != '\0' && *pstrList != '=' ) {
            const char *pstrTemp = CharNext(pstrList);
            while( pstrList < pstrTemp) {
                sItem += *pstrList++;
            }
        }
        assert( *pstrList == '=' );
        if( *pstrList++ != '=' ) return;
        assert( *pstrList == '\"' );
        if( *pstrList++ != '\"' ) return;
        while( *pstrList != '\0' && *pstrList != '\"' ) {
            const char *pstrTemp = CharNext(pstrList);
            while( pstrList < pstrTemp) {
                sValue += *pstrList++;
            }
        }
        assert( *pstrList == '\"' );
        if( *pstrList++ != '\"' ) return;
        SetAttribute(sItem.GetData(), sValue.GetData());
        if( *pstrList++ != ' ' ) return;
    }
}

SIZE UIControl::EstimateSize(SIZE szAvailable) {
    return m_cxyFixed;
}

bool UIControl::Paint(HANDLE_DC hDC, const RECT &rcPaint, UIControl *pStopControl) {
    if (pStopControl == this) return false;
    if( !UIIntersectRect(&m_rcPaint, &rcPaint, &m_rcItem) ) return true;
    if( OnPaint ) {
        if( !OnPaint(this) ) return true;
    }
    if (!DoPaint(hDC, rcPaint, pStopControl)) return false;
    if( m_cover != nullptr ) return m_cover->Paint(hDC, rcPaint);
    return true;
}

bool UIControl::DoPaint(HANDLE_DC hDC, const RECT &rcPaint, UIControl *pStopControl) {
    // 绘制循序：背景颜色->背景图->状态图->文本->边框
    if( m_cxyBorderRound.cx > 0 || m_cxyBorderRound.cy > 0 ) {
        UIRenderClip    roundClip;
        UIRenderClip::GenerateRoundClip(hDC, m_rcPaint, m_rcItem,m_cxyBorderRound.cx, m_cxyBorderRound.cy, roundClip);
        PaintBkColor(hDC);
        PaintBkImage(hDC);
        PaintStatusImage(hDC);
        PaintText(hDC);
        PaintBorder(hDC);
    }
    else {
        PaintBkColor(hDC);
        PaintBkImage(hDC);
        PaintStatusImage(hDC);
        PaintText(hDC);
        PaintBorder(hDC);
    }
    return true;
}

void UIControl::PaintBkColor(HANDLE_DC hDC) {
    if( m_backColor != 0 ) {
        if( m_backColor2 != 0 ) {
            if( m_backColor3 != 0 ) {
                RECT rc = m_rcItem;
                rc.bottom = (rc.bottom + rc.top) / 2;
                UIRenderEngine::DrawGradient(hDC, rc, GetAdjustColor(m_backColor), GetAdjustColor(m_backColor2), true, 8);
                rc.top = rc.bottom;
                rc.bottom = m_rcItem.bottom;
                UIRenderEngine::DrawGradient(hDC, rc, GetAdjustColor(m_backColor2), GetAdjustColor(m_backColor3), true, 8);
            }
            else
                UIRenderEngine::DrawGradient(hDC, m_rcItem, GetAdjustColor(m_backColor), GetAdjustColor(m_backColor2), true, 16);
        }
        else if( m_backColor >= 0xFF000000 ) UIRenderEngine::DrawColor(hDC, m_rcPaint, GetAdjustColor(m_backColor));
        else UIRenderEngine::DrawColor(hDC, m_rcItem, GetAdjustColor(m_backColor));
    }
}

void UIControl::PaintBkImage(HANDLE_DC hDC) {
    DrawImage(hDC, m_diBk);
}

void UIControl::PaintStatusImage(HANDLE_DC hDC) {

}

void UIControl::PaintText(HANDLE_DC hDC) {

}

void UIControl::PaintBorder(HANDLE_DC hDC) {
    if(m_rcBorderSize.left > 0 && (m_borderColor != 0 || m_focusBorderColor != 0)) {
        if( m_cxyBorderRound.cx > 0 || m_cxyBorderRound.cy > 0 )//画圆角边框
        {
            if (IsFocused() && m_focusBorderColor != 0)
                UIRenderEngine::DrawRoundRect(hDC, m_rcItem, m_cxyBorderRound.cx, m_cxyBorderRound.cy, m_rcBorderSize.left, GetAdjustColor(m_focusBorderColor), m_borderStyle);
            else
                UIRenderEngine::DrawRoundRect(hDC, m_rcItem,  m_cxyBorderRound.cx, m_cxyBorderRound.cy, m_rcBorderSize.left, GetAdjustColor(m_borderColor), m_borderStyle);
        }
        else {
            if (m_rcBorderSize.right == m_rcBorderSize.left && m_rcBorderSize.top == m_rcBorderSize.left && m_rcBorderSize.bottom == m_rcBorderSize.left) {
                if (IsFocused() && m_focusBorderColor != 0)
                    UIRenderEngine::DrawRect(hDC, m_rcItem, m_rcBorderSize.left, GetAdjustColor(m_focusBorderColor), m_borderStyle);
                else
                    UIRenderEngine::DrawRect(hDC, m_rcItem, m_rcBorderSize.left, GetAdjustColor(m_borderColor), m_borderStyle);
            }
            else {
                RECT rcBorder;
                if(m_rcBorderSize.left > 0){
                    rcBorder		= m_rcItem;
                    rcBorder.left  += m_rcBorderSize.left / 2;
                    rcBorder.right	= rcBorder.left;
                    if (IsFocused() && m_focusBorderColor != 0)
                        UIRenderEngine::DrawLine(hDC,rcBorder,m_rcBorderSize.left,GetAdjustColor(m_focusBorderColor),m_borderStyle);
                    else
                        UIRenderEngine::DrawLine(hDC,rcBorder,m_rcBorderSize.left,GetAdjustColor(m_borderColor),m_borderStyle);
                }
                if(m_rcBorderSize.top > 0) {
                    rcBorder		= m_rcItem;
                    rcBorder.top   += m_rcBorderSize.top / 2;
                    rcBorder.bottom	= rcBorder.top;
                    rcBorder.left  += m_rcBorderSize.left;
                    rcBorder.right -= m_rcBorderSize.right;
                    if (IsFocused() && m_focusBorderColor != 0)
                        UIRenderEngine::DrawLine(hDC,rcBorder,m_rcBorderSize.top,GetAdjustColor(m_focusBorderColor),m_borderStyle);
                    else
                        UIRenderEngine::DrawLine(hDC,rcBorder,m_rcBorderSize.top,GetAdjustColor(m_borderColor),m_borderStyle);
                }
                if(m_rcBorderSize.right > 0) {
                    rcBorder		= m_rcItem;
                    rcBorder.left	= m_rcItem.right - m_rcBorderSize.right / 2;
                    rcBorder.right  = rcBorder.left;
                    if (IsFocused() && m_focusBorderColor != 0)
                        UIRenderEngine::DrawLine(hDC,rcBorder,m_rcBorderSize.right,GetAdjustColor(m_focusBorderColor),m_borderStyle);
                    else
                        UIRenderEngine::DrawLine(hDC,rcBorder,m_rcBorderSize.right,GetAdjustColor(m_borderColor),m_borderStyle);
                }
                if(m_rcBorderSize.bottom > 0) {
                    rcBorder		= m_rcItem;
                    rcBorder.top	= m_rcItem.bottom - m_rcBorderSize.bottom / 2;
                    rcBorder.bottom = rcBorder.top;
                    rcBorder.left  += m_rcBorderSize.left;
                    rcBorder.right -= m_rcBorderSize.right;
                    if (IsFocused() && m_focusBorderColor != 0)
                        UIRenderEngine::DrawLine(hDC,rcBorder,m_rcBorderSize.bottom,GetAdjustColor(m_focusBorderColor),m_borderStyle);
                    else
                        UIRenderEngine::DrawLine(hDC,rcBorder,m_rcBorderSize.bottom,GetAdjustColor(m_borderColor),m_borderStyle);
                }
            }
        }
    }
}

void UIControl::DoPostPaint(HANDLE_DC hDC, const RECT &rcPaint) {
    if( OnPostPaint ) OnPostPaint(this);
}

void UIControl::SetVirtualWnd(const char *pstrValue) {
    m_virtualWnd = pstrValue;
    m_manager->UsedVirtualWnd(true);
}

UIString UIControl::GetVirtualWnd() const {
    UIString str;
    if(!m_virtualWnd.IsEmpty()){
        str = m_virtualWnd;
    }else{
        UIControl *parent = GetParent();
        if(parent != nullptr){
            str = parent->GetVirtualWnd();
        }else{
            str = UIString{""};
        }
    }
    return str;
}
