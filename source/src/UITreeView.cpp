#include <UITreeView.h>
#include <UICheckbox.h>
#include <UILabel.h>
#include <UIOption.h>
#include <UIPaintManager.h>
#include <UIScrollBar.h>
#include <UIRect.h>

UITreeNode::UITreeNode(UITreeNode *parentNode)
    :m_treeLevel{0},
    m_itemTextColor{0},
    m_itemHotTextColor{0},
    m_selectedItemTextColor{0},
    m_selectedItemHotTextColor{0},
    m_treeView {nullptr},
    m_isVisible{true},
    m_isCheckBox{false},
    m_parentTreeNode {nullptr},
    m_horizontalLayout{new UIHorizontalLayout},
    m_folderButton{new UICheckbox},
    m_dottedLine{new UILabel},
    m_checkBox{new UICheckbox},
    m_itemButton { new UIOption}
{
    this->SetFixedHeight(18);
    this->SetFixedWidth(250);
    m_folderButton->SetFixedWidth(GetFixedHeight());
    m_dottedLine->SetFixedWidth(2);
    m_checkBox->SetFixedWidth(GetFixedHeight());
    m_itemButton->SetAttribute("align", "left");
    m_dottedLine->SetVisible(false);
    m_checkBox->SetVisible(false);
    m_itemButton->SetMouseEnabled(false);
    if(parentNode)
    {
        if (strcasecmp(parentNode->GetClass().GetData(), DUI_CTR_TREENODE) != 0){
            return;
        }

        m_dottedLine->SetVisible(parentNode->IsVisible());
        m_dottedLine->SetFixedWidth(parentNode->GetDottedLine()->GetFixedWidth()+16);
        this->SetParentNode(parentNode);
    }

    m_horizontalLayout->Add(m_dottedLine);
    //m_horizontalLayout->Add(pDottedLine);
    m_horizontalLayout->Add(m_folderButton);
    m_horizontalLayout->Add(m_checkBox);
    m_horizontalLayout->Add(m_itemButton);
    Add(m_horizontalLayout);
}

UITreeNode::~UITreeNode() {

}

UIString UITreeNode::GetClass() const {
    return UIString{DUI_CTR_TREENODE};
}

LPVOID UITreeNode::GetInterface(const UIString &name) {
    if(name == DUI_CTR_TREENODE){
        return static_cast<UITreeNode*>(this);
    }
    return UIListContainerElement::GetInterface(name);
}

void UITreeNode::DoEvent(TEventUI &event) {
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_owner != NULL ) m_owner->DoEvent(event);
        else UIContainer::DoEvent(event);
        return;
    }

    UIListContainerElement::DoEvent(event);

    if( event.Type == UIEVENT_DBLCLICK )
    {
        if( IsEnabled() ) {
            m_manager->SendNotify(this, DUI_MSGTYPE_ITEMDBCLICK);
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_MOUSEENTER )
    {
        if( IsEnabled()) {
            if(m_selected && GetSelectedItemHotTextColor())
                m_itemButton->SetTextColor(GetSelectedItemHotTextColor());
            else
                m_itemButton->SetTextColor(GetItemHotTextColor());
        }
        else
            m_itemButton->SetTextColor(m_itemButton->GetDisabledTextColor());

        //return;
    }
    if( event.Type == UIEVENT_MOUSELEAVE )
    {
        if( IsEnabled()) {
            if(m_selected && GetSelectedItemTextColor())
                m_itemButton->SetTextColor(GetSelectedItemTextColor());
            else if(!m_selected)
                m_itemButton->SetTextColor(GetItemTextColor());
        }
        else
            m_itemButton->SetTextColor(m_itemButton->GetDisabledTextColor());

        //return;
    }
}

