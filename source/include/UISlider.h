#ifndef DIRECTUI_UISLIDER_H
#define DIRECTUI_UISLIDER_H
#include <UIProgress.h>

class UISlider : public UIProgress
{
public:
    UISlider();

    uint32_t    GetControlFlags() const override;

    UIString    GetClass() const override;

    LPVOID      GetInterface(const UIString &name) override;

    void        SetEnabled(bool bEnabled) override;

    int         GetChangeStep()const;
    void        SetChangeStep(int step);
    RECT        GetThumbRect()const;
    void        SetThumbSize(SIZE szXY);
    bool        IsImmMode()const;
    void        SetImmMode(bool immMode);
    UIString    GetThumbImage()const;
    void        SetThumbImage(const UIString &thumbImage);
    UIString    GetThumbHotImage()const;
    void        SetThumbHotImage(const UIString &hotImage);
    UIString    GetThumbPushedImage()const;
    void        SetThumbPushedImage(const UIString &pushedImage);

    void        DoEvent(TEventUI &event)override;
    void        SetAttribute(const char *pstrName, const char *pstrValue)override;
    void        PaintStatusImage(HANDLE_DC hDC)override;
protected:
    SIZE        m_szThumb;
    uint32_t    m_buttonState;
    int         m_step;
    bool        m_immMode;
    TDrawInfo   m_diThumb;
    TDrawInfo   m_diThumbHot;
    TDrawInfo   m_diThumbPushed;
};

#endif //DIRECTUI_UISLIDER_H