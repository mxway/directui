#ifndef DIRECTUI_UITREEVIEW_H
#define DIRECTUI_UITREEVIEW_H
#include "UIList.h"
#include "INotifyUI.h"

class UITreeView;
class UICheckbox;
class UILabel;
class UIOption;

class UITreeNode : public UIListContainerElement
{
public:
    UITreeNode(UITreeNode *parentNode = nullptr);
    ~UITreeNode();

public:
    UIString    GetClass() const override;
    LPVOID      GetInterface(const UIString &name) override;
    void        DoEvent(TEventUI &event) override;
    void        Invalidate()override;
    bool        Select(bool bSelect=true,bool bTriggerEvent=true)override;
    bool        Add(UIControl *treeNodeUI)override;
    bool        AddAt(UIControl *control, int iIndex)override;
    void        SetVisibleTag(bool isVisible);
    bool        GetVisibleTag();
    void        SetItemText(const UIString &text);
    UIString    GetItemText()const;
    void        CheckBoxSelected(bool selected);
    bool        IsCheckBoxSelected()const;
    bool        HasChild()const;
    //long        GetTreeLevel()const;
    bool        AddChildNode(UITreeNode *treeNodeUI);
    bool        RemoveAt(UITreeNode *treeNodeUI);
    void        SetParentNode(UITreeNode *parentTreeNode);
    UITreeNode  *GetParentNode()const;
    long        GetCountChild();
    void        SetTreeView(UITreeView *treeView);
    UITreeView  *GetTreeView()const;
    UITreeNode  *GetChildNode(int index);
    void        SetVisibleFolderBtn(bool visible);
    bool        GetVisibleFolderBtn()const;
    void        SetVisibleCheckBtn(bool visible);
    bool        GetVisibleCheckBtn()const;
    void        SetItemTextColor(uint32_t itemTextColor);
    uint32_t    GetItemTextColor()const;
    void        SetItemHotTextColor(uint32_t itemHotTextColor);
    uint32_t    GetItemHotTextColor()const;
    void        SetSelectedItemTextColor(uint32_t selectedItemTextColor);
    uint32_t    GetSelectedItemTextColor()const;
    void        SetSelectedItemHotTextColor(uint32_t selectedItemHotTextColor);
    uint32_t    GetSelectedItemHotTextColor()const;
    void        SetAttribute(const char *pstrName, const char *pstrValue)override;

    UIPtrArray  GetTreeNodes();
    int         GetTreeIndex();
    int         GetNodeIndex();
private:
    UITreeNode  *GetLastNode();
    UITreeNode  *CalLocation(UITreeNode *treeNode);
public:
    UIHorizontalLayout      *GetTreeNodeHorizontal()const{return m_horizontalLayout;}
    UICheckbox              *GetFolderButton() const {return m_folderButton;}
    UILabel                 *GetDottedLine()const {return m_dottedLine;}
    UICheckbox              *GetCheckBox()const{return m_checkBox;}
    UIOption                *GetItemButton()const{return m_itemButton;}
private:
    long        m_treeLevel;
    bool        m_isVisible;
    bool        m_isCheckBox;
    uint32_t    m_itemTextColor;
    uint32_t    m_itemHotTextColor;
    uint32_t    m_selectedItemTextColor;
    uint32_t    m_selectedItemHotTextColor;
    UITreeView  *m_treeView;
    UIHorizontalLayout  *m_horizontalLayout;
    UICheckbox          *m_folderButton;
    UILabel             *m_dottedLine;
    UICheckbox          *m_checkBox;
    UIOption            *m_itemButton;
    UITreeNode          *m_parentTreeNode;
    UIPtrArray          m_treeNodes;
};

class UITreeView : public UIList, public INotifyUI
{
public:
    UITreeView();
    ~UITreeView();
public:
    UIString    GetClass() const override;
    LPVOID      GetInterface(const UIString &name) override;
    bool        Add(UIControl *control) override;
    bool        AddAt(UIControl *control, int iIndex) override;
    bool        Remove(UIControl *control, bool bDoNotDestroy=false) override;
    bool        RemoveAt(int iIndex, bool bDoNotDestroy=false) override;
    void        RemoveAll() override;
    long        AddAt(UITreeNode *control, int index);
    bool        AddAt(UITreeNode *control, UITreeNode *indexNode);

    virtual     bool    OnCheckBoxChanged(void *param);
    virtual     bool    OnFolderChanged(void *param);
    virtual     bool    OnDBClickItem(void *param);
    virtual     bool    SetItemCheckBox(bool selected, UITreeNode *treeNode = nullptr);
    virtual     void    SetItemExpand(bool expanded, UITreeNode *treeNode = nullptr);
    void                Notify(TNotifyUI &msg)override;
    virtual     void    SetVisibleFolderBtn(bool visible);
    virtual     bool    GetVisibleFolderBtn();
    virtual     void    SetVisibleCheckBtn(bool visible);
    virtual     bool    GetVisibleCheckBtn();
    virtual     void    SetItemMinWidth(uint32_t minWidth);
    virtual   uint32_t  GetItemMinWidth()const;
    virtual     void    SetItemTextColor(uint32_t itemTextColor);
    virtual     void    SetItemHotTextColor(uint32_t itemHotTextColor);
    virtual     void    SetSelectedItemTextColor(uint32_t selectedItemTextColor);
    virtual     void    SetSelectedItemHotTextColor(uint32_t selectedItemHotTextColor);
    void                SetAttribute(const char *pstrName, const char *pstrValue)override;
private:
    uint32_t        m_itemMinWidth;
    bool            m_visibleFolderBtn;
    bool            m_visibleCheckBtn;
};


#endif //DIRECTUI_UITREEVIEW_H