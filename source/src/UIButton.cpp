#include <UIButton.h>
#include <UIPaintManager.h>
#include <UIResourceMgr.h>
#include "UIRect.h"
#include "UIRenderEngine.h"

UIButton::UIButton()
    :m_buttonState{0},
    m_hotTextColor{0},
    m_pushedTextColor{0},
    m_focusedTextColor{0},
    m_hotBkColor{0},
    m_fadeAlpha {255},
    m_fadeAlphaDelta{0}
{
    m_textStyle = DT_SINGLELINE | DT_VCENTER|DT_CENTER;
}

UIString UIButton::GetClass() const {
    return UIString{DUI_CTR_BUTTON};
}

LPVOID UIButton::GetInterface(const UIString &name) {
    if(name == DUI_CTR_BUTTON){
        return static_cast<UIButton*>(this);
    }
    return UILabel::GetInterface(name);
}

uint32_t UIButton::GetControlFlags() const {
    return (IsKeyboardEnabled() ? UIFLAG_TABSTOP : 0) | (IsEnabled() ? UIFLAG_SETCURSOR : 0);
}

bool UIButton::Activate() {
    if( !UIControl::Activate() ) return false;
    if( m_manager != nullptr ) m_manager->SendNotify(this, DUI_MSGTYPE_CLICK);
    return true;
}

void UIButton::SetEnabled(bool bEnabled) {
    UIControl::SetEnabled(bEnabled);
    if( !IsEnabled() ) {
        m_buttonState = 0;
    }
}

