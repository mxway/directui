#include <UIEdit.h>
#include <UIPaintManager.h>
#include <UIRenderEngine.h>
#include <iostream>

using namespace std;

UIEdit::UIEdit()
    :m_maxChar{255},
    m_readOnly{false},
    m_numberOnly{false},
    m_passwordMode{false},
    m_passwordChar{'*'},
    m_autoSelAll{false},
    m_buttonState{0},
    m_editBkColor{0xFFFFFFFF}
{
    InitInternal();
    SetTextPadding(RECT{4,0, 4, 0});
    SetBkColor(0xFFFFFFFF);
}

uint32_t UIEdit::GetControlFlags() const {
    if(!IsEnabled()){
        return UILabel::GetControlFlags();
    }
    return UIFLAG_SETCURSOR|UIFLAG_TABSTOP;
}

UIString UIEdit::GetClass() const {
    return UIString{DUI_CTR_EDIT};
}

LPVOID UIEdit::GetInterface(const UIString &name) {
    if(name == DUI_CTR_EDIT){
        return static_cast<UIEdit*>(this);
    }
    return UILabel::GetInterface(name);
}

void UIEdit::SetEnabled(bool enabled) {
    UIControl::SetEnabled(enabled);
    if(!IsEnabled()){
        m_buttonState = 0;
    }
}

void UIEdit::SetMaxChar(uint32_t maxChar) {
    m_maxChar = maxChar;
}

uint32_t UIEdit::GetMaxChar() const {
    return m_maxChar;
}

void UIEdit::SetReadOnly(bool readOnly) {
    if(m_readOnly == readOnly){
        return;
    }
    m_readOnly = readOnly;
    Invalidate();
}

bool UIEdit::IsReadOnly() const {
    return m_readOnly;
}

void UIEdit::SetPasswordMode(bool passwordMode) {
    if(m_passwordMode == passwordMode){
        return;
    }
    m_passwordMode = passwordMode;
    Invalidate();
}

bool UIEdit::IsPasswordMode() const {
    return m_passwordMode;
}

void UIEdit::SetPasswordChar(char passwordChar) {
    if(m_passwordChar == passwordChar){
        return;
    }
    m_passwordChar = passwordChar;
    Invalidate();
}

char UIEdit::GetPasswordChar() const {
    return m_passwordChar;
}

bool UIEdit::IsAutoSelAll() const {
    return m_autoSelAll;
}

void UIEdit::SetAutoSelAll(bool autoSelAll) {
    m_autoSelAll = autoSelAll;
}

void UIEdit::SetNumberOnly(bool numberOnly) {
    m_numberOnly = numberOnly;
}

bool UIEdit::IsNumberOnly() const {
    return m_numberOnly;
}

UIString UIEdit::GetNormalImage() const {
    return m_diNormal.sDrawString;
}

void UIEdit::SetNormalImage(const UIString &normalImage) {
    if( m_diNormal.sDrawString == normalImage && m_diNormal.pImageInfo != nullptr ) return;
    m_diNormal.Clear();
    m_diNormal.sDrawString = normalImage;
    Invalidate();
}

UIString UIEdit::GetHotImage() const {
    return m_diHot.sDrawString;
}

void UIEdit::SetHotImage(const UIString &hotImage) {
    if( m_diHot.sDrawString == hotImage && m_diHot.pImageInfo != nullptr ) return;
    m_diHot.Clear();
    m_diHot.sDrawString = hotImage;
    Invalidate();
}

UIString UIEdit::GetFocusedImage() const {
    return m_diFocused.sDrawString;
}

void UIEdit::SetFocusedImage(const UIString &focusedImage) {
    if( m_diFocused.sDrawString == focusedImage && m_diFocused.pImageInfo != nullptr ) return;
    m_diFocused.Clear();
    m_diFocused.sDrawString = focusedImage;
    Invalidate();
}

UIString UIEdit::GetDisabledImage() const {
    return m_diDisabled.sDrawString;
}

