#ifndef DIRECTUI_UISCROLLBAR_H
#define DIRECTUI_UISCROLLBAR_H
#include <UIControl.h>

class UIContainer;

class UIScrollBar : public UIControl
{
public:
    UIScrollBar();

    UIString GetClass() const override;

    LPVOID GetInterface(const UIString &name) override;

    UIContainer *GetOwner()const;
    void        SetOwner(UIContainer *owner);

    void        SetVisible(bool visible=true);
    void        SetEnabled(bool enabled=true);
    void        SetFocus();

    bool        IsHorizontal();
    void        SetHorizontal(bool horizontal=true);
    int         GetScrollRange()const;
    void        SetScrollRange(int range);
    int         GetScrollPos()const;
    void        SetScrollPos(int pos, bool triggerEvent=true);
    int         GetLineSize()const;
    void        SetLineSize(int size);
    int         GetScrollUnit()const;
    void        SetScrollUnit(int unit);

    bool        GetShowButton1();
    void        SetShowButton1(bool show);
    uint32_t    GetButton1Color()const;
    void        SetButton1Color(uint32_t color);
    UIString    GetButton1NormalImage()const;
    void        SetButton1NormalImage(const UIString &normalImage);
    UIString    GetButton1HotImage()const;
    void        SetButton1HotImage(const UIString &hotImage);
    UIString    GetButton1PushedImage()const;
    void        SetButton1PushedImage(const UIString &pushedImage);
    UIString    GetButton1DisabledImage()const;
    void        SetButton1DisabledImage(const UIString &disabledImage);

    bool        GetShowButton2();
    void        SetShowButton2(bool show);
    uint32_t    GetButton2Color()const;
    void        SetButton2Color(uint32_t color);
    UIString    GetButton2NormalImage()const;
    void        SetButton2NormalImage(const UIString &normalImage);
    UIString    GetButton2HotImage()const;
    void        SetButton2HotImage(const UIString &hotImage);
    UIString    GetButton2PushedImage()const;
    void        SetButton2PushedImage(const UIString &pushedImage);
    UIString    GetButton2DisabledImage()const;
    void        SetButton2DisabledImage(const UIString &disabledImage);

    uint32_t    GetThumbColor()const;
    void        SetThumbColor(uint32_t color);
    UIString    GetThumbNormalImage()const;
    void        SetThumbNormalImage(const UIString &normalImage);
    UIString    GetThumbHotImage()const;
    void        SetThumbHotImage(const UIString &hotImage);
    UIString    GetThumbPushedImage()const;
    void        SetThumbPushedImage(const UIString &pushedImage);
    UIString    GetThumbDisabledImage()const;
    void        SetThumbDisabledImage(const UIString &disabledImage);

    UIString    GetRailNormalImage()const;
    void        SetRailNormalImage(const UIString &normalImage);
    UIString    GetRailHotImage()const;
    void        SetRailHotImage(const UIString &hotImage);
    UIString    GetRailPushedImage()const;
    void        SetRailPushedImage(const UIString &pushedImage);
    UIString    GetRailDisabledImage()const;
    void        SetRailDisabledImage(const UIString &disabledImage);

    UIString    GetBkNormalImage()const;
    void        SetBkNormalImage(const UIString &normalImage);
    UIString    GetBkHotImage()const;
    void        SetBkHotImage(const UIString &hotImage);
    UIString    GetBkPushedImage()const;
    void        SetBkPushedImage(const UIString &pushedImage);
    UIString    GetBkDisabledImage()const;
    void        SetBkDisabledImage(const UIString &disabledImage);

    void        SetPos(RECT rc, bool bNeedInvalidate=true)override;
    void        DoEvent(TEventUI &event)override;
    void        SetAttribute(const char *name, const char *value)override;
    bool        DoPaint(HANDLE_DC  hDC, const RECT &rcPaint, UIControl *stopControl)override;

    void        PaintBk(HANDLE_DC  hDC);
    void        PaintButton1(HANDLE_DC hDC);
    void        PaintButton2(HANDLE_DC hDC);
    void        PaintThumb(HANDLE_DC hDC);
    void        PaintRail(HANDLE_DC hDC);

protected:
    enum{
        DEFAULT_SCROLLBAR_SIZE=16,
    };
    uint32_t m_timerId;
    bool    m_horizontal;
    int     m_range;
    int     m_scrollPos;
    int     m_lineSize;
    int     m_scrollUnit;
    UIContainer *m_owner;
    POINT   ptLastMouse;
    int     m_lastScrollPos;
    int     m_lastScrollOffset;
    int     m_scrollRepeatDelay;
    TDrawInfo   m_diBkNormal;
    TDrawInfo   m_diBkHot;
    TDrawInfo   m_diBkPushed;
    TDrawInfo   m_diBkDisabled;

    bool        m_showButton1;
    RECT        m_rcButton1;
    uint32_t    m_button1State;
    uint32_t    m_button1Color;
    TDrawInfo   m_diButton1Normal;
    TDrawInfo   m_diButton1Hot;
    TDrawInfo   m_diButton1Pushed;
    TDrawInfo   m_diButton1Disabled;

    bool        m_showButton2;
    RECT        m_rcButton2;
    uint32_t    m_button2State;
    uint32_t    m_button2Color;
    TDrawInfo   m_diButton2Normal;
    TDrawInfo   m_diButton2Hot;
    TDrawInfo   m_diButton2Pushed;
    TDrawInfo   m_diButton2Disabled;

    RECT        m_rcThumb;
    uint32_t    m_thumbState;
    uint32_t    m_thumbColor;
    TDrawInfo   m_diThumbNormal;
    TDrawInfo   m_diThumbHot;
    TDrawInfo   m_diThumbPushed;
    TDrawInfo   m_diThumbDisabled;

    TDrawInfo   m_diRailNormal;
    TDrawInfo   m_diRailHot;
    TDrawInfo   m_diRailPushed;
    TDrawInfo   m_diRailDisabled;
};

#endif //DIRECTUI_UISCROLLBAR_H