void UIButton::DoEvent(TEventUI &event) {
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_parent != NULL ) m_parent->DoEvent(event);
        else UILabel::DoEvent(event);
        return;
    }
    UIRect rcItem{m_rcItem};

    if( event.Type == UIEVENT_SETFOCUS )
    {
        Invalidate();
    }
    if( event.Type == UIEVENT_KILLFOCUS )
    {
        Invalidate();
    }
    if( event.Type == UIEVENT_KEYDOWN )
    {
        if (IsKeyboardEnabled() && IsEnabled()) {
            if( event.chKey == VK_SPACE || event.chKey == VK_RETURN ) {
                Activate();
                return;
            }
        }
    }
    if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK )
    {
        if( rcItem.IsPtIn(event.ptMouse) && IsEnabled() ) {
            m_buttonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_MOUSEMOVE )
    {
        if( (m_buttonState & UISTATE_CAPTURED) != 0 ) {
            if( rcItem.IsPtIn(event.ptMouse) ) m_buttonState |= UISTATE_PUSHED;
            else m_buttonState &= ~UISTATE_PUSHED;
            Invalidate();
        }
        return;
    }

    if( event.Type == UIEVENT_BUTTONUP )
    {
        if( (m_buttonState & UISTATE_CAPTURED) != 0 ) {
            if( rcItem.IsPtIn(event.ptMouse) && IsEnabled()) Activate();
            m_buttonState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_CONTEXTMENU )
    {
        if( IsContextMenuUsed() && IsEnabled()) {
            m_manager->SendNotify(this, DUI_MSGTYPE_MENU, event.wParam, event.lParam);
        }
        return;
    }

    if( event.Type == UIEVENT_MOUSEENTER )
    {
        if( rcItem.IsPtIn(event.ptMouse ) ) {
            if( IsEnabled() ) {
                if( (m_buttonState & UISTATE_HOT) == 0  ) {
                    m_buttonState |= UISTATE_HOT;
                    Invalidate();
                }
            }
        }
        if ( GetFadeAlphaDelta() > 0 ) {
            //m_manager->SetTimer(this, FADE_TIMERID, FADE_ELLAPSE);
        }
    }
    if( event.Type == UIEVENT_MOUSELEAVE )
    {
        if( !rcItem.IsPtIn(event.ptMouse ) ) {
            if( IsEnabled() ) {
                if( (m_buttonState & UISTATE_HOT) != 0  ) {
                    m_buttonState &= ~UISTATE_HOT;
                    Invalidate();
                }
            }
            if (m_manager) m_manager->RemoveMouseLeaveNeeded(this);
            if ( GetFadeAlphaDelta() > 0 ) {
                //m_manager->SetTimer(this, FADE_TIMERID, FADE_ELLAPSE);
            }
        }
        else {
            if (m_manager) m_manager->AddMouseLeaveNeeded(this);
            return;
        }
    }
    if( event.Type == UIEVENT_SETCURSOR )
    {
        UILoadCursor(m_manager, UI_IDC_HAND);
        return;
    }
    if( event.Type == UIEVENT_TIMER  && (long )event.wParam == FADE_TIMERID )
    {
        if( (m_buttonState & UISTATE_HOT) != 0 ) {
            if( m_fadeAlpha > m_fadeAlphaDelta ) m_fadeAlpha -= m_fadeAlphaDelta;
            else {
                m_fadeAlpha = 0;
                m_manager->KillTimer(this, FADE_TIMERID);
            }
        }
        else {
            if( m_fadeAlpha < 255-m_fadeAlphaDelta ) m_fadeAlpha += m_fadeAlphaDelta;
            else {
                m_fadeAlpha = 255;
                m_manager->KillTimer(this, FADE_TIMERID);
            }
        }
        Invalidate();
        return;
    }
    UILabel::DoEvent(event);
}

UIString UIButton::GetNormalImage() const {
    return m_diNormal.sDrawString;
}

void UIButton::SetNormalImage(const UIString &normalImage) {
    if(m_diNormal.sDrawString==normalImage && m_diNormal.pImageInfo != nullptr){
        return;
    }
    m_diNormal.Clear();
    m_diNormal.sDrawString = normalImage;
    Invalidate();
}

UIString UIButton::GetHotImage() const {
    return m_diHot.sDrawString;
}

void UIButton::SetHotImage(const UIString &hotImage) {
    if(m_diHot.sDrawString==hotImage && m_diHot.pImageInfo != nullptr){
        return;
    }
    m_diHot.Clear();
    m_diHot.sDrawString = hotImage;
    Invalidate();
}

UIString UIButton::GetPushedImage() const {
    return m_diPushed.sDrawString;
}

void UIButton::SetPushedImage(const UIString &pushedImage) {
    if(m_diPushed.sDrawString == pushedImage && m_diPushed.pImageInfo != nullptr){
        return;
    }
    m_diPushed.Clear();
    m_diPushed.sDrawString = pushedImage;
    Invalidate();
}

UIString UIButton::GetFocusedImage() const {
    return m_diFocused.sDrawString;
}

void UIButton::SetFocusedImage(const UIString &focusedImage) {
    if(m_diFocused.sDrawString == focusedImage && m_diFocused.pImageInfo != nullptr)
    {
        return;
    }
    m_diFocused.Clear();
    m_diFocused.sDrawString = focusedImage;
    Invalidate();
}

UIString UIButton::GetDisabledImage() const {
    return m_diDisabled.sDrawString;
}

void UIButton::SetDisabledImage(const UIString &disabledImage) {
    if(m_diDisabled.sDrawString == disabledImage && m_diDisabled.pImageInfo!=nullptr)
    {
        return;
    }
    m_diDisabled.Clear();
    m_diDisabled.sDrawString = disabledImage;
    Invalidate();
}

UIString UIButton::GetForeImage() const {
    return m_diFore.sDrawString;
}

void UIButton::SetForeImage(const UIString &foreImage) {
    if(m_diFore.sDrawString==foreImage && m_diFore.pImageInfo!=nullptr){
        return;
    }

    m_diFore.Clear();
    m_diFore.sDrawString = foreImage;
    Invalidate();
}

UIString UIButton::GetHotForeImage() const {
    return m_diHotFore.sDrawString;
}

void UIButton::SetHotForeImage(const UIString &hotForeImage) {
    if(m_diHotFore.sDrawString == hotForeImage && m_diHotFore.pImageInfo != nullptr){
        return;
    }
    m_diHotFore.Clear();
    m_diHotFore.sDrawString = hotForeImage;
    Invalidate();
}

void UIButton::SetFiveStatusImage(const UIString &fiveStatusImage) {
    m_diNormal.Clear();
    m_diNormal.sDrawString = fiveStatusImage;
    DrawImage(nullptr, m_diNormal);
    if(m_diNormal.pImageInfo){
        long width = m_diNormal.pImageInfo->nX/5;
        long height = m_diNormal.pImageInfo->nY;
        m_diNormal.rcBmpPart = UIRect(0,0, width, height);
        if(m_float && m_cxyFixed.cx==0 && m_cxyFixed.cy == 0){
            m_cxyFixed.cx = width;
            m_cxyFixed.cy = height;
        }
    }

    m_diPushed.Clear();
    m_diPushed.sDrawString = fiveStatusImage;
    DrawImage(NULL, m_diPushed);
    if (m_diPushed.pImageInfo) {
        long width = m_diPushed.pImageInfo->nX / 5;
        long height = m_diPushed.pImageInfo->nY;
        m_diPushed.rcBmpPart = UIRect(width, 0, width*2, height);
    }

    m_diHot.Clear();
    m_diHot.sDrawString = fiveStatusImage;
    DrawImage(nullptr, m_diHot);
    if (m_diHot.pImageInfo) {
        long width = m_diHot.pImageInfo->nX / 5;
        long height = m_diHot.pImageInfo->nY;
        m_diHot.rcBmpPart = UIRect(width*2, 0, width*3, height);
    }

    m_diFocused.Clear();
    m_diFocused.sDrawString = fiveStatusImage;
    DrawImage(nullptr, m_diFocused);
    if (m_diFocused.pImageInfo) {
        long width = m_diFocused.pImageInfo->nX / 5;
        long height = m_diFocused.pImageInfo->nY;
        m_diFocused.rcBmpPart = UIRect(width*3, 0, width*4, height);
    }

    m_diDisabled.Clear();
    m_diDisabled.sDrawString = fiveStatusImage;
    DrawImage(nullptr, m_diDisabled);
    if (m_diDisabled.pImageInfo) {
        long width = m_diDisabled.pImageInfo->nX / 5;
        long height = m_diDisabled.pImageInfo->nY;
        m_diDisabled.rcBmpPart = UIRect(width*4, 0, width*5, height);
    }

    Invalidate();
}

void UIButton::SetFadeAlphaDelta(uint8_t delta) {
    m_fadeAlphaDelta = delta;
}

uint8_t UIButton::GetFadeAlphaDelta() const {
    return m_fadeAlphaDelta;
}

void UIButton::SetHotBkColor(uint32_t color) {
    if(m_hotBkColor == color){
        return;
    }
    m_hotBkColor = color;
    Invalidate();
}

uint32_t UIButton::GetHotBkColor() const {
    return m_hotBkColor;
}

void UIButton::SetHotTextColor(uint32_t color) {
    if(m_hotTextColor == color){
        return;
    }
    m_hotTextColor = color;
    Invalidate();
}

uint32_t UIButton::GetHotTextColor() const {
    return m_hotTextColor;
}

void UIButton::SetPushedTextColor(uint32_t color) {
    if(m_pushedTextColor == color){
        return;
    }
    m_pushedTextColor = color;
    Invalidate();
}

uint32_t UIButton::GetPushedTextColor() const {
    return m_pushedTextColor;
}

void UIButton::SetFocusedTextColor(uint32_t color) {
    if(m_focusedTextColor == color){
        return;
    }
    m_focusedTextColor = color;
    Invalidate();
}

uint32_t UIButton::GetFocusedTextColor() const {
    return m_focusedTextColor;
}

SIZE UIButton::EstimateSize(SIZE szAvailable) {

    if( m_cxyFixed.cy == 0 ) return SIZE{m_cxyFixed.cx, static_cast<long>(UIResourceMgr::GetInstance().GetFontHeight(GetFont(),m_manager->GetPaintDC()) + 8)};
    return UILabel::EstimateSize(szAvailable);
}

void UIButton::SetAttribute(const char *pstrName, const char *pstrValue) {
    if( strcasecmp(pstrName, "normalimage") == 0 ) SetNormalImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "hotimage") == 0 ) SetHotImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "pushedimage") == 0 ) SetPushedImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "focusedimage") == 0 ) SetFocusedImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "disabledimage") == 0 ) SetDisabledImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "foreimage") == 0 ) SetForeImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "hotforeimage") == 0 ) SetHotForeImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "fivestatusimage") == 0 ) SetFiveStatusImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "fadedelta") == 0 ) SetFadeAlphaDelta((uint8_t)atoi(pstrValue));
    else if( strcasecmp(pstrName, "hotbkcolor") == 0 )
    {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetHotBkColor(clrColor);
    }
    else if( strcasecmp(pstrName, "hottextcolor") == 0 )
    {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetHotTextColor(clrColor);
    }
    else if( strcasecmp(pstrName, "pushedtextcolor") == 0 )
    {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetPushedTextColor(clrColor);
    }
    else if( strcasecmp(pstrName, "focusedtextcolor") == 0 )
    {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetFocusedTextColor(clrColor);
    }
    else UILabel::SetAttribute(pstrName, pstrValue);
}

