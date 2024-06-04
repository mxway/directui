#include "UIFriends.h"
#include "UIButton.h"
#include <cmath>

const int kFriendListItemNormalHeight = 32;
const int kFriendListItemSelectedHeight = 50;

UIFriends::UIFriends(UIPaintManager &paint_manager)
        : root_node_(NULL)
        , delay_deltaY_(0)
        , delay_number_(0)
        , delay_left_(0)
        , level_expand_image_("<i list_icon_b.png>")
        , level_collapse_image_("<i list_icon_a.png>")
        , level_text_start_pos_(10)
        , text_padding_(10, 0, 0, 0)
        , paint_manager_(paint_manager)
        , m_timerId{0}
{
    SetItemShowHtml(true);

    root_node_ = new Node;
    root_node_->data().level_ = -1;
    root_node_->data().child_visible_ = true;
    root_node_->data().has_child_ = true;
    root_node_->data().list_elment_ = NULL;
}

UIFriends::~UIFriends() {
    if (root_node_)
        delete root_node_;

    root_node_ = NULL;
}

bool UIFriends::Add(UIControl *pControl) {
    if (!pControl)
        return false;

    if (strcasecmp(pControl->GetClass().GetData(), DUI_CTR_LISTCONTAINERELEMENT) != 0)
        return false;

    return UIList::Add(pControl);
}

bool UIFriends::AddAt(UIControl *pControl, int iIndex) {
    if (!pControl)
        return false;

    if (strcasecmp(pControl->GetClass().GetData(), DUI_CTR_LISTCONTAINERELEMENT) != 0)
        return false;

    return UIList::AddAt(pControl, iIndex);
}

bool UIFriends::Remove(UIControl *pControl, bool bDoNotDestroy) {
    if (!pControl)
        return false;

    if (strcasecmp(pControl->GetClass().GetData(), DUI_CTR_LISTCONTAINERELEMENT) != 0)
        return false;

    if (reinterpret_cast<Node*>(static_cast<UIListContainerElement*>(pControl->GetInterface(UIString{DUI_CTR_LISTCONTAINERELEMENT}))->GetTag()) == NULL)
        return UIList::Remove(pControl, bDoNotDestroy);
    else
        return RemoveNode(reinterpret_cast<Node*>(static_cast<UIListContainerElement*>(pControl->GetInterface(UIString{DUI_CTR_LISTCONTAINERELEMENT}))->GetTag()));
}

bool UIFriends::RemoveAt(int iIndex, bool bDoNotDestroy) {
    UIControl* pControl = GetItemAt(iIndex);
    if (!pControl)
        return false;

    if (strcasecmp(pControl->GetClass().GetData(), DUI_CTR_LISTCONTAINERELEMENT) != 0)
        return false;

    if (reinterpret_cast<Node*>(static_cast<UIListContainerElement*>(pControl->GetInterface(UIString{DUI_CTR_LISTCONTAINERELEMENT}))->GetTag()) == NULL)
        return UIList::RemoveAt(iIndex, bDoNotDestroy);
    else
        return RemoveNode(reinterpret_cast<Node*>(static_cast<UIListContainerElement*>(pControl->GetInterface(UIString{DUI_CTR_LISTCONTAINERELEMENT}))->GetTag()));
}

void UIFriends::RemoveAll() {
    UIList::RemoveAll();
    for (int i = 0; i < root_node_->num_children(); ++i)
    {
        Node* child = root_node_->child(i);
        RemoveNode(child);
    }
    delete root_node_;

    root_node_ = new Node;
    root_node_->data().level_ = -1;
    root_node_->data().child_visible_ = true;
    root_node_->data().has_child_ = true;
    root_node_->data().list_elment_ = NULL;
}

