#include <UILabel.h>
#include <UIPaintManager.h>
#include <cassert>
#include <cstring>
#include "UIResourceMgr.h"
#include "UIRenderEngine.h"

UILabel::UILabel()
    :m_textColor{0},
    m_disabledTextColor{0},
    m_fontId{-1},
    m_textStyle{DT_LEFT|DT_VCENTER|DT_SINGLELINE},
    m_rcTextPadding{0,0,0,0},
    m_showHtml{false}
{

}

UIString UILabel::GetClass() const {
    return UIString{DUI_CTR_LABEL};
}

LPVOID UILabel::GetInterface(const UIString &name) {
    if(name==DUI_CTR_LABEL){
        return static_cast<UILabel*>(this);
    }
    return UIControl::GetInterface(name);
}

void UILabel::SetTextStyle(uint32_t style) {
    m_textStyle = style;
    Invalidate();
}

uint32_t UILabel::GetTextStyle() const {
    return m_textStyle;
}

void UILabel::SetTextColor(uint32_t textColor) {
    m_textColor = textColor;
    Invalidate();
}

uint32_t UILabel::GetTextColor() const {
    return m_textColor;
}

void UILabel::SetDisabledTextColor(uint32_t disabledTextColor) {
    m_disabledTextColor = disabledTextColor;
    Invalidate();
}

uint32_t UILabel::GetDisabledTextColor() const {
    return m_disabledTextColor;
}

void UILabel::SetFont(int fontId) {
    m_fontId = fontId;
    Invalidate();
}

int UILabel::GetFont() const {
    return m_fontId;
}

RECT UILabel::GetTextPadding() const {
    return m_rcTextPadding;
}

void UILabel::SetTextPadding(const RECT &rc) {
    m_rcTextPadding = rc;
    Invalidate();
}

bool UILabel::IsShowHtml() const {
    return m_showHtml;
}

void UILabel::SetShowHtml(bool showHtml) {
    if(m_showHtml == showHtml){
        return;
    }
    m_showHtml = showHtml;
    Invalidate();
}

SIZE UILabel::EstimateSize(SIZE szAvailable) {
    // TODO 完成计算大小
    if(m_cxyFixed.cy == 0) {
        return SIZE{m_cxyFixed.cx, static_cast<long>(UIResourceMgr::GetInstance().GetFontHeight(m_fontId,m_manager->GetPaintDC()) + 4)};
    }
    return UIControl::EstimateSize(szAvailable);
}

void UILabel::DoEvent(TEventUI &event) {
    if(event.Type == UIEVENT_SETFOCUS){
        m_focused = true;
        return;
    }
    if(event.Type == UIEVENT_KILLFOCUS){
        m_focused = false;
        return;
    }
    if(event.Type == UIEVENT_MOUSEENTER){
        // return;
    }
    if(event.Type == UIEVENT_MOUSELEAVE){
        // return;
    }
    UIControl::DoEvent(event);
}

void UILabel::SetAttribute(const char *pstrName, const char *pstrValue) {
    if( strcasecmp(pstrName, "align") == 0 ) {
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
    else if( strcasecmp(pstrName, "font") == 0 ) SetFont(atoi(pstrValue));
    else if( strcasecmp(pstrName, "textcolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetTextColor(clrColor);
    }
    else if( strcasecmp(pstrName, "disabledtextcolor") == 0 ) {
        if( *pstrValue == '#') pstrValue = CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetDisabledTextColor(clrColor);
    }
    else if( strcasecmp(pstrName, "textpadding") == 0 ) {
        RECT rcTextPadding = { 0 };
        char* pstr = nullptr;
        rcTextPadding.left = strtol(pstrValue, &pstr, 10);  assert(pstr);
        rcTextPadding.top = strtol(pstr + 1, &pstr, 10);    assert(pstr);
        rcTextPadding.right = strtol(pstr + 1, &pstr, 10);  assert(pstr);
        rcTextPadding.bottom = strtol(pstr + 1, &pstr, 10); assert(pstr);
        SetTextPadding(rcTextPadding);
    }
    //else if( strcasecmp(pstrName, "multiline") == 0 ) SetMultiLine(strcasecmp(pstrValue, "true") == 0);
    else if( strcasecmp(pstrName, "showhtml") == 0 ) SetShowHtml(strcasecmp(pstrValue, "true") == 0);
    else UIControl::SetAttribute(pstrName, pstrValue);
}

void UILabel::PaintText(HANDLE_DC hDC) {
    if( m_textColor == 0 ) m_textColor = m_manager->GetDefaultFontColor();
    if( m_disabledTextColor == 0 ) m_disabledTextColor = m_manager->GetDefaultDisabledColor();

    RECT rc = m_rcItem;
    rc.left += m_rcTextPadding.left;
    rc.right -= m_rcTextPadding.right;
    rc.top += m_rcTextPadding.top;
    rc.bottom -= m_rcTextPadding.bottom;

    if( m_text.IsEmpty() ) return;
    int nLinks = 0;
    if( IsEnabled() ) {
        if( m_showHtml )
            UIRenderEngine::DrawHtmlText(hDC, m_manager, rc, m_text, m_textColor, \
					nullptr, nullptr, nLinks, m_fontId, m_textStyle);
        else
            UIRenderEngine::DrawText(hDC, m_manager, rc, m_text, m_textColor, \
					m_fontId, m_textStyle);
    }
    else {
        if( m_showHtml )
            UIRenderEngine::DrawHtmlText(hDC, m_manager, rc, m_text, m_disabledTextColor, \
					nullptr, nullptr, nLinks, m_fontId, m_textStyle);
        else
            UIRenderEngine::DrawText(hDC, m_manager, rc, m_text, m_disabledTextColor, \
					m_fontId, m_textStyle);
    }

}
