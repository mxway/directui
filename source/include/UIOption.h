#ifndef DIRECTUI_UIOPTION_H
#define DIRECTUI_UIOPTION_H
#include <UIButton.h>

class UIOption : public UIButton
{
public:
    UIOption();
    ~UIOption();

    UIString        GetClass() const override;

    LPVOID          GetInterface(const UIString &name) override;

    bool            Activate() override;

    void            SetEnabled(bool bEnabled) override;

    void            SetManager(UIPaintManager *manager, UIControl *parent, bool bInit=true) override;

    UIString        GetSelectedImage()const;
    void            SetSelectedImage(const UIString &selectedImage);
    UIString        GetSelectedHotImage()const;
    void            SetSelectedHotImage(const UIString &selectedHotImage);
    uint32_t        GetSelectedTextColor();
    void            SetSelectedTextColor(uint32_t selectedTextColor);
    uint32_t        GetSelectedBkColor() const;
    void            SetSelectedBkColor(uint32_t bkcolor);
    UIString        GetForeImage()const;
    void            SetForeImage(const UIString &foreImage);
    UIString        GetGroup()const;
    void            SetGroup(const UIString &pstrGroupName);
    bool            IsSelected()const;
    virtual  void   Selected(bool selected, bool triggerEvent=true);
    SIZE            EstimateSize(SIZE szAvailabe)override;
    void            SetAttribute(const char *pstrName, const char *pstrValue) override;
    void            PaintStatusImage(HANDLE_DC hDC) override;
    void            PaintText(HANDLE_DC hDC) override;
protected:
    bool            m_selected;
    UIString        m_groupName;
    uint32_t        m_selectedBkColor;
    uint32_t        m_selectedTextColor;
    TDrawInfo       m_diSelected;
    TDrawInfo       m_diSelectedHot;
    TDrawInfo       m_diFore;
};

#endif //DIRECTUI_UIOPTION_H