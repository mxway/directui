#ifndef DIRECTUI_UICOMBO_H
#define DIRECTUI_UICOMBO_H
#include <UIContainer.h>
#include <UIList.h>

class UIComboWnd;

class UICombo : public UIContainer, public IListOwnerUI
{
    friend class UIComboWnd;
public:
    UICombo();

    UIString    GetClass() const override;

    LPVOID      GetInterface(const UIString &name) override;

    void        DoInit() override;

    uint32_t    GetControlFlags() const override;

    UIString    GetText() const override;

    void        SetEnabled(bool bEnabled) override;

    UIString    GetDropBoxAttributeList();
    void        SetDropBoxAttributeList(const char *pstrList);
    SIZE        GetDropBoxSize()const;
    void        SetDropBoxSize(SIZE szDropBox);

    int         GetCurSel()const;
    bool        GetSelectCloseFlag();
    void        SetSelectCloseFlag(bool flag);
    bool        SelectItem(int iIndex, bool bTakeFocus=false,bool bTriggerEvent=true);
    bool        ExpandItem(int iIndex, bool bExpand=true);
    int         GetExpandedItem()const;

    bool        SetItemIndex(UIControl *control, int iNewIndex);
    bool        SetMultiItemIndex(UIControl *startControl, int iCount, int iNewStartIndex);
    bool        Add(UIControl *control);
    bool        AddAt(UIControl *control, int iIndex);
    bool        Remove(UIControl *control, bool bDoNotDestroy=false);
    bool        RemoveAt(int iIndex, bool bDoNotDestroy=false);
    void        RemoveAll();
    bool        Activate();

    bool        GetShowText()const;
    void        SetShowText(bool flag);
    RECT        GetTextPadding()const;
    void        SetTextPadding(RECT rc);
    UIString    GetNormalImage()const;
    void        SetNormalImage(const UIString &normalImage);
    UIString    GetHotImage()const;
    void        SetHotImage(const UIString &hotImage);
    UIString    GetPushedImage()const;
    void        SetPushedImage(const UIString &pushedImage);
    UIString    GetFocusedImage()const;
    void        SetFocusedImage(const UIString &focusedImage);
    UIString    GetDisabledImage()const;
    void        SetDisabledImage(const UIString &disabledImage);

    TListInfoUI *GetListInfo();
    uint32_t    GetItemFixedHeight();
    void        SetItemFixedHeight(uint32_t nHeight);
    int         GetItemFont();
    void        SetItemFont(int index);
    uint32_t    GetItemTextStyle();
    void        SetItemTextStyle(uint32_t style);
    RECT        GetItemTextPadding()const;
    void        SetItemTextPadding(RECT rc);
    uint32_t    GetItemTextColor()const;
    void        SetItemTextColor(uint32_t color);
    uint32_t    GetItemBkColor()const;
    void        SetItemBkColor(uint32_t color);
    UIString    GetItemBkImage()const;
    void        SetItemBkImage(const UIString &bkImage);
    bool        IsAlternateBk()const;
    void        SetAlternateBk(bool bAlternateBk);
    uint32_t    GetSelectedItemTextColor()const;
    void        SetSelectedItemTextColor(uint32_t textColor);
    uint32_t    GetSelectedItemBkColor()const;
    void        SetSelectedItemBkColor(uint32_t bkColor);
    UIString    GetSelectedItemImage()const;
    void        SetSelectedItemImage(const UIString &selectedImage);
    uint32_t    GetHotItemTextColor()const;
    void        SetHotItemTextColor(uint32_t hotTextColor);
    uint32_t    GetHotItemBkColor()const;
    void        SetHotItemBkColor(uint32_t bkColor);
    UIString    GetHotItemImage()const;
    void        SetHotItemImage(const UIString &hotImage);
    uint32_t    GetDisabledItemTextColor()const;
    void        SetDisabledItemTextColor(uint32_t textColor);
    uint32_t    GetDisabledItemBkColor()const;
    void        SetDisabledItemBkColor(uint32_t bkColor);

    UIString    GetDisabledItemImage()const;
    void        SetDisabledItemImage(const UIString &disabledImage);
    int         GetItemHLineSize()const;
    void        SetItemHLineSize(int iSize);
    uint32_t    GetItemHLineColor()const;
    void        SetItemHLineColor(uint32_t lineColor);
    int         GetItemVLineSize()const;
    void        SetItemVLineSize(int iSize);
    uint32_t    GetItemVLineColor()const;
    void        SetItemVLineColor(uint32_t lineColor);
    bool        IsItemShowHtml();
    void        SetItemShowHtml(bool bShowHtml=true);

    SIZE        EstimateSize(SIZE szAvailable);
    void        SetPos(RECT rc, bool bNeedInvalidate=true)override;
    void        Move(SIZE szOffset, bool bNeedInvalidate=true)override;
    void        DoEvent(TEventUI &event)override;
    void        SetAttribute(const char *pstrName, const char *pstrValue)override;
    bool        DoPaint(HANDLE_DC hDC, const RECT &rcPaint,UIControl *stopControl)override;
    void        PaintText(HANDLE_DC hDC)override;
    void        PaintStatusImage(HANDLE_DC hDC)override;

protected:
    UIComboWnd  *m_pWindow;
    int         m_curSel;
    bool        m_showText;
    bool        m_selectCloseFlag;
    RECT        m_rcTextPadding;
    UIString    m_dropBoxAttributes;
    SIZE        m_dropBox;
    uint32_t    m_buttonState;
    TDrawInfo   m_diNormal;
    TDrawInfo   m_diHot;
    TDrawInfo   m_diPushed;
    TDrawInfo   m_diFocused;
    TDrawInfo   m_diDisabled;
    TListInfoUI m_listInfo;
};

#endif //DIRECTUI_UICOMBO_H