void UIButton::PaintText(HANDLE_DC hDC) {
    if( IsFocused() ) m_buttonState |= UISTATE_FOCUSED;
    else m_buttonState &= ~ UISTATE_FOCUSED;
    if( !IsEnabled() ) m_buttonState |= UISTATE_DISABLED;
    else m_buttonState &= ~ UISTATE_DISABLED;

    if( m_textColor == 0 ) m_textColor = m_manager->GetDefaultFontColor();
    if( m_disabledTextColor == 0 ) m_disabledTextColor = m_manager->GetDefaultDisabledColor();

    if( m_text.IsEmpty() ) return;
    int nLinks = 0;
    RECT rc = m_rcItem;
    rc.left += m_rcTextPadding.left;
    rc.right -= m_rcTextPadding.right;
    rc.top += m_rcTextPadding.top;
    rc.bottom -= m_rcTextPadding.bottom;

    uint32_t clrColor = IsEnabled()?m_textColor:m_disabledTextColor;

    if( ((m_buttonState & UISTATE_PUSHED) != 0) && (GetPushedTextColor() != 0) )
        clrColor = GetPushedTextColor();
    else if( ((m_buttonState & UISTATE_HOT) != 0) && (GetHotTextColor() != 0) )
        clrColor = GetHotTextColor();
    else if( ((m_buttonState & UISTATE_FOCUSED) != 0) && (GetFocusedTextColor() != 0) )
        clrColor = GetFocusedTextColor();

    if( m_showHtml )
        UIRenderEngine::DrawHtmlText(hDC, m_manager, rc, m_text, clrColor, \
			nullptr, nullptr, nLinks, m_fontId, m_textStyle);
    else
        UIRenderEngine::DrawText(hDC, m_manager, rc, m_text, clrColor, \
			m_fontId, m_textStyle);
}

