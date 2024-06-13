#ifndef DIRECTUI_UIPAINTMANAGER_H
#define DIRECTUI_UIPAINTMANAGER_H
#include <UIControl.h>
#include <memory>
#include <INotifyUI.h>

using namespace std;

// Flags for CControlUI::GetControlFlags()
#define UIFLAG_TABSTOP       0x00000001
#define UIFLAG_SETCURSOR     0x00000002
#define UIFLAG_WANTRETURN    0x00000004

// Flags for FindControl()
#define UIFIND_ALL           0x00000000
#define UIFIND_VISIBLE       0x00000001
#define UIFIND_ENABLED       0x00000002
#define UIFIND_HITTEST       0x00000004
#define UIFIND_UPDATETEST    0x00000008
#define UIFIND_TOP_FIRST     0x00000010
#define UIFIND_ME_FIRST      0x80000000

// Flags used for controlling the paint
#define UISTATE_FOCUSED      0x00000001
#define UISTATE_SELECTED     0x00000002
#define UISTATE_DISABLED     0x00000004
#define UISTATE_HOT          0x00000008
#define UISTATE_PUSHED       0x00000010
#define UISTATE_READONLY     0x00000020
#define UISTATE_CAPTURED     0x00000040

typedef enum tagEVENTTYPE_UI
{
    UIEVENT__FIRST = 1,
    UIEVENT__KEYBEGIN,
    UIEVENT_KEYDOWN,
    UIEVENT_KEYUP,
    UIEVENT_CHAR,
    UIEVENT_SYSKEY,
    UIEVENT__KEYEND,
    UIEVENT__MOUSEBEGIN,
    UIEVENT_MOUSEMOVE,
    UIEVENT_MOUSELEAVE,
    UIEVENT_MOUSEENTER,
    UIEVENT_MOUSEHOVER,
    UIEVENT_BUTTONDOWN,
    UIEVENT_BUTTONUP,
    UIEVENT_RBUTTONDOWN,
    UIEVENT_DBLCLICK,
    UIEVENT_CONTEXTMENU,
    UIEVENT_SCROLLWHEEL,
    UIEVENT__MOUSEEND,
    UIEVENT_KILLFOCUS,
    UIEVENT_SETFOCUS,
    UIEVENT_WINDOWSIZE,
    UIEVENT_SETCURSOR,
    UIEVENT_TIMER,
    UIEVENT_NOTIFY,
    UIEVENT_COMMAND,
    UIEVENT__LAST,
}EVENTTYPE_UI;

class UIPaintManagerInternalImp;

class UIPaintManager
{
public:
    UIPaintManager();

	~UIPaintManager();

    void Init(HANDLE_WND hWnd, const UIString &name=UIString{""});

    static void    MessageLoop();

    void            ReapObjects(UIControl *control);

    void            AddDelayedCleanup(UIControl* pControl);
    void            AddMouseLeaveNeeded(UIControl* pControl);
    bool            RemoveMouseLeaveNeeded(UIControl *control);

    bool            RenameControl(UIControl *control, const UIString &name);

    void            Invalidate(RECT& rcItem);
    UIControl*      GetFocus() const;
    void            SetFocus(UIControl* pControl, bool bFocusWnd=true);

    uint32_t SetTimer(UIControl *pControl, uint32_t uElapse);
    bool KillTimer(UIControl* pControl, uint32_t nTimerID);
    void KillTimer(UIControl* pControl);
    void RemoveAllTimers();

    void            NeedUpdate();

    static bool            GetHSL(short *H, short *S, short *L);

    bool AddNotifier(INotifyUI* notifier);
    bool RemoveNotifier(INotifyUI* notifier);
    void SendNotify(TNotifyUI& Msg, bool bAsync = false, bool bEnableRepeat = true);
    void SendNotify(UIControl* pControl, const char *pstrMessage, WPARAM wParam = 0, LPARAM lParam = 0, bool bAsync = false, bool bEnableRepeat = true);

    void    UsedVirtualWnd(bool used);

    HANDLE_WND GetPaintWindow(){
        return m_paintWnd;
    }

    HANDLE_DC   GetPaintDC();
    POINT       GetMousePos()const;

    RECT&       GetCaptionRect();
    void        SetCaptionRect(RECT& rcCaption);
    void        SetRoundCorner(SIZE roundCorner);
    SIZE        GetRoundCorner()const;
    SIZE        GetInitSize()const;
    void        SetInitSize(int cx, int cy);
    RECT       GetSizeBox()const;
    void        SetSizeBox(RECT& rcSizeBox);
    SIZE        GetMinInfo() const;
    void        SetMinInfo(int cx, int cy);
    SIZE        GetMaxInfo() const;
    void        SetMaxInfo(int cx, int cy);