void UITreeNode::Invalidate() {
    if( !IsVisible() )
        return;

    if( GetParent() ) {
        auto* pParentContainer = static_cast<UIContainer*>(GetParent()->GetInterface(UIString{DUI_CTR_CONTAINER}));
        if( pParentContainer ) {
            RECT rc = pParentContainer->GetPos();
            RECT rcInset = pParentContainer->GetInset();
            rc.left += rcInset.left;
            rc.top += rcInset.top;
            rc.right -= rcInset.right;
            rc.bottom -= rcInset.bottom;
            UIScrollBar* pVerticalScrollBar = pParentContainer->GetVerticalScrollBar();
            if( pVerticalScrollBar && pVerticalScrollBar->IsVisible() ) rc.right -= pVerticalScrollBar->GetFixedWidth();
            UIScrollBar* pHorizontalScrollBar = pParentContainer->GetHorizontalScrollBar();
            if( pHorizontalScrollBar && pHorizontalScrollBar->IsVisible() ) rc.bottom -= pHorizontalScrollBar->GetFixedHeight();

            RECT invalidateRc = m_rcItem;
            if( !::UIIntersectRect(&invalidateRc, &m_rcItem, &rc) )
                return;

            UIControl* pParent = GetParent();
            RECT rcTemp;
            RECT rcParent;
            while( pParent = pParent->GetParent() )
            {
                rcTemp = invalidateRc;
                rcParent = pParent->GetPos();
                if( !::UIIntersectRect(&invalidateRc, &rcTemp, &rcParent) )
                    return;
            }

            if( m_manager != nullptr ) m_manager->Invalidate(invalidateRc);
        }
        else {
            UIContainer::Invalidate();
        }
    }
    else {
        UIContainer::Invalidate();
    }
}

bool UITreeNode::Select(bool bSelect, bool bTriggerEvent) {
    bool nRet = UIListContainerElement::Select(bSelect, bTriggerEvent);
    if(m_selected)
        m_itemButton->SetTextColor(GetSelectedItemTextColor());
    else
        m_itemButton->SetTextColor(GetItemTextColor());

    return nRet;
}

bool UITreeNode::Add(UIControl *treeNodeUI) {
    if (treeNodeUI->GetClass() == DUI_CTR_TREENODE)
        return AddChildNode((UITreeNode*)treeNodeUI);

    return UIListContainerElement::Add(treeNodeUI);
}

bool UITreeNode::AddAt(UIControl *control, int iIndex) {
    if(nullptr == static_cast<UITreeNode*>(control->GetInterface(UIString{DUI_CTR_TREENODE})))
        return false;

    auto* pIndexNode = static_cast<UITreeNode*>(m_treeNodes.GetAt(iIndex));
    if(!pIndexNode){
        if(!m_treeNodes.Add(control))
            return false;
    }
    else if(pIndexNode && !m_treeNodes.InsertAt(iIndex,control))
        return false;

    if(!pIndexNode && m_treeView && m_treeView->GetItemAt(GetTreeIndex()+1))
        pIndexNode = static_cast<UITreeNode*>(m_treeView->GetItemAt(GetTreeIndex()+1)->GetInterface(UIString{DUI_CTR_TREENODE}));

    control = CalLocation((UITreeNode*)control);

    if(m_treeView && pIndexNode)
        return m_treeView->AddAt((UITreeNode*)control,pIndexNode);
    else
        return m_treeView->Add((UITreeNode*)control);
}

void UITreeNode::SetVisibleTag(bool isVisible) {
    m_isVisible = isVisible;
}

bool UITreeNode::GetVisibleTag() {
    return m_isVisible;
}

void UITreeNode::SetItemText(const UIString &text) {
    m_itemButton->SetText(text);
}

UIString UITreeNode::GetItemText() const {
    return m_itemButton->GetText();
}

void UITreeNode::CheckBoxSelected(bool selected) {
    m_checkBox->Selected(selected);
}

bool UITreeNode::IsCheckBoxSelected() const {
    return m_checkBox->IsSelected();
}

bool UITreeNode::HasChild() const {
    return !m_treeNodes.IsEmpty();
}

/*long UITreeNode::GetTreeLevel() const {
    return 0;
}*/

