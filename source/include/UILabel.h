#ifndef DIRECTUI_UILABEL_H
#define DIRECTUI_UILABEL_H
#include <UIControl.h>

class UILabel : public UIControl
{
public:
    UILabel();

    UIString GetClass() const override;

    LPVOID GetInterface(const UIString &name) override;

    void    SetTextStyle(uint32_t style);
    uint32_t GetTextStyle()const;
    void    SetTextColor(uint32_t textColor);
    uint32_t GetTextColor()const;
    void    SetDisabledTextColor(uint32_t disabledTextColor);
    uint32_t GetDisabledTextColor()const;
    void     SetFont(int fontId);
    int      GetFont()const;
    RECT    GetTextPadding()const;
    void    SetTextPadding(const RECT &rc);
    bool    IsShowHtml() const;
    void    SetShowHtml(bool showHtml=true);
    SIZE    EstimateSize(SIZE szAvailable)override;
    void    DoEvent(TEventUI &event)override;
    void    SetAttribute(const char *pstrName, const char *pstrValue)override;

    void    PaintText(HANDLE_DC hDC)override;

protected:
    uint32_t m_textColor;
    uint32_t m_disabledTextColor;
    int      m_fontId;
    uint32_t m_textStyle;
    RECT     m_rcTextPadding;
    bool     m_showHtml;

};

#endif //DIRECTUI_UILABEL_H