void UIFriends::DoEvent(TEventUI &event) {
    if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND)
    {
        if (m_parent != NULL)
            m_parent->DoEvent(event);
        else
            UIVerticalLayout::DoEvent(event);
        return;
    }

    if (event.Type == UIEVENT_TIMER && event.wParam == SCROLL_TIMERID)
    {
        if (delay_left_ > 0)
        {
            --delay_left_;
            SIZE sz = GetScrollPos();
            LONG lDeltaY =  (LONG)(CalculateDelay((double)delay_left_ / delay_number_) * delay_deltaY_);
            if ((lDeltaY > 0 && sz.cy != 0)  || (lDeltaY < 0 && sz.cy != GetScrollRange().cy ))
            {
                sz.cy -= lDeltaY;
                SetScrollPos(sz);
                return;
            }
        }
        delay_deltaY_ = 0;
        delay_number_ = 0;
        delay_left_ = 0;
        if(m_timerId != 0){
            m_manager->KillTimer(this, m_timerId);
            m_timerId = 0;
        }
        return;
    }
    if (event.Type == UIEVENT_SCROLLWHEEL)
    {
        LONG lDeltaY = 0;
        if (delay_number_ > 0)
            lDeltaY =  (LONG)(CalculateDelay((double)delay_left_ / delay_number_) * delay_deltaY_);
        switch (LOWORD(event.wParam))
        {
            case SB_LINEUP:
                if (delay_deltaY_ >= 0)
                    delay_deltaY_ = lDeltaY + 8;
                else
                    delay_deltaY_ = lDeltaY + 12;
                break;
            case SB_LINEDOWN:
                if (delay_deltaY_ <= 0)
                    delay_deltaY_ = lDeltaY - 8;
                else
                    delay_deltaY_ = lDeltaY - 12;
                break;
        }
        if
                (delay_deltaY_ > 100) delay_deltaY_ = 100;
        else if
                (delay_deltaY_ < -100) delay_deltaY_ = -100;

        delay_number_ = (DWORD)sqrt((double)abs(delay_deltaY_)) * 5;
        delay_left_ = delay_number_;
        if(m_timerId == 0){
            m_timerId = m_manager->SetTimer(this, 50U);
        }
        //m_manager->SetTimer(this, SCROLL_TIMERID, 50U);
        return;
    }

    UIList::DoEvent(event);
}

Node *UIFriends::GetRoot() {
    return root_node_;
}

const char* const kLogoButtonControlName = "logo";
const char* const kLogoContainerControlName = "logo_container";
const char* const kNickNameControlName = "nickname";
const char* const kDescriptionControlName = "description";
const char* const kOperatorPannelControlName = "operation";

static bool OnLogoButtonEvent(void* event) {
    if( ((TEventUI*)event)->Type == UIEVENT_BUTTONDOWN ) {
        UIControl* pButton = ((TEventUI*)event)->pSender;
        if( pButton != NULL ) {
            UIListContainerElement* pListElement = (UIListContainerElement*)(pButton->GetTag());
            if( pListElement != NULL ) pListElement->DoEvent(*(TEventUI*)event);
        }
    }
    return true;
}

