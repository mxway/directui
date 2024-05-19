#ifndef DIRECTUI_UICONTAINER_H
#define DIRECTUI_UICONTAINER_H
#include <UIContainer.h>
#include <IContainerUI.h>

class UIScrollBar;

class UIContainer : public UIControl, public IContainerUI
{
public:
    UIContainer();
    virtual ~UIContainer();

    UIString GetClass() const override;

    LPVOID GetInterface(const UIString &name) override;

    UIControl *GetItemAt(int iIndex) const override;

    int GetItemIndex(UIControl *pControl) const override;

    bool SetItemIndex(UIControl *pControl, int iNewIndex) override;

    bool SetMultiItemIndex(UIControl *pStartControl, int iCount, int iNewStartIndex) override;

    int GetCount() const override;

    bool Add(UIControl *pControl) override;

    bool AddAt(UIControl *pControl, int iIndex) override;

    bool Remove(UIControl *pControl, bool bDoNotDestroy=false) override;

    bool RemoveAt(int iIndex, bool bDoNotDestroy=false) override;

    void RemoveAll() override;

    void DoEvent(TEventUI& event)override;
    void SetVisible(bool bVisible = true)override;
    void SetInternVisible(bool bVisible = true)override;
    void SetMouseEnabled(bool bEnable = true)override;

    virtual RECT GetInset() const;
    virtual void SetInset(RECT rcInset); // 设置内边距，相当于设置客户区
    virtual int GetChildPadding() const;
    virtual void SetChildPadding(int iPadding);
    virtual uint32_t GetChildAlign() const;
    virtual void SetChildAlign(uint32_t iAlign);
    virtual uint32_t GetChildVAlign() const;
    virtual void SetChildVAlign(uint32_t iVAlign);
    virtual bool IsAutoDestroy() const;
    virtual void SetAutoDestroy(bool bAuto);
    virtual bool IsDelayedDestroy() const;
    virtual void SetDelayedDestroy(bool bDelayed);
    virtual bool IsMouseChildEnabled() const;
    virtual void SetMouseChildEnabled(bool bEnable = true);

    virtual int FindSelectable(int iIndex, bool bForward = true) const;

    RECT GetClientPos() const;
    void SetPos(RECT rc, bool bNeedInvalidate = true)override;
    void Move(SIZE szOffset, bool bNeedInvalidate = true)override;
    bool DoPaint(HANDLE_DC hDC, const RECT& rcPaint, UIControl* pStopControl)override;

    void SetAttribute(const char* pstrName, const char* pstrValue)override;

    void SetManager(UIPaintManager* pManager, UIControl* pParent, bool bInit = true)override;
    UIControl* FindControl(FindControlProc Proc, LPVOID pData, uint32_t uFlags)override;

    bool SetSubControlText(const char* pstrSubControlName,const char* pstrText);
    bool SetSubControlFixedHeight(const char* pstrSubControlName,int cy);
    bool SetSubControlFixedWidth(const char* pstrSubControlName, int cx);
    bool SetSubControlUserData(const char* pstrSubControlName,const char* pstrText);

    UIString GetSubControlText(const char* pstrSubControlName);
    int GetSubControlFixedHeight(const char* pstrSubControlName);
    int GetSubControlFixedWidth(const char* pstrSubControlName);
    const UIString GetSubControlUserData(const char* pstrSubControlName);
    UIControl* FindSubControl(const char* pstrSubControlName);

    virtual SIZE GetScrollPos() const;
    virtual SIZE GetScrollRange() const;
    virtual void SetScrollPos(SIZE szPos);
    virtual void LineUp();
    virtual void LineDown();
    virtual void PageUp();
    virtual void PageDown();
    virtual void HomeUp();
    virtual void EndDown();
    virtual void LineLeft();
    virtual void LineRight();
    virtual void PageLeft();
    virtual void PageRight();
    virtual void HomeLeft();
    virtual void EndRight();
    virtual void EnableScrollBar(bool bEnableVertical = true, bool bEnableHorizontal = false);
    virtual UIScrollBar* GetVerticalScrollBar() const;
    virtual UIScrollBar* GetHorizontalScrollBar() const;

protected:
    virtual void SetFloatPos(int iIndex);
    virtual void ProcessScrollBar(RECT rc, int cxRequired, int cyRequired);

protected:
    UIPtrArray m_items;
    RECT m_rcInset;
    int m_iChildPadding;
    uint32_t m_iChildAlign;
    uint32_t m_iChildVAlign;
    bool m_bAutoDestroy;
    bool m_bDelayedDestroy;
    bool m_bMouseChildEnabled;
    bool m_bScrollProcess; // 防止SetPos循环调用

    UIScrollBar* m_pVerticalScrollBar;
    UIScrollBar* m_pHorizontalScrollBar;
};

#endif //DIRECTUI_UICONTAINER_H