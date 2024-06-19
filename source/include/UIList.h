#ifndef DIRECTUI_UILIST_H
#define DIRECTUI_UILIST_H
#include <UIDefine.h>
#include <UIContainer.h>
#include <UIVerticalLayout.h>
#include <UIHorizontalLayout.h>

typedef int (*PULVCompareFunc)(LPVOID, LPVOID, LPVOID);

class UIListHeader;

#define UILIST_MAX_COLUMNS 64

typedef struct tagTListInfoUI
{
    int nColumns;
    RECT        rcColumn[UILIST_MAX_COLUMNS];
    uint32_t    uFixedHeight;
    int         nFont;
    uint32_t    textStyle;
    RECT        rcTextPadding;
    uint32_t    dwTextColor;
    uint32_t    dwBkColor;
    TDrawInfo   diBk;
    bool        bAlternateBk;
    uint32_t    dwSelectedTextColor;
    uint32_t    dwSelectedBkColor;
    TDrawInfo   diSelected;
    uint32_t    dwHotTextColor;
    uint32_t    dwHotBkColor;
    TDrawInfo   diHot;
    uint32_t    dwDisabledTextColor;
    uint32_t    dwDisabledBkColor;
    TDrawInfo   diDisabled;
    int         iHLineSize;
    uint32_t    dwHLineColor;
    int         iVLineSize;
    uint32_t    dwVLineColor;
    bool        bShowHtml;
    bool        bMultiExpandable;
}TListInfoUI;

class IListCallbackUI
{
public:
    virtual ~IListCallbackUI()=default;
    virtual UIString GetItemText(UIControl *pList,int iItem, int iSubItem) = 0;
};

class IListOwnerUI
{
public:
    virtual ~IListOwnerUI()=default;
    virtual     TListInfoUI  *GetListInfo() = 0;
    virtual     int          GetCurSel() const = 0;
    virtual     bool         SelectItem(int iIndex, bool bTakeFocus=false, bool bTriggerEvent=true)=0;
    virtual     void         DoEvent(TEventUI &event) = 0;
    virtual     bool         ExpandItem(int iIndex, bool bExpand=true)= 0;
    virtual     int         GetExpandedItem()const = 0;
};

class IListUI : public IListOwnerUI
{
public:
    virtual ~IListUI()=default;
    virtual UIListHeader    *GetHeader()const = 0;
    virtual UIContainer     *GetList() const = 0;
    virtual IListCallbackUI *GetTextCallback()const=0;
    virtual void            SetTextCallback(IListCallbackUI *pCallback) = 0;
};

class IListItemUI
{
public:
    virtual ~IListItemUI()=default;
    virtual     int        GetIndex()const=0;
    virtual     void       SetIndex(int iIndex) = 0;
    virtual     int        GetDrawIndex()const=0;
    virtual     void       SetDrawIndex(int iIndex) = 0;
    virtual     IListOwnerUI    *GetOwner()=0;
    virtual     void            SetOwner(UIControl *pOwner) = 0;
    virtual     bool            IsSelected()const=0;
    virtual     bool            Select(bool bSelect=true,bool bTriggerEvent=true)=0;
    virtual     bool            IsExpanded()const=0;
    virtual     bool            Expand(bool bExpand = true)=0;
    virtual     void            DrawItemText(HANDLE_DC hDC, const RECT &rcItem) = 0;
};

class UIListBody;
class UIListHeader;

class UIList : public UIVerticalLayout, public IListUI
{
public:
    UIList();

    UIString GetClass() const override;

    LPVOID GetInterface(const UIString &name) override;

    uint32_t GetControlFlags() const override;

    bool            GetScrollSelect()const;
    void            SetScrollSelect(bool bScrollSelect);
    int             GetCurSel()const;
    bool            SelectItem(int iIndex, bool bTakeFocus = false, bool bTriggerEvent=true);

