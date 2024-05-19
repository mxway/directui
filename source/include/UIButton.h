#ifndef DIRECTUI_UIBUTTON_H
#define DIRECTUI_UIBUTTON_H
#include <UILabel.h>

class UIButton : public UILabel
{
public:
    UIButton();

    UIString GetClass() const override;

    LPVOID GetInterface(const UIString &name) override;

    uint32_t GetControlFlags() const override;

    bool Activate() override;

    void SetEnabled(bool bEnabled) override;

    void DoEvent(TEventUI &event) override;

    UIString        GetNormalImage()const;
    void            SetNormalImage(const UIString &normalImage);
    UIString        GetHotImage()const;
    void            SetHotImage(const UIString &hotImage);
    UIString        GetPushedImage()const;
    void            SetPushedImage(const UIString &pushedImage);
    UIString        GetFocusedImage()const;
    void            SetFocusedImage(const UIString &focusedImage);
    UIString        GetDisabledImage()const;
    void            SetDisabledImage(const UIString &disabledImage);
    UIString        GetForeImage()const;
    void            SetForeImage(const UIString &foreImage);
    UIString        GetHotForeImage()const;
    void            SetHotForeImage(const UIString &hotForeImage);

    void            SetFiveStatusImage(const UIString &fiveStatusImage);
    void            SetFadeAlphaDelta(uint8_t delta);
    uint8_t         GetFadeAlphaDelta()const;

    void            SetHotBkColor(uint32_t color);
    uint32_t        GetHotBkColor()const;
    void            SetHotTextColor(uint32_t color);
    uint32_t        GetHotTextColor()const;
    void            SetPushedTextColor(uint32_t color);
    uint32_t        GetPushedTextColor()const;
    void            SetFocusedTextColor(uint32_t color);
    uint32_t        GetFocusedTextColor()const;
    SIZE            EstimateSize(SIZE szAvailable) override;
    void            SetAttribute(const char *pstrName, const char *pstrValue)override;

    void            PaintText(HANDLE_DC hDC)override;
    void            PaintStatusImage(HANDLE_DC hDC)override;

protected:
    enum {
        FADE_TIMERID = 11,
        FADE_ELLAPSE = 30,
    };

    uint32_t    m_buttonState;
    uint32_t    m_hotBkColor;
    uint32_t    m_hotTextColor;
    uint32_t    m_pushedTextColor;
    uint32_t    m_focusedTextColor;
    uint8_t     m_fadeAlpha;
    uint8_t     m_fadeAlphaDelta;

    TDrawInfo   m_diNormal;
    TDrawInfo   m_diHot;
    TDrawInfo   m_diHotFore;
    TDrawInfo   m_diPushed;
    TDrawInfo   m_diPushedFore;
    TDrawInfo   m_diFocused;
    TDrawInfo   m_diDisabled;
};

#endif //DIRECTUI_UIBUTTON_H