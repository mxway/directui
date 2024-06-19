#ifndef DIRECTUI_UICONTROL_H
#define DIRECTUI_UICONTROL_H
#include <UIString.h>
#include <UIDefine.h>
#include <UIDelegate.h>
#include <UIStringPtrMap.h>

class UIPaintManager;

typedef UIControl* (*FindControlProc)(UIControl*, LPVOID);

typedef struct tagTPercentInfo {
    double left;
    double top;
    double right;
    double bottom;
} TPercentInfo;

class UIControl
{
public:
    UIControl();
    virtual void        Delete();
    virtual UIString    GetName()const;
    virtual void        SetName(const UIString &name);
    virtual UIString    GetClass()const;
    virtual LPVOID      GetInterface(const UIString &name);
    virtual uint32_t    GetControlFlags()const;
    virtual HANDLE_WND  GetNativeWindow()const;

    virtual bool        Activate();

    virtual UIPaintManager *GetManager()const;
    virtual void        SetManager(UIPaintManager *manager, UIControl *parent, bool bInit=true);
    virtual UIControl   *GetParent()const;
    virtual UIControl   *GetCover()const;
    virtual void        SetCover(UIControl *pControl);

    virtual UIString    GetText()const;
    virtual void        SetText(const UIString &text);

    uint32_t            GetBkColor()const;
    void                SetBkColor(uint32_t backColor);
    uint32_t            GetBkColor2()const;
    void                SetBkColor2(uint32_t backColor);
    uint32_t            GetBkColor3()const;
    void                SetBkColor3(uint32_t backColor);

    UIString            GetBkImage()const;
    void                SetBkImage(const UIString &bkImage);

    uint32_t            GetFocusBorderColor()const;
    void                SetFocusBorderColor(uint32_t borderColor);

    bool                IsColorHSL()const;
    void                SetColorHSL(bool colorHSL);
    SIZE                GetBorderRound()const;
    void                SetBorderRound(SIZE cxyRound);

    bool                DrawImage(HANDLE_DC hDC, TDrawInfo &drawInfo);

    uint32_t            GetBorderColor()const;
    void                SetBorderColor(uint32_t borderColor);
    RECT                GetBorderSize()const;
    void                SetBorderSize(RECT rc);
    void                SetBorderSize(int iSize);

    int                 GetBorderStyle()const;
    void                SetBorderStyle(int nStyle);

    //位置相关
    virtual const RECT  &GetPos()const;
    // 相对(父控件)位置
    virtual RECT        GetRelativePos()const;
    //客户区域（除去scrollbar和inset）
    virtual RECT        GetClientPos()const;

    //只有控件为float的时候，外部调用SetPos和Move才是有效的，位置参数是相对父控件的位置
    virtual void        SetPos(RECT rc, bool bNeedInvalidate=true);
    virtual void        Move(SIZE szOffset, bool bNeedInvalidate=true);
    virtual int         GetWidth()const;
    virtual int         GetHeight()const;
    virtual int         GetX()const;
    virtual int         GetY()const;
    virtual RECT        GetPadding()const;
    virtual void        SetPadding(RECT rcPadding);
    virtual SIZE        GetFixedXY()const;
    virtual void        SetFixedXY(SIZE szXY);

    virtual TPercentInfo        GetFloatPercent() const;
    virtual void        SetFloatPercent(TPercentInfo piFloatPercent);

    virtual int         GetFixedWidth()const;
    virtual void        SetFixedWidth(int cx);
    virtual int         GetFixedHeight()const;
    virtual void        SetFixedHeight(int cy);
    virtual int         GetMinWidth()const;
    virtual void        SetMinWidth(int cx);
    virtual int         GetMaxWidth()const;
    virtual void        SetMaxWidth(int cx);
    virtual int         GetMinHeight()const;
    virtual void        SetMinHeight(int cy);
    virtual int         GetMaxHeight()const;
    virtual void        SetMaxHeight(int cy);

    //TODO 鼠标提示
//    virtual UIString GetToolTip()const;
//    virtual void    SetToolTip(const char *pstrText);
//    virtual void    SetToolTipWidth(int nWidth);
//    virtual int     GetToolTipWidth();

    //快捷键
    virtual char        GetShortcut()const;
    virtual void        SetShortcut(char ch);

    // 菜单
    virtual bool        IsContextMenuUsed()const;
    virtual void        SetContextMenuUsed(bool bMenuUsed);

    //用户属性
    virtual const UIString &GetUserData();//辅助函数，供用户使用
    virtual void            SetUserData(const char *pstrText);//辅助函数供用户使用
    virtual void*           GetTag()const; //辅助函数，供用户使用
    virtual void            SetTag(void *pTag);//辅助函数，供用户使用