    UIControl       *GetItemAt(int iIndex)const;
    int             GetItemIndex(UIControl *control)const;
    bool            SetItemIndex(UIControl *control, int iIndex);
    bool            SetMultiItemIndex(UIControl *startControl, int iCount, int iNewStartIndex);
    int             GetCount()const;
    bool            Add(UIControl *control);
    bool            AddAt(UIControl *control, int iIndex);
    bool            Remove(UIControl *control, bool bDoNotDestroy=false);
    bool            RemoveAt(int iIndex, bool bDoNotDestroy=false);
    void            RemoveAll();
    void            EnsureVisible(int iIndex);
    void            Scroll(int dx, int dy);
    int             GetChildPadding()const;
    void            SetChildPadding(int iPadding);

    UIListHeader    *GetHeader()const;
    UIContainer     *GetList()const;
    TListInfoUI     *GetListInfo();

    uint32_t        GetItemFixedHeight()const;
    void            SetItemFixedHeight(uint32_t nHeight);
    int             GetItemFont(int index)const;
    void            SetItemFont(int index);
    uint32_t        GetItemTextStyle()const;
    void            SetItemTextStyle(uint32_t style);
    RECT            GetItemTextPadding()const;
    void            SetItemTextPadding(RECT rc);
    uint32_t        GetItemTextColor()const;
    void            SetItemTextColor(uint32_t textColor);
    uint32_t        GetItemBkColor()const;
    void            SetItemBkColor(uint32_t bkColor);
    UIString        GetItemBkImage()const;
    void            SetItemBkImage(const UIString &bkImage);
    bool            IsAlternateBk()const;
    void            SetAlternateBk(bool alternateBk);
    uint32_t        GetSelectedItemTextColor()const;
    void            SetSelectedItemTextColor(uint32_t  textColor);
    uint32_t        GetSelectedItemBkColor()const;
    void            SetSelectedItemBkColor(uint32_t bkColor);
    UIString        GetSelectedItemImage()const;
    void            SetSelectedItemImage(const UIString &selectedImage);
    uint32_t        GetHotItemTextColor()const;
    void            SetHotItemTextColor(uint32_t textColor);
    uint32_t        GetHotItemBkColor()const;
    void            SetHotItemBkColor(uint32_t bkColor);
    UIString        GetHotItemImage()const;
    void            SetHotItemImage(const UIString &hotItemImage);
    uint32_t        GetDisabledItemTextColor()const;
    void            SetDisabledItemTextColor(uint32_t itemTextColor);
    uint32_t        GetDisabledItemBkColor()const;
    void            SetDisabledItemBkColor(uint32_t bkColor);

    UIString        GetDisabledItemImage()const;
    void            SetDisabledItemImage(const UIString &image);
    int             GetItemHLineSize()const;
    void            SetItemHLineSize(int size);
    uint32_t        GetItemHLineColor()const;
    void            SetItemHLineColor(uint32_t lineColor);
    uint32_t        GetItemVLineSize()const;
    void            SetItemVLineSize(int size);
    uint32_t        GetItemVLineColor()const;
    void            SetItemVLineColor(uint32_t lineColor);
    bool            IsItemShowHtml() const;
    void            SetItemShowHtml(bool bShowHtml=true);

    void            SetMultiExpanding(bool bMultiExpandable);
    int             GetExpandedItem()const;
    bool            ExpandItem(int iIndex, bool bExpand=true);

    void            SetPos(RECT rc, bool bNeedInvalidate=true)override;
    void            Move(SIZE szOffset, bool bNeedInvalidate=true)override;
    void            DoEvent(TEventUI  &event)override;
    void            SetAttribute(const char *pstrName, const char *pstrValue)override;

    IListCallbackUI     *GetTextCallback()const;
    void                SetTextCallback(IListCallbackUI *callback);