bool UITreeNode::AddChildNode(UITreeNode *treeNodeUI) {
    if (!treeNodeUI)
        return false;

    if (strcasecmp(treeNodeUI->GetClass().GetData(), DUI_CTR_TREENODE) != 0)
        return false;

    treeNodeUI = CalLocation(treeNodeUI);

    bool nRet = true;

    if(m_treeView){
        auto* pNode = static_cast<UITreeNode*>(m_treeNodes.GetAt(m_treeNodes.GetSize()-1));
        nRet = m_treeView->AddAt(treeNodeUI, GetTreeIndex() + 1) >= 0;
    }

    if(nRet)
        m_treeNodes.Add(treeNodeUI);

    return nRet;
}

bool UITreeNode::RemoveAt(UITreeNode *treeNodeUI) {
    int nIndex = m_treeNodes.Find(treeNodeUI);
    UITreeNode* pNode = static_cast<UITreeNode*>(m_treeNodes.GetAt(nIndex));
    if(pNode && pNode == treeNodeUI)
    {
        while(pNode->HasChild())
            pNode->RemoveAt(static_cast<UITreeNode*>(pNode->m_treeNodes.GetAt(0)));

        m_treeNodes.Remove(nIndex);

        if(m_treeView)
            m_treeView->Remove(treeNodeUI);

        return true;
    }
    return false;
}

void UITreeNode::SetParentNode(UITreeNode *parentTreeNode) {
    m_parentTreeNode = parentTreeNode;
}

UITreeNode *UITreeNode::GetParentNode() const {
    return m_parentTreeNode;
}

long UITreeNode::GetCountChild() {
    return m_treeNodes.GetSize();
}

void UITreeNode::SetTreeView(UITreeView *treeView) {
    m_treeView = treeView;
}

UITreeView *UITreeNode::GetTreeView() const {
    return m_treeView;
}

UITreeNode *UITreeNode::GetChildNode(int index) {
    return static_cast<UITreeNode*>(m_treeNodes.GetAt(index));
}

void UITreeNode::SetVisibleFolderBtn(bool visible) {
    m_folderButton->SetVisible(visible);
}

bool UITreeNode::GetVisibleFolderBtn() const {
    return m_folderButton->IsVisible();
}

void UITreeNode::SetVisibleCheckBtn(bool visible) {
    m_checkBox->SetVisible(visible);
}

bool UITreeNode::GetVisibleCheckBtn() const {
    return m_checkBox->IsVisible();
}

void UITreeNode::SetItemTextColor(uint32_t itemTextColor) {
    m_itemTextColor	= itemTextColor;
    m_itemButton->SetTextColor(m_itemTextColor);
}

uint32_t UITreeNode::GetItemTextColor() const {
    return m_itemTextColor;
}

void UITreeNode::SetItemHotTextColor(uint32_t itemHotTextColor) {
    m_itemHotTextColor = itemHotTextColor;
    Invalidate();
}

uint32_t UITreeNode::GetItemHotTextColor() const {
    return m_itemHotTextColor;
}

void UITreeNode::SetSelectedItemTextColor(uint32_t selectedItemTextColor) {
    m_selectedItemTextColor = selectedItemTextColor;
    Invalidate();
}

uint32_t UITreeNode::GetSelectedItemTextColor() const {
    return m_selectedItemTextColor;
}

void UITreeNode::SetSelectedItemHotTextColor(uint32_t selectedItemHotTextColor) {
    m_selectedItemHotTextColor = selectedItemHotTextColor;
    Invalidate();
}

uint32_t UITreeNode::GetSelectedItemHotTextColor() const {
    return m_selectedItemHotTextColor;
}