Node *UIFriends::AddNode(const FriendListItemInfo &item, Node *parent) {
    if (!parent)
        parent = root_node_;

    TCHAR szBuf[MAX_PATH] = {0};

    UIListContainerElement* pListElement = NULL;
    UIDlgBuilder    dlgBuilder;
    pListElement = dynamic_cast<UIListContainerElement*>(dlgBuilder.Create(
            UIString{"friend_list_item.xml"},
            UIDlgBuilder::SkinType::SkinType_XmlFile,
            nullptr,
            &paint_manager_));

#if 0
    if( !m_dlgBuilder.GetMarkup()->IsValid() ) {
        pListElement = static_cast<CListContainerElementUI*>(m_dlgBuilder.Create(_T("friend_list_item.xml"), (UINT)0, NULL, &paint_manager_));
    }
    else {
        pListElement = static_cast<CListContainerElementUI*>(m_dlgBuilder.Create((UINT)0, &paint_manager_));
    }
#endif
    if (pListElement == NULL)
        return NULL;

    Node* node = new Node;

    node->data().level_ = parent->data().level_ + 1;
    if (item.folder)
        node->data().has_child_ = !item.empty;
    else
        node->data().has_child_ = false;

    node->data().folder_ = item.folder;

    node->data().child_visible_ = (node->data().level_ == 0);
    node->data().child_visible_ = false;

    node->data().text_ = item.nick_name;
    node->data().value = item.id;
    node->data().list_elment_ = pListElement;

    if (!parent->data().child_visible_)
        pListElement->SetVisible(false);

    if (parent != root_node_ && !parent->data().list_elment_->IsVisible())
        pListElement->SetVisible(false);

    UIRect rcPadding = text_padding_;
    for (int i = 0; i < node->data().level_; ++i)
    {
        rcPadding.left += level_text_start_pos_;
    }
    pListElement->SetPadding(rcPadding);

    UIButton* log_button = static_cast<UIButton*>(paint_manager_.FindSubControlByName(pListElement, kLogoButtonControlName));
    if (log_button != NULL)
    {
        if (!item.folder && !item.logo.IsEmpty())
        {
#if defined(UNDER_WINCE)
            _stprintf(szBuf, _T("%s"), item.logo);
#else
            //_stprintf_s(szBuf, MAX_PATH - 1, _T("%s"), item.logo);
#endif
            log_button->SetNormalImage(item.logo);
        }
        else
        {
            auto* logo_container = dynamic_cast<UIContainer*>(paint_manager_.FindSubControlByName(pListElement, kLogoContainerControlName));
            if (logo_container != NULL)
                logo_container->SetVisible(false);
        }
        log_button->SetTag((PVOID)pListElement);
        log_button->OnEvent += MakeDelegate(&OnLogoButtonEvent);
    }

    UIString html_text;
    if (node->data().has_child_)
    {
        if (node->data().child_visible_)
            html_text += level_expand_image_;
        else
            html_text += level_collapse_image_;

#if defined(UNDER_WINCE)
        _stprintf(szBuf, _T("<x %d>"), level_text_start_pos_);
#else
        //_stprintf_s(szBuf, MAX_PATH - 1, _T("<x %d>"), level_text_start_pos_);
        snprintf(szBuf, MAX_PATH - 1, "<x %d>", level_text_start_pos_);
#endif
        html_text += szBuf;
    }

    if (item.folder)
    {
        html_text += node->data().text_;
    }
    else
    {
#if defined(UNDER_WINCE)
        _stprintf(szBuf, _T("%s"), item.nick_name);
#else
        //_stprintf_s(szBuf, MAX_PATH - 1, _T("%s"), item.nick_name);
        snprintf(szBuf, MAX_PATH - 1, "%s", item.nick_name.GetData());
#endif
        html_text += szBuf;
    }

    auto* nick_name = dynamic_cast<UILabel*>(paint_manager_.FindSubControlByName(pListElement, kNickNameControlName));
    if (nick_name != NULL)
    {
        if (item.folder)
            nick_name->SetFixedWidth(0);

        nick_name->SetShowHtml(true);
        nick_name->SetText(html_text);
    }

    if (!item.folder && !item.description.IsEmpty())
    {
        auto* description = dynamic_cast<UILabel*>(paint_manager_.FindSubControlByName(pListElement, kDescriptionControlName));
        if (description != NULL)
        {
#if defined(UNDER_WINCE)
            _stprintf(szBuf, _T("<x 20><c #808080>%s</c>"), item.description);
#else
            //_stprintf_s(szBuf, MAX_PATH - 1, _T("<x 20><c #808080>%s</c>"), item.description);
            snprintf(szBuf, MAX_PATH - 1, "<x 20><c #808080>%s</c>", item.description.GetData());
#endif
            description->SetShowHtml(true);
            description->SetText(UIString{szBuf});
        }
    }

    pListElement->SetFixedHeight(kFriendListItemNormalHeight);
    pListElement->SetTag((LPVOID)node);
    int index = 0;
    if (parent->has_children())
    {
        Node* prev = parent->get_last_child();
        index = prev->data().list_elment_->GetIndex() + 1;
    }
    else
    {
        if (parent == root_node_)
            index = 0;
        else
            index = parent->data().list_elment_->GetIndex() + 1;
    }
    if (!UIList::AddAt(pListElement, index))
    {
        delete pListElement;
        delete node;
        node = NULL;
    }

    parent->add_child(node);
    return node;
}

bool UIFriends::RemoveNode(Node *node) {
    if (!node || node == root_node_) return false;

    for (int i = 0; i < node->num_children(); ++i)
    {
        Node* child = node->child(i);
        RemoveNode(child);
    }

    UIList::Remove(node->data().list_elment_);
    node->parent()->remove_child(node);
    delete node;

    return true;
}