void UIButton::PaintStatusImage(HANDLE_DC hDC) {
    if( IsFocused() ) m_buttonState |= UISTATE_FOCUSED;
    else m_buttonState &= ~ UISTATE_FOCUSED;
    if( !IsEnabled() ) m_buttonState |= UISTATE_DISABLED;
    else m_buttonState &= ~ UISTATE_DISABLED;

    if( (m_buttonState & UISTATE_DISABLED) != 0 ) {
        if (DrawImage(hDC, m_diDisabled)) goto Label_ForeImage;
    }
    else if( (m_buttonState & UISTATE_PUSHED) != 0 ) {
        if (!DrawImage(hDC, m_diPushed))
            DrawImage(hDC, m_diNormal);
        if (DrawImage(hDC, m_diPushedFore)) return;
        else goto Label_ForeImage;
    }
    else if( (m_buttonState & UISTATE_HOT) != 0 ) {
        if( GetFadeAlphaDelta() > 0 ) {
            if( m_fadeAlpha == 0 ) {
                m_diHot.uFade = 255;
                DrawImage(hDC, m_diHot);
            }
            else {
                m_diNormal.uFade = m_fadeAlpha;
                DrawImage(hDC, m_diNormal);
                m_diHot.uFade = 255 - m_fadeAlpha;
                DrawImage(hDC, m_diHot);
            }
        }
        else {
            if (!DrawImage(hDC, m_diHot))
                DrawImage(hDC, m_diNormal);
        }

        if (DrawImage(hDC, m_diHotFore)) return;
        else if(m_hotBkColor != 0) {
            UIRenderEngine::DrawColor(hDC, m_rcPaint, GetAdjustColor(m_hotBkColor));
            return;
        }
        else goto Label_ForeImage;
    }
    else if( (m_buttonState & UISTATE_FOCUSED) != 0 ) {
        if (DrawImage(hDC, m_diFocused)) goto Label_ForeImage;;
    }

    if ( GetFadeAlphaDelta() > 0 ) {
        if( m_fadeAlpha == 255 ) {
            m_diNormal.uFade = 255;
            DrawImage(hDC, m_diNormal);
        }
        else {
            m_diHot.uFade = 255 - m_fadeAlpha;
            DrawImage(hDC, m_diHot);
            m_diNormal.uFade = m_fadeAlpha;
            DrawImage(hDC, m_diNormal);
        }
    }
    else {
        DrawImage(hDC, m_diNormal);
    }

    Label_ForeImage:
    DrawImage(hDC, m_diFore);
}