void UITreeNode::SetAttribute(const char *pstrName, const char *pstrValue) {
    if(strcasecmp(pstrName, "text") == 0 )
        m_itemButton->SetText(UIString{pstrValue});
    else if(strcasecmp(pstrName, "horizattr") == 0 )
        m_horizontalLayout->SetAttributeList(pstrValue);
    else if(strcasecmp(pstrName, "dotlineattr") == 0 )
        m_dottedLine->SetAttributeList(pstrValue);
    else if(strcasecmp(pstrName, "folderattr") == 0 )
        m_folderButton->SetAttributeList(pstrValue);
    else if(strcasecmp(pstrName, "checkboxattr") == 0 )
        m_checkBox->SetAttributeList(pstrValue);
    else if(strcasecmp(pstrName, "itemattr") == 0 )
        m_itemButton->SetAttributeList(pstrValue);
    else if(strcasecmp(pstrName, "itemtextcolor") == 0 ){
        if( *pstrValue == '#') pstrValue = ::CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetItemTextColor(clrColor);
    }
    else if(strcasecmp(pstrName, "itemhottextcolor") == 0 ){
        if( *pstrValue == '#') pstrValue = ::CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetItemHotTextColor(clrColor);
    }
    else if(strcasecmp(pstrName, "selitemtextcolor") == 0 ){
        if( *pstrValue == '#') pstrValue = ::CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetSelectedItemTextColor(clrColor);
    }
    else if(strcasecmp(pstrName, "selitemhottextcolor") == 0 ){
        if( *pstrValue == '#') pstrValue = ::CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetSelectedItemHotTextColor(clrColor);
    }
    else UIListContainerElement::SetAttribute(pstrName,pstrValue);
}

UIPtrArray UITreeNode::GetTreeNodes() {
    return m_treeNodes;
}

int UITreeNode::GetTreeIndex() {
    if(!m_treeView)
        return -1;

    for(int nIndex = 0;nIndex < m_treeView->GetCount();nIndex++){
        if(this == m_treeView->GetItemAt(nIndex))
            return nIndex;
    }
    return -1;
}

int UITreeNode::GetNodeIndex() {
    if(!GetParentNode() && !m_treeView)
        return -1;

    if(!GetParentNode() && m_treeView)
        return GetTreeIndex();

    return GetParentNode()->GetTreeNodes().Find(this);
}

UITreeNode *UITreeNode::GetLastNode() {
    if(!HasChild())
        return this;

    UITreeNode* nRetNode = nullptr;

    for(int nIndex = 0;nIndex < GetTreeNodes().GetSize();nIndex++){
        UITreeNode* pNode = static_cast<UITreeNode*>(GetTreeNodes().GetAt(nIndex));
        if(!pNode)
            continue;

        UIString aa = pNode->GetItemText();

        if(pNode->HasChild())
            nRetNode = pNode->GetLastNode();
        else
            nRetNode = pNode;
    }

    return nRetNode;
}

UITreeNode *UITreeNode::CalLocation(UITreeNode *treeNode) {
    treeNode->GetDottedLine()->SetVisible(true);
    treeNode->GetDottedLine()->SetFixedWidth(m_dottedLine->GetFixedWidth()+16);
    treeNode->SetParentNode(this);
    treeNode->GetItemButton()->SetGroup(m_itemButton->GetGroup());
    treeNode->SetTreeView(m_treeView);

    return treeNode;
}

UITreeView::UITreeView()
    :m_visibleFolderBtn{true},
    m_visibleCheckBtn{false},
    m_itemMinWidth{0}
{
    this->GetHeader()->SetVisible(false);
}

UITreeView::~UITreeView() {

}

UIString UITreeView::GetClass() const {
    return UIString{DUI_CTR_TREEVIEW};
}

LPVOID UITreeView::GetInterface(const UIString &name) {
    if(name == DUI_CTR_TREEVIEW){
        return static_cast<UITreeView*>(this);
    }
    return UIList::GetInterface(name);
}

bool UITreeView::Add(UIControl *control) {
    if (!control) return false;

    UITreeNode* pTreeNode = static_cast<UITreeNode*>(control->GetInterface(UIString{DUI_CTR_TREENODE}));
    if (pTreeNode == NULL) return false;

    pTreeNode->OnNotify += MakeDelegate(this,&UITreeView::OnDBClickItem);
    pTreeNode->GetFolderButton()->OnNotify += MakeDelegate(this,&UITreeView::OnFolderChanged);
    pTreeNode->GetCheckBox()->OnNotify += MakeDelegate(this,&UITreeView::OnCheckBoxChanged);

    pTreeNode->SetVisibleFolderBtn(m_visibleFolderBtn);
    pTreeNode->SetVisibleCheckBtn(m_visibleCheckBtn);
    if(m_itemMinWidth > 0)	pTreeNode->SetMinWidth(m_itemMinWidth);

    UIList::Add(pTreeNode);

    if(pTreeNode->GetCountChild() > 0)
    {
        int nCount = pTreeNode->GetCountChild();
        for(int nIndex = 0;nIndex < nCount;nIndex++)
        {
            UITreeNode* pNode = pTreeNode->GetChildNode(nIndex);
            if(pNode) Add(pNode);
        }
    }

    pTreeNode->SetTreeView(this);
    return true;
}