void UIFriends::SetChildVisible(Node *node, bool visible) {
    if (!node || node == root_node_)
        return;

    if (node->data().child_visible_ == visible)
        return;

    node->data().child_visible_ = visible;

    TCHAR szBuf[MAX_PATH] = {0};
    UIString html_text;
    if (node->data().has_child_)
    {
        if (node->data().child_visible_)
            html_text += level_expand_image_;
        else
            html_text += level_collapse_image_;

#if defined(UNDER_WINCE)
        _stprintf(szBuf, _T("<x %d>"), level_text_start_pos_);
#else
        //_stprintf_s(szBuf, MAX_PATH - 1, _T("<x %d>"), level_text_start_pos_);
        snprintf(szBuf, MAX_PATH - 1, "<x %d>", level_text_start_pos_);
#endif
        html_text += szBuf;

        html_text += node->data().text_;

        auto* nick_name = dynamic_cast<UILabel*>(paint_manager_.FindSubControlByName(node->data().list_elment_, kNickNameControlName));
        if (nick_name != NULL)
        {
            nick_name->SetShowHtml(true);
            nick_name->SetText(html_text);
        }
    }

    if (!node->data().list_elment_->IsVisible())
        return;

    if (!node->has_children())
        return;

    Node* begin = node->child(0);
    Node* end = node->get_last_child();
    for (int i = begin->data().list_elment_->GetIndex(); i <= end->data().list_elment_->GetIndex(); ++i)
    {
        UIControl* control = GetItemAt(i);
        if (strcasecmp(control->GetClass().GetData(), DUI_CTR_LISTCONTAINERELEMENT) == 0)
        {
            if (visible)
            {
                Node* local_parent = ((Node*)control->GetTag())->parent();
                if (local_parent->data().child_visible_ && local_parent->data().list_elment_->IsVisible())
                {
                    control->SetVisible(true);
                }
            }
            else
            {
                control->SetVisible(false);
            }
        }
    }
}

bool UIFriends::CanExpand(Node *node) const {
    if (!node || node == root_node_)
        return false;

    return node->data().has_child_;
}

bool UIFriends::SelectItem(int iIndex, bool bTakeFocus) {
    if( iIndex == m_curSel ) return true;

    // We should first unselect the currently selected item
    if( m_curSel >= 0 ) {
        UIControl* pControl = GetItemAt(m_curSel);
        if( pControl != NULL) {
            IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(UIString{DUI_CTR_ILISTITEM}));
            if( pListItem != NULL )
            {
                auto* pFriendListItem = dynamic_cast<UIListContainerElement*>(pControl);
                Node* node = (Node*)pControl->GetTag();
                if ((pFriendListItem != NULL) && (node != NULL) && !node->folder())
                {
                    pFriendListItem->SetFixedHeight(kFriendListItemNormalHeight);
                    auto* pOperatorPannel = dynamic_cast<UIContainer*>(paint_manager_.FindSubControlByName(pFriendListItem, kOperatorPannelControlName));
                    if (pOperatorPannel != NULL)
                    {
                        pOperatorPannel->SetVisible(false);
                    }
                }
                pListItem->Select(false);
            }
        }

        m_curSel = -1;
    }

    if( iIndex < 0 )
        return false;

    if (!UIList::SelectItem(iIndex, bTakeFocus))
        return false;


    UIControl* pControl = GetItemAt(m_curSel);
    if( pControl != nullptr) {
        auto* pFriendListItem = dynamic_cast<UIListContainerElement*>(pControl);
        Node* node = (Node*)pControl->GetTag();
        if ((pFriendListItem != nullptr) && (node != nullptr) && !node->folder())
        {
            pFriendListItem->SetFixedHeight(kFriendListItemSelectedHeight);
            auto* pOperatorPannel = dynamic_cast<UIContainer*>(paint_manager_.FindSubControlByName(pFriendListItem, kOperatorPannelControlName));
            if (pOperatorPannel != NULL)
            {
                pOperatorPannel->SetVisible(true);
            }
        }
    }
    return true;
}