    bool MessageHandler(uint32_t uMsg, WPARAM wParam, LPARAM lParam, long& lRes);

    bool AttachDialog(UIControl* pControl);

    bool InitControls(UIControl* control, UIControl* parent = nullptr);
    void        SetCapture();
    void        ReleaseCapture();

    bool        IsCaptured() const;

    UIControl   *GetRoot()const;
    UIControl   *FindControl(POINT pt)const;
    UIControl   *FindControl(const char *pstrName)const;
    UIControl   *FindSubControlByPoint(UIControl *parent, POINT pt)const;
    UIControl   *FindSubControlByName(UIControl *parent, const char *pstrName)const;
    UIControl   *FindSubControlByClass(UIControl *parent, const char *pstrClass, int iIndex=0);
    UIPtrArray   *FindSubControlsByClass(UIControl *parent, const char *pstrClass);

    void        SetWindowAttribute(const char *pstrName, const char *pstrValue);

    void            SetDefaultDisabledColor(uint32_t disabledColor);
    uint32_t        GetDefaultDisabledColor()const;
    void            SetDefaultFontColor(uint32_t fontColor);
    uint32_t        GetDefaultFontColor()const;
    void            SetDefaultLinkFontColor(uint32_t linkFontColor);
    uint32_t        GetDefaultLinkFontColor()const;
    void            SetDefaultLinkHoverFontColor(uint32_t linkHoverFontColor);
    uint32_t        GetDefaultLinkHoverFontColor()const;
    void            SetDefaultSelectedBkColor(uint32_t selectedBkColor);
    uint32_t        GetDefaultSelectedBkColor()const;

    static UIPtrArray* GetPlugins();

    void AddDefaultAttributeList(const char *pStrControlName, const char *pStrControlAttrList, bool shared=false);

    const char* GetDefaultAttributeList(const char *pStrControlName) const;

    void        RemoveAllDefaultAttributeList();

    bool        AddOptionGroup(const UIString &groupName, UIControl *control);
    UIPtrArray  *GetOptionGroup(const UIString &groupName);
    void        RemoveOptionGroup(const UIString &groupName, UIControl *control);
    void        RemoveAllOptionGroups();

private:
    UIPtrArray* GetFoundControls();
    static UIControl* __FindControlFromNameHash(UIControl* pThis, LPVOID pData);
    static UIControl*  __FindControlFromPoint(UIControl* pThis, LPVOID pData);
    static UIControl*  __FindControlFromTab(UIControl* pThis, LPVOID pData);
    static UIControl*  __FindControlFromShortcut(UIControl* pThis, LPVOID pData);
    static UIControl*  __FindControlFromName(UIControl* pThis, LPVOID pData);
    static UIControl*  __FindControlFromClass(UIControl* pThis, LPVOID pData);
    static UIControl*  __FindControlsFromClass(UIControl* pThis, LPVOID pData);
    static UIControl*  __FindControlsFromUpdate(UIControl* pThis, LPVOID pData);
    static UIControl*  __FindControlFromUpdate(UIControl *pThis, LPVOID pData);

private:
    shared_ptr<UIPaintManagerInternalImp>   m_impl;
    UIString                m_sName;
    HANDLE_WND              m_paintWnd;
    bool                    m_bUpdateNeeded;
    bool                    m_bNoActivate;
    int                     m_iHoverTime;
    static      bool        m_useHSL;
    static      short       m_H;
    static      short       m_S;
    static      short       m_L;
    POINT                   m_ptLastMousePos;

    SIZE                    m_szMinWindow;
    SIZE                    m_szMaxWindow;
    SIZE                    m_szInitWindowSize;
    RECT                    m_rcSizeBox;
    SIZE                    m_roundCorner;
    RECT                    m_rcCaption;

    UIControl               *m_pRoot;
    UIControl               *m_pFocus;
    UIControl               *m_pEventHover;
    UIControl               *m_pEventClick;
    bool                    m_bMouseTracking;
    bool                    m_bMouseCapture;

    UIStringPtrMap          m_defaultAttributesMapping;
    UIStringPtrMap          m_mNameHash;
    UIStringPtrMap          m_optionGroup;
    UIPtrArray              m_aNotifiers;
    UIPtrArray              m_aTimers;
    UIPtrArray              m_aFoundControls;
    UIPtrArray              m_aDelayedCleanup;
    UIPtrArray              m_aNeedMouseLeaveNeeded;

    uint32_t                m_defaultDisabledColor;
    uint32_t                m_defaultFontColor;
    uint32_t                m_defaultLinkFontColor;
    uint32_t                m_defaultLinkHoverFontColor;
    uint32_t                m_defaultSelectedBkColor;

};

#endif //DIRECTUI_UIPAINTMANAGER_H