bool UITreeView::AddAt(UIControl *control, int iIndex) {
    if (!control) return false;

    UITreeNode* pTreeNode = static_cast<UITreeNode*>(control->GetInterface(UIString{DUI_CTR_TREENODE}));
    if (pTreeNode == nullptr) return false;
    return AddAt(pTreeNode, iIndex) >= 0;
}

bool UITreeView::Remove(UIControl *control, bool bDoNotDestroy) {
    if (!control) return false;

    UITreeNode* pTreeNode = static_cast<UITreeNode*>(control->GetInterface(UIString{DUI_CTR_TREENODE}));
    if (pTreeNode == NULL) return UIList::Remove(control, bDoNotDestroy);

    if(pTreeNode->GetCountChild() > 0)
    {
        int nCount = pTreeNode->GetCountChild();
        for(int nIndex = 0;nIndex < nCount;nIndex++)
        {
            UITreeNode* pNode = pTreeNode->GetChildNode(nIndex);
            if(pNode){
                pTreeNode->Remove(pNode, true);
            }
        }
    }
    return UIList::Remove(control, bDoNotDestroy);
}

bool UITreeView::RemoveAt(int iIndex, bool bDoNotDestroy) {
    UIControl* pControl = GetItemAt(iIndex);
    if (pControl == NULL) return false;

    UITreeNode* pTreeNode = static_cast<UITreeNode*>(pControl->GetInterface(UIString{DUI_CTR_TREENODE}));
    if (pTreeNode == NULL) return UIList::Remove(pControl, bDoNotDestroy);

    return Remove(pTreeNode);
}

void UITreeView::RemoveAll() {
    UIList::RemoveAll();
}

long UITreeView::AddAt(UITreeNode *control, int index) {
    if (!control) return -1;

    UITreeNode* pTreeNode = static_cast<UITreeNode*>(control->GetInterface(UIString{DUI_CTR_TREENODE}));
    if (pTreeNode == NULL) return -1;

    UITreeNode* pParent = static_cast<UITreeNode*>(GetItemAt(index));
    if(!pParent) return -1;

    pTreeNode->OnNotify += MakeDelegate(this,&UITreeView::OnDBClickItem);
    pTreeNode->GetFolderButton()->OnNotify += MakeDelegate(this,&UITreeView::OnFolderChanged);
    pTreeNode->GetCheckBox()->OnNotify += MakeDelegate(this,&UITreeView::OnCheckBoxChanged);

    pTreeNode->SetVisibleFolderBtn(m_visibleFolderBtn);
    pTreeNode->SetVisibleCheckBtn(m_visibleCheckBtn);

    if(m_itemMinWidth > 0) pTreeNode->SetMinWidth(m_itemMinWidth);

    UIList::AddAt(pTreeNode,index);

    if(pTreeNode->GetCountChild() > 0)
    {
        int nCount = pTreeNode->GetCountChild();
        for(int nIndex = 0;nIndex < nCount;nIndex++)
        {
            UITreeNode* pNode = pTreeNode->GetChildNode(nIndex);
            if(pNode)
                return AddAt(pNode,index+1);
        }
    }
    else
        return index+1;

    return -1;
}

bool UITreeView::AddAt(UITreeNode *control, UITreeNode *indexNode) {
    if(!indexNode && !control)
        return false;

    int nItemIndex = -1;

    for(int nIndex = 0;nIndex < GetCount();nIndex++){
        if(indexNode == GetItemAt(nIndex)){
            nItemIndex = nIndex;
            break;
        }
    }

    if(nItemIndex == -1)
        return false;

    return AddAt(control,nItemIndex) >= 0;
}

