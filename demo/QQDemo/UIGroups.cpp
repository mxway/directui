#include "UIGroups.h"
#include "../../include/UIButton.h"
#include <cmath>


const int kGroupListItemNormalHeight = 32;
const int kGroupListItemSelectedHeight = 48;

UIGroups::UIGroups(UIPaintManager& paint_manager)
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
    root_node_->data().list_elment_ = nullptr;
}

UIGroups::~UIGroups()
{
    if (root_node_)
        delete root_node_;

    root_node_ = NULL;
}

bool UIGroups::Add(UIControl* pControl)
{
    if (!pControl)
        return false;

    if(strcasecmp(pControl->GetClass().GetData(), DUI_CTR_LISTCONTAINERELEMENT)!=0)
        return false;

    return UIList::Add(pControl);
}

bool UIGroups::AddAt(UIControl* pControl, int iIndex)
{
    if (!pControl)
        return false;

    if(strcasecmp(pControl->GetClass().GetData(), DUI_CTR_LISTCONTAINERELEMENT)!=0)
        return false;

    return UIList::AddAt(pControl, iIndex);
}

bool UIGroups::Remove(UIControl* pControl, bool bDoNotDestroy)
{
    if (!pControl)
        return false;

    if(strcasecmp(pControl->GetClass().GetData(), DUI_CTR_LISTCONTAINERELEMENT)!=0)
        return false;

    if (reinterpret_cast<Node*>(static_cast<UIListContainerElement*>(pControl->GetInterface(UIString{DUI_CTR_LISTCONTAINERELEMENT}))->GetTag()) == NULL)
        return UIList::Remove(pControl, bDoNotDestroy);
    else
        return RemoveNode(reinterpret_cast<Node*>(static_cast<UIListContainerElement*>(pControl->GetInterface(UIString{DUI_CTR_LISTCONTAINERELEMENT}))->GetTag()));
}

bool UIGroups::RemoveAt(int iIndex, bool bDoNotDestroy)
{
    UIControl* pControl = GetItemAt(iIndex);
    if (!pControl)
        return false;

    if(strcasecmp(pControl->GetClass().GetData(), DUI_CTR_LISTCONTAINERELEMENT)!=0)
        return false;

    if (reinterpret_cast<Node*>(static_cast<UIListContainerElement*>(pControl->GetInterface(UIString{DUI_CTR_LISTCONTAINERELEMENT}))->GetTag()) == NULL)
        return UIList::RemoveAt(iIndex, bDoNotDestroy);
    else
        return RemoveNode(reinterpret_cast<Node*>(static_cast<UIListContainerElement*>(pControl->GetInterface(UIString{DUI_CTR_LISTCONTAINERELEMENT}))->GetTag()));
}

void UIGroups::RemoveAll()
{
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

void UIGroups::DoEvent(TEventUI& event)
{
    if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND)
    {
        if (m_parent != NULL)
            m_parent->DoEvent(event);
        else
            UIVerticalLayout::DoEvent(event);
        return;
    }

    if (event.Type == UIEVENT_TIMER)
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

        return;
    }

    UIList::DoEvent(event);
}

Node* UIGroups::GetRoot()
{
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
            auto* pListElement = (UIListContainerElement*)(pButton->GetTag());
            if( pListElement != NULL ) pListElement->DoEvent(*(TEventUI*)event);
        }
    }
    return true;
}

Node* UIGroups::AddNode(const GroupsListItemInfo& item, Node* parent)
{
    if (!parent)
        parent = root_node_;

    TCHAR szBuf[MAX_PATH] = {0};

    UIListContainerElement* pListElement = NULL;
    UIDlgBuilder    dlgBuilder;
    pListElement = dynamic_cast<UIListContainerElement*>(dlgBuilder.Create(
            UIString{"group_list_item.xml"},
            UIDlgBuilder::SkinType::SkinType_XmlFile,
            nullptr,
            &paint_manager_));
    /*if( !m_dlgBuilder.GetMarkup()->IsValid() ) {
        pListElement = static_cast<CListContainerElementUI*>(m_dlgBuilder.Create(_T("group_list_item.xml"), (UINT)0, NULL, &paint_manager_));
    }
    else {
        pListElement = static_cast<CListContainerElementUI*>(m_dlgBuilder.Create((UINT)0, &paint_manager_));
    }*/
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

    node->data().text_ = item.nick_name;
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
            //snprintf(szBuf, MAX_PATH-1, "%s",item.logo);
#endif
            log_button->SetNormalImage(item.logo);
        }
        else
        {
            UIContainer* logo_container = static_cast<UIContainer*>(paint_manager_.FindSubControlByName(pListElement, kLogoContainerControlName));
            if (logo_container != NULL)
                logo_container->SetVisible(false);
        }
        log_button->SetTag((LPVOID)pListElement);
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

    auto* nick_name = static_cast<UILabel*>(paint_manager_.FindSubControlByName(pListElement, kNickNameControlName));
    if (nick_name != NULL)
    {
        if (item.folder)
            nick_name->SetFixedWidth(0);

        nick_name->SetShowHtml(true);
        nick_name->SetText(html_text);
    }

    if (!item.folder && !item.description.IsEmpty())
    {
        auto* description = static_cast<UILabel*>(paint_manager_.FindSubControlByName(pListElement, kDescriptionControlName));
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

    pListElement->SetFixedHeight(kGroupListItemNormalHeight);
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

bool UIGroups::RemoveNode(Node* node)
{
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

void UIGroups::SetChildVisible(Node* node, bool visible)
{
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
        if(strcasecmp(control->GetClass().GetData(), DUI_CTR_LISTCONTAINERELEMENT)==0)
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

bool UIGroups::CanExpand(Node* node) const
{
    if (!node || node == root_node_) return false;

    return node->data().has_child_;
}