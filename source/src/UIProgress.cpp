#include <UIProgress.h>

UIProgress::UIProgress()
    :m_horizontal{true},
    m_min{0},
    m_max{100},
    m_value{0}
{
    m_textStyle = DT_SINGLELINE|DT_CENTER;
    UIControl::SetFixedHeight(12);
}

UIString UIProgress::GetClass() const {
    return UIString{DUI_CTR_PROGRESS};
}

LPVOID UIProgress::GetInterface(const UIString &name) {
    if(name == DUI_CTR_PROGRESS){
        return static_cast<UIProgress*>(this);
    }
    return UILabel::GetInterface(name);
}

bool UIProgress::IsHorizontal() const {
    return m_horizontal;
}

void UIProgress::SetHorizontal(bool horizontal) {
    if(m_horizontal == horizontal){
        return;
    }
    m_horizontal = horizontal;
    Invalidate();
}

int UIProgress::GetMinValue() const {
    return m_min;
}

void UIProgress::SetMinValue(int minValue) {
    if(m_min == minValue){
        return;
    }
    m_min = minValue;
    Invalidate();
}

int UIProgress::GetMaxValue() const {
    return m_max;
}

void UIProgress::SetMaxValue(int maxValue) {
    if(m_max == maxValue){
        return;
    }
    m_max = maxValue;
    Invalidate();
}

int UIProgress::GetValue() const {
    return m_value;
}

void UIProgress::SetValue(int value) {
    if(m_value == value){
        return;
    }
    m_value = value;
    if(m_value>m_max){
        m_value = m_max;
    }
    if(m_value<m_min){
        m_value = m_min;
    }
    Invalidate();
}

UIString UIProgress::GetForeImage() const {
    return m_diFore.sDrawString;
}

void UIProgress::SetForeImage(const UIString &foreImage) {
    if(m_diFore.sDrawString==foreImage && m_diFore.pImageInfo!=nullptr)
    {
        return;
    }
    m_diFore.Clear();
    m_diFore.sDrawString = foreImage;
    Invalidate();
}

void UIProgress::SetAttribute(const char *pstrName, const char *pstrValue) {
    if( strcasecmp(pstrName, "foreimage") == 0 ) SetForeImage(UIString{pstrValue});
    else if( strcasecmp(pstrName, "hor") == 0 ) SetHorizontal(strcasecmp(pstrValue, "true") == 0);
    else if( strcasecmp(pstrName, "min") == 0 ) SetMinValue(atoi(pstrValue));
    else if( strcasecmp(pstrName, "max") == 0 ) SetMaxValue(atoi(pstrValue));
    else if( strcasecmp(pstrName, "value") == 0 ) SetValue(atoi(pstrValue));
    else UILabel::SetAttribute(pstrName, pstrValue);
}

void UIProgress::PaintStatusImage(HANDLE_DC hDC) {
    if( m_max <= m_min ) m_max = m_min + 1;
    if( m_value > m_max ) m_value = m_max;
    if( m_value < m_min ) m_value = m_min;

    RECT rc = {0};
    if( m_horizontal ) {
        rc.right = (m_value - m_min) * (m_rcItem.right - m_rcItem.left) / (m_max - m_min);
        rc.bottom = m_rcItem.bottom - m_rcItem.top;
    }
    else {
        rc.top = (m_rcItem.bottom - m_rcItem.top) * (m_max - m_value) / (m_max - m_min);
        rc.right = m_rcItem.right - m_rcItem.left;
        rc.bottom = m_rcItem.bottom - m_rcItem.top;
    }
    m_diFore.rcDestOffset = rc;
    if( DrawImage(hDC, m_diFore) ) return;
}
