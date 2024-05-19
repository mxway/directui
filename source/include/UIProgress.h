#ifndef DIRECTUI_UIPROGRESS_H
#define DIRECTUI_UIPROGRESS_H
#include <UILabel.h>

class UIProgress : public UILabel
{
public:
    UIProgress();

    UIString    GetClass() const override;

    LPVOID      GetInterface(const UIString &name) override;

    bool        IsHorizontal()const;
    void        SetHorizontal(bool horizontal=true);
    int         GetMinValue()const;
    void        SetMinValue(int minValue);
    int         GetMaxValue()const;
    void        SetMaxValue(int maxValue);
    int         GetValue()const;
    void        SetValue(int value);
    UIString    GetForeImage()const;
    void        SetForeImage(const UIString &foreImage);

    void        SetAttribute(const char *pstrName, const char *pstrValue) override;

    void        PaintStatusImage(HANDLE_DC hDC) override;
protected:
    bool        m_horizontal;
    int         m_max;
    int         m_min;
    int         m_value;
    TDrawInfo   m_diFore;

};

#endif //DIRECTUI_UIPROGRESS_H