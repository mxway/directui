#include <UIOption.h>
#include <UIPaintManager.h>
#include "UIFont.h"
#include "UIResourceMgr.h"
#include "UIRenderEngine.h"

UIOption::UIOption()
    :m_selected{false},
    m_selectedBkColor{0},
    m_selectedTextColor{0}
{

}

UIOption::~UIOption() {
    if((!m_groupName.IsEmpty()) && m_manager){
        m_manager->RemoveOptionGroup(m_groupName,this);
    }
}

UIString UIOption::GetClass() const {
    return UIString{DUI_CTR_OPTION};
}

LPVOID UIOption::GetInterface(const UIString &name) {
    if(name == DUI_CTR_OPTION){
        return static_cast<UIOption*>(this);
    }
    return UIButton::GetInterface(name);
}

bool UIOption::Activate() {
    if( !UIButton::Activate() ) return false;
    if( !m_groupName.IsEmpty() ) Selected(true);
    else Selected(!m_selected);

    return true;
}

void UIOption::SetEnabled(bool bEnabled) {
    UIButton::SetEnabled(bEnabled);
    if( !IsEnabled() ) {
        if( m_selected ) m_buttonState = UISTATE_SELECTED;
        else m_buttonState = 0;
    }
}

void UIOption::SetManager(UIPaintManager *manager, UIControl *parent, bool bInit) {
    UIControl::SetManager(manager, parent, bInit);
    if(bInit && (!m_groupName.IsEmpty())){
        if(m_manager){
            m_manager->AddOptionGroup(m_groupName, this);
        }
    }
}

UIString UIOption::GetSelectedImage() const {
    return m_diSelected.sDrawString;
}

void UIOption::SetSelectedImage(const UIString &selectedImage) {
    if(m_diSelected.sDrawString == selectedImage && m_diSelected.pImageInfo != nullptr){
        return;
    }
    m_diSelected.Clear();
    m_diSelected.sDrawString = selectedImage;
    Invalidate();
}

UIString UIOption::GetSelectedHotImage() const {
    return m_diSelectedHot.sDrawString;
}

void UIOption::SetSelectedHotImage(const UIString &selectedHotImage) {
    if(m_diSelectedHot.sDrawString == selectedHotImage && m_diSelectedHot.pImageInfo != nullptr){
        return;
    }
    m_diSelectedHot.Clear();
    m_diSelectedHot.sDrawString = selectedHotImage;
    Invalidate();
}

uint32_t UIOption::GetSelectedTextColor() {
    if (m_selectedTextColor == 0) m_selectedTextColor = m_manager->GetDefaultFontColor();
    return m_selectedTextColor;
}

void UIOption::SetSelectedTextColor(uint32_t selectedTextColor) {
    if(m_selectedTextColor == selectedTextColor){
        return;
    }
    m_selectedTextColor = selectedTextColor;
    Invalidate();
}

uint32_t UIOption::GetSelectedBkColor() const {
    return m_selectedBkColor;
}

void UIOption::SetSelectedBkColor(uint32_t bkcolor) {
    if(m_selectedBkColor == bkcolor){
        return;
    }
    m_selectedBkColor = bkcolor;
    Invalidate();
}

UIString UIOption::GetForeImage() const {
    return m_diFore.sDrawString;
}

void UIOption::SetForeImage(const UIString &foreImage) {
    if( m_diFore.sDrawString == foreImage && m_diFore.pImageInfo != NULL ) return;
    m_diFore.Clear();
    m_diFore.sDrawString = foreImage;
    Invalidate();
}

UIString UIOption::GetGroup() const {
    return m_groupName;
}

void UIOption::SetGroup(const UIString &groupName) {
    if( groupName.IsEmpty() ) {
        if( m_groupName.IsEmpty() ) return;
        m_groupName.Empty();
    }
    else {
        if( m_groupName == groupName ) return;
        if (!m_groupName.IsEmpty() && m_manager) m_manager->RemoveOptionGroup(m_groupName, this);
        m_groupName = groupName;
    }

    if( !m_groupName.IsEmpty() ) {
        if (m_manager) m_manager->AddOptionGroup(m_groupName, this);
    }
    else {
        if (m_manager) m_manager->RemoveOptionGroup(m_groupName, this);
    }

    Selected(m_selected);
}

bool UIOption::IsSelected() const {
    return m_selected;
}