bool UITreeView::OnCheckBoxChanged(void *param) {
    TNotifyUI* pMsg = (TNotifyUI*)param;
    if(pMsg->sType == DUI_MSGTYPE_SELECTCHANGED)
    {
        UICheckbox* pCheckBox = (UICheckbox*)pMsg->pSender;
        UITreeNode* pItem = (UITreeNode*)pCheckBox->GetParent()->GetParent();
        SetItemCheckBox(pCheckBox->GetCheck(),pItem);
        return true;
    }
    return true;
}

bool UITreeView::OnFolderChanged(void *param) {
    TNotifyUI* pMsg = (TNotifyUI*)param;
    if(pMsg->sType == DUI_MSGTYPE_SELECTCHANGED)
    {
        UICheckbox* pFolder = (UICheckbox*)pMsg->pSender;
        UITreeNode* pItem = (UITreeNode*)pFolder->GetParent()->GetParent();
        pItem->SetVisibleTag(!pFolder->GetCheck());
        SetItemExpand(!pFolder->GetCheck(),pItem);
        return true;
    }
    return true;
}

bool UITreeView::OnDBClickItem(void *param) {
    TNotifyUI* pMsg = (TNotifyUI*)param;
    if(pMsg->sType == DUI_MSGTYPE_ITEMDBCLICK)
    {
        UITreeNode* pItem		= static_cast<UITreeNode*>(pMsg->pSender);
        UICheckbox* pFolder	= pItem->GetFolderButton();
        pFolder->Selected(!pFolder->IsSelected());
        pItem->SetVisibleTag(!pFolder->GetCheck());
        SetItemExpand(!pFolder->GetCheck(),pItem);
        return true;
    }
    return false;
}

bool UITreeView::SetItemCheckBox(bool selected, UITreeNode *treeNode) {
    if(treeNode)
    {
        if(treeNode->GetCountChild() > 0)
        {
            int nCount = treeNode->GetCountChild();
            for(int nIndex = 0;nIndex < nCount;nIndex++)
            {
                UITreeNode* pItem = treeNode->GetChildNode(nIndex);
                pItem->GetCheckBox()->Selected(selected);
                if(pItem->GetCountChild())
                    SetItemCheckBox(selected,pItem);
            }
        }
    }
    else
    {
        int nIndex = 0;
        int nCount = GetCount();
        while(nIndex < nCount)
        {
            UITreeNode* pItem = (UITreeNode*)GetItemAt(nIndex);
            pItem->GetCheckBox()->Selected(selected);
            if(pItem->GetCountChild())
                SetItemCheckBox(selected,pItem);

            nIndex++;
        }
    }
    return true;
}

void UITreeView::SetItemExpand(bool expanded, UITreeNode *treeNode) {
    if(treeNode)
    {
        if(treeNode->GetCountChild() > 0)
        {
            int nCount = treeNode->GetCountChild();
            for(int nIndex = 0;nIndex < nCount;nIndex++)
            {
                UITreeNode* pItem = treeNode->GetChildNode(nIndex);
                pItem->SetVisible(expanded);

                if(pItem->GetCountChild() && !pItem->GetFolderButton()->IsSelected())
                    SetItemExpand(expanded,pItem);
            }
        }
    }
    else
    {
        int nIndex = 0;
        int nCount = GetCount();
        while(nIndex < nCount)
        {
            UITreeNode* pItem = (UITreeNode*)GetItemAt(nIndex);

            pItem->SetVisible(expanded);

            if(pItem->GetCountChild() && !pItem->GetFolderButton()->IsSelected())
                SetItemExpand(expanded,pItem);

            nIndex++;
        }
    }
}

void UITreeView::Notify(TNotifyUI &msg) {

}

void UITreeView::SetVisibleFolderBtn(bool visible) {
    m_visibleFolderBtn = visible;
    int nCount = this->GetCount();
    for(int nIndex = 0;nIndex < nCount;nIndex++)
    {
        auto* pItem = dynamic_cast<UITreeNode*>(this->GetItemAt(nIndex));
        pItem->GetFolderButton()->SetVisible(m_visibleFolderBtn);
    }
}

bool UITreeView::GetVisibleFolderBtn() {
    return m_visibleFolderBtn;
}