    SIZE            GetScrollPos()const;
    SIZE            GetScrollRange()const;
    void            SetScrollPos(SIZE pos);
    void            LineUp();
    void            LineDown();
    void            PageUp();
    void            PageDown();
    void            HomeUp();
    void            EndDown();
    void            LineLeft();
    void            LineRight();
    void            PageLeft();
    void            PageRight();
    void            HomeLeft();
    void            EndRight();
    void            EnableScrollBar(bool bEnableVertical=true, bool bEnableHorizontal=false);
    virtual         UIScrollBar *GetVerticalScrollBar()const;
    virtual         UIScrollBar *GetHorizontalScrollBar()const;
    bool            SortItems(PULVCompareFunc pfnCompare, LPVOID data);

protected:
    bool            m_scrollSelect;
    int             m_curSel;
    int             m_expandedItem;
    IListCallbackUI *m_callback;
    UIListBody      *m_pList;
    UIListHeader    *m_pHeader;
    TListInfoUI     m_ListInfo;
};

class UIListHeader : public  UIHorizontalLayout
{
public:
    UIListHeader();

    SIZE EstimateSize(SIZE szAvailable) override;

    UIString GetClass() const override;

    LPVOID GetInterface(const UIString &name) override;
};

class UIListHeaderItem : public UIControl
{
public:
    UIListHeaderItem();

    UIString    GetClass() const override;

    LPVOID      GetInterface(const UIString &name) override;

    uint32_t    GetControlFlags() const override;

    void        SetEnabled(bool bEnabled) override;

    bool        IsDraggable()const;
    void        SetDraggable(bool draggable);
    uint32_t    GetSepWidth()const;
    void        SetSepWidth(int width);
    uint32_t    GetTextStyle()const;
    void        SetTextStyle(uint32_t style);
    uint32_t    GetTextColor()const;
    void        SetTextColor(uint32_t textColor);
    uint32_t    GetSepColor()const;
    void        SetSepColor(uint32_t sepColor);
    void        SetTextPadding(RECT  rc);
    RECT        GetTextPadding()const;
    void        SetFont(int index);
    bool        IsShowHtml() const;
    void        SetShowHtml(bool showHtml=true);
    UIString    GetNormalImage()const;
    void        SetNormalImage(const UIString &normalImage);
    UIString    GetHotImage()const;
    void        SetHotImage(const UIString &hotImage);
    UIString    GetPushedImage()const;
    void        SetPushedImage(const UIString &pushedImage);
    UIString    GetFocusedImage()const;
    void        SetFocusedImage(const UIString &focusedImage);
    UIString    GetSepImage()const;
    void        SetSepImage(const UIString &sepImage);

    void        DoEvent(TEventUI &event)override;
    SIZE        EstimateSize(SIZE szAvailable);
    void        SetAttribute(const char *pstrName, const char *pstrValue)override;
    RECT        GetThumbRect()const;

    void        PaintText(HANDLE_DC hDC)override;
    void        PaintStatusImage(HANDLE_DC hDC);
protected:
    POINT       ptLastMouse;
    bool        m_dragable;
    uint32_t    m_buttonState;
    int         m_sepWidth;
    uint32_t    m_textColor;
    uint32_t    m_sepColor;
    int         m_font;
    uint32_t    m_textStyle;
    bool        m_showHtml;
    RECT        m_textPadding;
    TDrawInfo   m_diNormal;
    TDrawInfo   m_diHot;
    TDrawInfo   m_diPushed;
    TDrawInfo   m_diFocused;
    TDrawInfo   m_diSep;
};

class UIListElement : public UIControl, public IListItemUI
{
public:
    UIListElement();

    UIString        GetClass() const override;

    LPVOID          GetInterface(const UIString &name) override;

    uint32_t        GetControlFlags() const override;

    void            SetEnabled(bool bEnable=true) override;

    int             GetIndex()const;
    void            SetIndex(int index);
    int             GetDrawIndex()const;
    void            SetDrawIndex(int index);
    IListOwnerUI    *GetOwner();
    void            SetOwner(UIControl *owner);
    void            SetVisible(bool visible=true);
    bool            IsSelected()const;
    bool            Select(bool select=true, bool bTriggerEvent=true);
    bool            IsExpanded()const;
    bool            Expand(bool expand=true);

    void            Invalidate();
    bool            Activate()override;
    void            DoEvent(TEventUI &event)override;
    void            SetAttribute(const char *pstrName, const char *pstrValue)override;