void UIEdit::SetDisabledImage(const UIString &disabledImage) {
    if( m_diDisabled.sDrawString == disabledImage && m_diDisabled.pImageInfo != nullptr ) return;
    m_diDisabled.Clear();
    m_diDisabled.sDrawString = disabledImage;
    Invalidate();
}

void UIEdit::SetSel(long startChar, long endChar) {
    //TODO SetSel?
}

void UIEdit::SetSelAll() {
    //TODO SetSelAll
}

void UIEdit::SetReplaceSel(const UIString &replace) {
    //TODO SetReplaceSel ??
}

void UIEdit::SetVisible(bool visible) {
    UIControl::SetVisible(visible);
    if(!IsVisible()){
        m_manager->SetFocus(nullptr);
    }
}

void UIEdit::SetInternVisible(bool visible) {
    if(!IsVisible()){
        m_manager->SetFocus(nullptr);
    }
}

void UIEdit::SetAttribute(const char *pstrName, const char *pstrValue) {
    if( strcasecmp(pstrName, "readonly") == 0 ) SetReadOnly(strcasecmp(pstrValue, "true") == 0);
    else if( strcasecmp(pstrName, "numberonly") == 0 ) SetNumberOnly(strcasecmp(pstrValue, "true") == 0);
    else if( strcasecmp(pstrName, "password") == 0 ) SetPasswordMode(strcasecmp(pstrValue, "true") == 0);
    else if( strcasecmp(pstrName, "autoselall") == 0 ) SetAutoSelAll(strcasecmp(pstrValue, "true") == 0);
    else if( strcasecmp(pstrName, "maxchar") == 0 ) SetMaxChar(atoi(pstrValue));
    else if( strcasecmp(pstrName, "normalimage") == 0 ) SetNormalImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "hotimage") == 0 ) SetHotImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "focusedimage") == 0 ) SetFocusedImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "disabledimage") == 0 ) SetDisabledImage(UIString{pstrValue});
    else UILabel::SetAttribute(pstrName, pstrValue);
}

void UIEdit::PaintStatusImage(HANDLE_DC hDC) {
    if( IsFocused() ) m_buttonState |= UISTATE_FOCUSED;
    else m_buttonState &= ~ UISTATE_FOCUSED;
    if( !IsEnabled() ) m_buttonState |= UISTATE_DISABLED;
    else m_buttonState &= ~ UISTATE_DISABLED;

    if( (m_buttonState & UISTATE_DISABLED) != 0 ) {
        if( DrawImage(hDC, m_diDisabled) ) return;
    }
    else if( (m_buttonState & UISTATE_FOCUSED) != 0 ) {
        if( DrawImage(hDC, m_diFocused) ) return;
    }
    else if( (m_buttonState & UISTATE_HOT) != 0 ) {
        if( DrawImage(hDC, m_diHot) ) return;
    }

    if( DrawImage(hDC, m_diNormal) ) return;
}

void UIEdit::PaintText(HANDLE_DC hDC) {
    if( m_textColor == 0 ) m_textColor = m_manager->GetDefaultFontColor();
    if( m_disabledTextColor == 0 ) m_disabledTextColor = m_manager->GetDefaultDisabledColor();
    this->DrawCaret(hDC);
    if( m_text.IsEmpty() ) return;

    //CDuiString sText = m_sText;
    UIString  sText{m_text};
    if( m_passwordMode ) {
        sText.Empty();
        uint32_t numberOfChar = this->GetCharNumber();
        while(numberOfChar--){
           sText += m_passwordChar;
        }
    }

    RECT rc = m_rcItem;
    rc.left += m_rcTextPadding.left;
    rc.right -= m_rcTextPadding.right;
    rc.top += m_rcTextPadding.top;
    rc.bottom -= m_rcTextPadding.bottom;
    if( IsEnabled() ) {
        UIRenderEngine::DrawText(hDC, m_manager, rc, sText, m_textColor, \
				m_fontId, DT_SINGLELINE | m_textStyle);
    }
    else {
        UIRenderEngine::DrawText(hDC, m_manager, rc, sText, m_disabledTextColor, \
				m_fontId, DT_SINGLELINE | m_textStyle);

    }
}