void UITreeView::SetVisibleCheckBtn(bool visible) {
    m_visibleCheckBtn = visible;
    int nCount = this->GetCount();
    for(int nIndex = 0;nIndex < nCount;nIndex++)
    {
        UITreeNode* pItem = static_cast<UITreeNode*>(this->GetItemAt(nIndex));
        pItem->GetCheckBox()->SetVisible(m_visibleCheckBtn);
    }
}

bool UITreeView::GetVisibleCheckBtn() {
    return m_visibleCheckBtn;
}

void UITreeView::SetItemMinWidth(uint32_t minWidth) {
    m_itemMinWidth = minWidth;

    for(int nIndex = 0;nIndex < GetCount();nIndex++){
        auto* pTreeNode = dynamic_cast<UITreeNode*>(GetItemAt(nIndex));
        if(pTreeNode)
            pTreeNode->SetMinWidth(GetItemMinWidth());
    }
    Invalidate();
}

uint32_t UITreeView::GetItemMinWidth() const {
    return m_itemMinWidth;
}

void UITreeView::SetItemTextColor(uint32_t itemTextColor) {
    for(int nIndex = 0;nIndex < GetCount();nIndex++){
        auto* pTreeNode = dynamic_cast<UITreeNode*>(GetItemAt(nIndex));
        if(pTreeNode)
            pTreeNode->SetItemTextColor(itemTextColor);
    }
}

void UITreeView::SetItemHotTextColor(uint32_t itemHotTextColor) {
    for(int nIndex = 0;nIndex < GetCount();nIndex++){
        auto* pTreeNode = dynamic_cast<UITreeNode*>(GetItemAt(nIndex));
        if(pTreeNode)
            pTreeNode->SetItemHotTextColor(itemHotTextColor);
    }
}

void UITreeView::SetSelectedItemTextColor(uint32_t selectedItemTextColor) {
    for(int nIndex = 0;nIndex < GetCount();nIndex++){
        auto* pTreeNode = dynamic_cast<UITreeNode*>(GetItemAt(nIndex));
        if(pTreeNode)
            pTreeNode->SetSelectedItemTextColor(selectedItemTextColor);
    }
}

void UITreeView::SetSelectedItemHotTextColor(uint32_t selectedItemHotTextColor) {
    for(int nIndex = 0;nIndex < GetCount();nIndex++){
        auto* pTreeNode = dynamic_cast<UITreeNode*>(GetItemAt(nIndex));
        if(pTreeNode)
            pTreeNode->SetSelectedItemHotTextColor(selectedItemHotTextColor);
    }
}

void UITreeView::SetAttribute(const char *pstrName, const char *pstrValue) {
    if(strcasecmp(pstrName,"visiblefolderbtn") == 0)
        SetVisibleFolderBtn(strcasecmp(pstrValue,"true") == 0);
    else if(strcasecmp(pstrName,"visiblecheckbtn") == 0)
        SetVisibleCheckBtn(strcasecmp(pstrValue,"true") == 0);
    else if(strcasecmp(pstrName,"itemminwidth") == 0)
        SetItemMinWidth(atoi(pstrValue));
    else if(strcasecmp(pstrName, "itemtextcolor") == 0 ){
        if( *pstrValue == '#') pstrValue = ::CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetItemTextColor(clrColor);
    }
    else if(strcasecmp(pstrName, "itemhottextcolor") == 0 ){
        if( *pstrValue == '#') pstrValue = ::CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetItemHotTextColor(clrColor);
    }
    else if(strcasecmp(pstrName, "selitemtextcolor") == 0 ){
        if( *pstrValue == '#') pstrValue = ::CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetSelectedItemTextColor(clrColor);
    }
    else if(strcasecmp(pstrName, "selitemhottextcolor") == 0 ){
        if( *pstrValue == '#') pstrValue = ::CharNext(pstrValue);
        char *pstr = nullptr;
        uint32_t clrColor = strtoul(pstrValue, &pstr, 16);
        SetSelectedItemHotTextColor(clrColor);
    }
    else UIList::SetAttribute(pstrName,pstrValue);
}