    //一些重要属性
    virtual bool            IsVisible()const;
    virtual void            SetVisible(bool bVisible=true);
    virtual void            SetInternVisible(bool bVisible=true);//仅供内部调用，有些UI拥有窗口句柄，需要重写此函数
    virtual bool            IsEnabled()const;
    virtual void            SetEnabled(bool bEnabled = true);
    virtual bool            IsMouseEnabled()const;
    virtual void            SetMouseEnabled(bool bEnabled=true);
    virtual bool            IsKeyboardEnabled()const;
    virtual void            SetKeyboardEnabled(bool bEnabled = true);
    virtual bool            IsFocused()const;
    virtual void            SetFocus();
    virtual bool            IsFloat()const;
    virtual void            SetFloat(bool bFloat=true);

    //自定义（未处理的）属性
    void                    AddCustomAttribute(const char *name, const char *value);
    const char*             GetCustomAttribute(const char *name)const;
    bool                    RemoveCustomAttribute(const char*name);
    void                    RemoveAllCustomeAttribute();

    virtual     UIControl   *FindControl(FindControlProc Proc, LPVOID pData, uint32_t uFlags);

    virtual void            Invalidate();
    bool                    IsUpdateNeeded()const;
    void                    NeedUpdate();
    void                    NeedParentUpdate();
    uint32_t                GetAdjustColor(uint32_t color);

    virtual void            Init();
    virtual void            DoInit();
    virtual void            Event(TEventUI& event);
    virtual void            DoEvent(TEventUI& event);

    virtual UIString        GetAttribute(const char *name);
    virtual void            SetAttribute(const char *name, const char *value);
    virtual UIString        GetAttributeList(bool bIgnoreDefault=true);
    virtual void            SetAttributeList(const char *pstrList);

    virtual SIZE            EstimateSize(SIZE szAvailable);
    virtual bool            Paint(HANDLE_DC hDC, const RECT &rcPaint, UIControl *pStopControl = nullptr);//返回要不要继续绘制
    virtual bool            DoPaint(HANDLE_DC hDC, const RECT &rcPaint, UIControl *pStopControl);
    virtual void            PaintBkColor(HANDLE_DC hDC);
    virtual void            PaintBkImage(HANDLE_DC hDC);
    virtual void            PaintStatusImage(HANDLE_DC hDC);
    virtual void            PaintText(HANDLE_DC hDC);
    virtual void            PaintBorder(HANDLE_DC hDC);

    virtual void            DoPostPaint(HANDLE_DC hDC, const RECT &rcPaint);

    //虚拟窗口参数
    void                    SetVirtualWnd(const char *pstrValue);
    UIString                GetVirtualWnd()const;

public:
    virtual ~UIControl();

public:
    CEventSource    OnInit;
    CEventSource    OnDestroy;
    CEventSource    OnSize;
    CEventSource    OnEvent;
    CEventSource    OnNotify;
    CEventSource    OnPaint;
    CEventSource    OnPostPaint;

protected:
    UIPaintManager  *m_manager;
    UIControl       *m_parent;
    UIControl       *m_cover;
    UIString        m_virtualWnd;
    UIString        m_name;
    bool            m_updateNeeded;
    bool            m_menuUsed;
    bool            m_asyncNotify;
    RECT            m_rcItem;
    RECT            m_rcPadding;
    SIZE            m_cXY;
    SIZE            m_cxyFixed;
    SIZE            m_cxyMin;
    SIZE            m_cxyMax;
    bool            m_visible;
    bool            m_internVisible;
    bool            m_enabled;
    bool            m_mouseEnabled;
    bool            m_keyboardEnabled;
    bool            m_focused;
    bool            m_float;
    TPercentInfo    m_piFloatPercent;
    bool            m_setPos;
    UIString        m_text;
    //UIString        m_toolTip;
    char            m_shortCut;
    UIString        m_userData;
    LPVOID          m_tag;

    uint32_t        m_backColor;
    uint32_t        m_backColor2;
    uint32_t        m_backColor3;
    TDrawInfo       m_diBk;
    TDrawInfo       m_diFore;
    uint32_t        m_borderColor;
    uint32_t        m_focusBorderColor;
    bool            m_colorHSL;
    int             m_borderStyle;
    //int             m_tooltipWidth;
    SIZE            m_cxyBorderRound;
    RECT            m_rcPaint;
    RECT            m_rcBorderSize;
    UIStringPtrMap  m_customAttrHash;
};

#endif //DIRECTUI_UICONTROL_H