    void            DrawItemBk(HANDLE_DC hDC, const RECT &rcItem);
protected:
    int             m_index;
    int             m_drawIndex;
    bool            m_selected;
    uint32_t        m_buttonState;
    IListOwnerUI    *m_owner;
};

class UIListLabelElement : public UIListElement
{
public:
    UIListLabelElement();

    UIString GetClass() const override;

    LPVOID GetInterface(const UIString &name) override;

    void            SetOwner(UIControl *owner);
    void            SetFixedWidth(int cx);
    void            SetFixedHeight(int cy);
    void            SetText(const UIString &text)override;
    void            DoEvent(TEventUI &event)override;
    SIZE            EstimateSize(SIZE szAvailable)override;
    bool            DoPaint(HANDLE_DC hDC, const RECT &rcPaint, UIControl *stopControl)override;
    void            DrawItemText(HANDLE_DC hDC, const RECT &rcItem);

protected:
    SIZE        m_cxyFixedLast;
    bool        m_needEstimateSize;
    SIZE        m_szAvailableLast;
    uint32_t    m_fixedHeightLast;
    int         m_fontLast;
    uint32_t    m_textStyleLast;
    RECT        m_textPaddingLast;
};

class UIListTextElement: public UIListLabelElement
{
public:
    UIListTextElement();
    ~UIListTextElement();

    uint32_t        GetControlFlags() const override;

    UIString        GetClass() const override;

    LPVOID          GetInterface(const UIString &name) override;

    UIString        GetText(int index)const;
    void            SetText(int index, const UIString &text);
    void            SetOwner(UIControl *owner);
    UIString        *GetLinkContent(int index);
    void            DoEvent(TEventUI  &event)override;
    SIZE            EstimateSize(SIZE szAvailable)override;
    void            DrawItemText(HANDLE_DC hDC, const RECT &rcItem);
protected:
    enum { MAX_LINK = 8};
    int         m_links;
    RECT        m_rcLinks[MAX_LINK];
    UIString    m_sLinks[MAX_LINK];
    int         m_hoverLink;
    IListUI     *m_ListOwner;
    UIPtrArray  m_texts;
    UIString    m_textLast;
};

class UIListContainerElement : public UIContainer, public IListItemUI
{
public:
    UIListContainerElement();
    UIString            GetClass()const override;
    uint32_t            GetControlFlags()const override;
    LPVOID              GetInterface(const UIString &name)override;

    int                 GetIndex()const;
    void                SetIndex(int index);
    int                 GetDrawIndex()const;
    void                SetDrawIndex(int index);
    IListOwnerUI        *GetOwner();
    void                SetOwner(UIControl *owner);
    void                SetVisible(bool visible=true)override;
    void                SetEnabled(bool enable=true);
    bool                IsSelected()const;
    bool                Select(bool select=true, bool triggerEvent=true);
    bool                IsExpandable()const;
    void SetExpandable(bool expandable);
    bool                IsExpanded()const;
    bool                Expand(bool expand=true);

    void                Invalidate()override;
    bool                Activate()override;
    void                DoEvent(TEventUI &event)override;
    void                SetAttribute(const char *pstrName, const char *pstrValue)override;
    bool                DoPaint(HANDLE_DC hDC, const RECT &rcPaint, UIControl *stopControl)override;

    void                DrawItemText(HANDLE_DC hDC, const RECT &rcItem);
    void                DrawItemBk(HANDLE_DC hDC, const RECT &rcItem);
    SIZE                EstimateSize(SIZE szAvailable);

protected:
    int             m_index;
    int             m_drawIndex;
    bool            m_selected;
    bool            m_expandable;
    bool            m_expand;
    uint32_t        m_buttonState;
    IListOwnerUI    *m_owner;
};

class UIListHBoxElement : public UIListContainerElement
{
public:
    UIListHBoxElement();

    UIString        GetClass()const override;
    LPVOID          GetInterface(const UIString &name)override;

    void            SetPos(RECT rc, bool needInvalidate=true)override;
    bool            DoPaint(HANDLE_DC  hDC, const RECT &rcPaint, UIControl *stopControl);
};

#endif //DIRECTUI_UILIST_H