void UIOption::Selected(bool selected, bool triggerEvent) {
    if( m_selected == selected ) return;
    m_selected = selected;
    if( m_selected ) m_buttonState |= UISTATE_SELECTED;
    else m_buttonState &= ~UISTATE_SELECTED;

    if( m_manager != nullptr ) {
        if( !m_groupName.IsEmpty() ) {
            if( m_selected ) {
                UIPtrArray* aOptionGroup = m_manager->GetOptionGroup(m_groupName);
                for( int i = 0; i < aOptionGroup->GetSize(); i++ ) {
                    auto* pControl = static_cast<UIOption*>(aOptionGroup->GetAt(i));
                    if( pControl != this ) {
                        pControl->Selected(false, triggerEvent);
                    }
                }
                if (triggerEvent) m_manager->SendNotify(this, DUI_MSGTYPE_SELECTCHANGED);
            }
        }
        else {
            if (triggerEvent) m_manager->SendNotify(this, DUI_MSGTYPE_SELECTCHANGED);
        }
    }

    Invalidate();
}

SIZE UIOption::EstimateSize(SIZE szAvailable) {
    UIFont *font = UIResourceMgr::GetInstance().GetFont(GetFont());
    uint32_t cy = font->GetFontHeight(m_manager->GetPaintDC()) + 8;
    if( m_cxyFixed.cy == 0 ) return SIZE{m_cxyFixed.cx, (long)cy};
    return UIButton::EstimateSize(szAvailable);
}

void UIOption::SetAttribute(const char *pstrName, const char *pstrValue) {
    if( strcasecmp(pstrName, "group") == 0 ) SetGroup(UIString{pstrValue});
    else if( strcasecmp(pstrName, "selected") == 0 ) Selected(strcasecmp(pstrValue, "true") == 0);
    else if( strcasecmp(pstrName, "selectedimage") == 0 ) SetSelectedImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "selectedhotimage") == 0 ) SetSelectedHotImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "foreimage") == 0 ) SetForeImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "selectedbkcolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetSelectedBkColor(clrColor);
    }
    else if( strcasecmp(pstrName, "selectedtextcolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetSelectedTextColor(clrColor);
    }
    else UIButton::SetAttribute(pstrName, pstrValue);
}

void UIOption::PaintStatusImage(HANDLE_DC hDC) {
    if( (m_buttonState & UISTATE_SELECTED) != 0 ) {
        if ((m_buttonState & UISTATE_HOT) != 0)
        {
            if (DrawImage(hDC, m_diSelectedHot)) {
                DrawImage(hDC, m_diFore);
                return;
            }
        }

        if( DrawImage(hDC, m_diSelected) ) {
            DrawImage(hDC, m_diFore);
            return;
        }
        else if(m_selectedBkColor != 0) {
            UIRenderEngine::DrawColor(hDC, m_rcPaint, GetAdjustColor(m_selectedBkColor));
            DrawImage(hDC, m_diFore);
            return ;
        }
    }

    uint32_t uSavedState = m_buttonState;
    m_buttonState &= ~UISTATE_PUSHED;
    UIButton::PaintStatusImage(hDC);
    m_buttonState = uSavedState;

    DrawImage(hDC, m_diFore);
}

void UIOption::PaintText(HANDLE_DC hDC) {
    if( (m_buttonState & UISTATE_SELECTED) != 0 )
    {
        uint32_t oldTextColor = m_textColor;
        if( m_selectedTextColor != 0 ) m_textColor = m_selectedTextColor;

        if( m_textColor == 0 ) m_textColor = m_manager->GetDefaultFontColor();
        if( m_disabledTextColor == 0 ) m_disabledTextColor = m_manager->GetDefaultDisabledColor();

        if( m_text.IsEmpty() ) return;
        int nLinks = 0;
        RECT rc = m_rcItem;
        rc.left += m_rcTextPadding.left;
        rc.right -= m_rcTextPadding.right;
        rc.top += m_rcTextPadding.top;
        rc.bottom -= m_rcTextPadding.bottom;

        if( m_showHtml )
            UIRenderEngine::DrawHtmlText(hDC, m_manager, rc, m_text, IsEnabled()?m_textColor:m_disabledTextColor, \
				nullptr, nullptr, nLinks, m_fontId, m_textStyle);
        else
            UIRenderEngine::DrawText(hDC, m_manager, rc, m_text, IsEnabled()?m_textColor:m_disabledTextColor, \
				m_fontId, m_textStyle);

        m_textColor = oldTextColor;
    }
    else
    {
        uint32_t uSavedState = m_buttonState;
        m_buttonState &= ~UISTATE_PUSHED;
        UIButton::PaintText(hDC);
        m_buttonState = uSavedState;
    }
}
