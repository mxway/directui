#ifndef DIRECTUI_CONTROLEX_H
#define DIRECTUI_CONTROLEX_H

#include <utility>
#include <vector>
#include <cmath>
#include <string>
#include "UIList.h"
#include "UIDlgBuilder.h"
#include "UITileLayout.h"

inline double CalculateDelay(double state) {
    return pow(state, 2);
}

// category(0)->game(1)->server(2)->room(3)
class UIGameList : public UIList
{
public:
    //enum { SCROLL_TIMERID = 10 };


    struct NodeData
    {
        int _level;
        bool _expand;
        UIString _text;
        UIListLabelElement* _pListElement;
    };

    class Node
    {
        typedef std::vector <Node*>	Children;
        Children	_children;
        Node*		_parent;
        NodeData    _data;

    private:
        void set_parent(Node* parent) { _parent = parent; }

    public:
        Node() : _parent (nullptr) {}
        explicit Node(NodeData t) : _data (std::move(t)), _parent (nullptr) {}
        Node(NodeData t, Node* parent)	: _data (std::move(t)), _parent (parent) {}
        ~Node()
        {
            for (int i = 0; i < num_children(); i++)
                delete _children[i];
        }
        NodeData& data() { return _data; }
        int num_children() const { return _children.size(); }
        Node* child(int i)	{ return _children[i]; }
        Node* parent() { return ( _parent);	}
        bool has_children() const {	return num_children() > 0; }
        void add_child(Node* child)
        {
            child->set_parent(this);
            _children.push_back(child);
        }
        void remove_child(Node* child)
        {
            auto iter = _children.begin();
            for( ; iter < _children.end(); ++iter )
            {
                if( *iter == child )
                {
                    _children.erase(iter);
                    return;
                }
            }
        }
        Node* get_last_child()
        {
            if( has_children() )
            {
                return child(num_children() - 1)->get_last_child();
            }
            else return this;
        }
    };

    UIGameList() : _root(nullptr), m_dwDelayDeltaY(0), m_dwDelayNum(0), m_dwDelayLeft(0), m_timerId{0}
    {
        SetItemShowHtml(true);

        _root = new Node;
        _root->data()._level = -1;
        _root->data()._expand = true;
        _root->data()._pListElement = nullptr;
    }

    ~UIGameList() { delete _root; }

    bool Add(UIControl* pControl)
    {
        if( !pControl ) return false;
        if( pControl->GetClass() != DUI_CTR_LISTLABELELEMENT ) return false;
        return UIList::Add(pControl);
    }

    bool AddAt(UIControl* pControl, int iIndex)
    {
        if( !pControl ) return false;
        if( pControl->GetClass() != DUI_CTR_LISTLABELELEMENT ) return false;
        return UIList::AddAt(pControl, iIndex);
    }

    bool Remove(UIControl* pControl, bool bDoNotDestroy=false) override
    {
        if( !pControl ) return false;
        if( pControl->GetClass() != DUI_CTR_LISTLABELELEMENT ) return false;

        if (reinterpret_cast<Node*>(static_cast<UIListLabelElement*>(pControl->GetInterface(UIString{DUI_CTR_LISTLABELELEMENT}))->GetTag()) == nullptr)
            return UIList::Remove(pControl, bDoNotDestroy);
        else
            return RemoveNode(reinterpret_cast<Node*>(static_cast<UIListLabelElement*>(pControl->GetInterface(UIString{DUI_CTR_LISTLABELELEMENT}))->GetTag()));
    }

    bool RemoveAt(int iIndex, bool bDoNotDestroy=false)
    {
        UIControl* pControl = GetItemAt(iIndex);
        if( !pControl ) return false;
        if( pControl->GetClass() != DUI_CTR_LISTLABELELEMENT ) return false;

        if (reinterpret_cast<Node*>(static_cast<UIListLabelElement*>(pControl->GetInterface(UIString{DUI_CTR_LISTLABELELEMENT}))->GetTag()) == nullptr)
            return UIList::RemoveAt(iIndex, bDoNotDestroy);
        else
            return RemoveNode(reinterpret_cast<Node*>(static_cast<UIListLabelElement*>(pControl->GetInterface(UIString{DUI_CTR_LISTLABELELEMENT}))->GetTag()));
    }

    void RemoveAll()
    {
        UIList::RemoveAll();
        for (int i = 0; i < _root->num_children(); ++i)
        {
            Node* child = _root->child(i);
            RemoveNode(child);
        }
        delete _root;
        _root = new Node;
        _root->data()._level = -1;
        _root->data()._expand = true;
        _root->data()._pListElement = nullptr;
    }
    void SetVisible(bool bVisible = true)
    {
        if( m_visible == bVisible ) return;
        UIControl::SetVisible(bVisible);
    }

    void SetInternVisible(bool bVisible = true)
    {
        UIControl::SetInternVisible(bVisible);
    }

    void DoEvent(TEventUI& event)
    {
        if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
            if( m_parent != nullptr ) m_parent->DoEvent(event);
            else UIVerticalLayout::DoEvent(event);
            return;
        }

        if( event.Type == UIEVENT_TIMER ) {
            if( m_dwDelayLeft > 0 ) {
                --m_dwDelayLeft;
                SIZE sz = GetScrollPos();
                long lDeltaY =  (long)(CalculateDelay((double)m_dwDelayLeft / m_dwDelayNum) * m_dwDelayDeltaY);
                if( (lDeltaY > 0 && sz.cy != 0)  || (lDeltaY < 0 && sz.cy != GetScrollRange().cy ) ) {
                    sz.cy -= lDeltaY;
                    SetScrollPos(sz);
                    return;
                }
            }
            m_dwDelayDeltaY = 0;
            m_dwDelayNum = 0;
            m_dwDelayLeft = 0;
            if(m_timerId){
                m_manager->KillTimer(this, m_timerId);
                m_timerId = 0;
            }

            return;
        }
        if( event.Type == UIEVENT_SCROLLWHEEL ) {
            long lDeltaY = 0;
            if( m_dwDelayNum > 0 ) lDeltaY =  (long)(CalculateDelay((double)m_dwDelayLeft / m_dwDelayNum) * m_dwDelayDeltaY);
            switch( LOWORD(event.wParam) ) {
                case SB_LINEUP:
                    if( m_dwDelayDeltaY >= 0 ) m_dwDelayDeltaY = lDeltaY + 8;
                    else m_dwDelayDeltaY = lDeltaY + 12;
                    break;
                case SB_LINEDOWN:
                    if( m_dwDelayDeltaY <= 0 ) m_dwDelayDeltaY = lDeltaY - 8;
                    else m_dwDelayDeltaY = lDeltaY - 12;
                    break;
            }
            if( m_dwDelayDeltaY > 100 ) m_dwDelayDeltaY = 100;
            else if( m_dwDelayDeltaY < -100 ) m_dwDelayDeltaY = -100;
            m_dwDelayNum = (uint32_t)sqrt((double)abs(m_dwDelayDeltaY)) * 5;
            m_dwDelayLeft = m_dwDelayNum;
            if(m_timerId == 0){
                m_timerId = m_manager->SetTimer(this, 50U);
            }
            return;
        }

        UIList::DoEvent(event);
    }

    Node* GetRoot() { return _root; }

    Node* AddNode(const UIString &text, Node* parent = nullptr)
    {
        if( !parent ) parent = _root;

        auto* pListElement = new UIListLabelElement;
        Node* node = new Node;
        node->data()._level = parent->data()._level + 1;
        if( node->data()._level == 0 ) node->data()._expand = true;
        else node->data()._expand = false;
        node->data()._text = text;
        node->data()._pListElement = pListElement;

        if( parent != _root ) {
            if( !(parent->data()._expand && parent->data()._pListElement->IsVisible()) )
                pListElement->SetInternVisible(false);
        }

        UIString html_text;
        html_text += "<x 6>";
        for( int i = 0; i < node->data()._level; ++i ) {
            html_text += "<x 24>";
        }
        if( node->data()._level < 3 ) {
            if( node->data()._expand ) html_text += "<v center><a><i tree_expand.png 2 1></a></v>";
            else html_text += "<v center><a><i tree_expand.png 2 0></a></v>";
        }
        html_text += node->data()._text;
        pListElement->SetText(html_text);
        //if( node->data()._level == 0 ) pListElement->SetFixedHeight(28);
        //else pListElement->SetFixedHeight(24);
        pListElement->SetTag((LPVOID)node);
        if( node->data()._level == 0 ) {
            pListElement->SetBkImage(UIString{"file='tree_top.png' corner='2,1,2,1' fade='100'"});
        }

        int index = 0;
        if( parent->has_children() ) {
            Node* prev = parent->get_last_child();
            index = prev->data()._pListElement->GetIndex() + 1;
        }
        else {
            if( parent == _root ) index = 0;
            else index = parent->data()._pListElement->GetIndex() + 1;
        }
        if( !UIList::AddAt(pListElement, index) ) {
            delete pListElement;
            delete node;
            node = NULL;
        }
        parent->add_child(node);
        return node;
    }

    bool RemoveNode(Node* node)
    {
        if( !node || node == _root ) return false;
        for( int i = 0; i < node->num_children(); ++i ) {
            Node* child = node->child(i);
            RemoveNode(child);
        }
        UIList::Remove(node->data()._pListElement);
        node->parent()->remove_child(node);
        delete node;
        return true;
    }

    void ExpandNode(Node* node, bool expand)
    {
        if( !node || node == _root ) return;

        if( node->data()._expand == expand ) return;
        node->data()._expand = expand;

        UIString html_text;
        html_text += "<x 6>";
        for( int i = 0; i < node->data()._level; ++i ) {
            html_text += "<x 24>";
        }
        if( node->data()._level < 3 ) {
            if( node->data()._expand ) html_text += "<v center><a><i tree_expand.png 2 1></a></v>";
            else html_text += "<v center><a><i tree_expand.png 2 0></a></v>";
        }
        html_text += node->data()._text;
        node->data()._pListElement->SetText(html_text);

        if( !node->data()._pListElement->IsVisible() ) return;
        if( !node->has_children() ) return;

        Node* begin = node->child(0);
        Node* end = node->get_last_child();
        for( int i = begin->data()._pListElement->GetIndex(); i <= end->data()._pListElement->GetIndex(); ++i ) {
            UIControl* control = GetItemAt(i);
            if( control->GetClass() == DUI_CTR_LISTLABELELEMENT ) {
                Node* local_parent = ((UIGameList::Node*)control->GetTag())->parent();
                control->SetInternVisible(local_parent->data()._expand && local_parent->data()._pListElement->IsVisible());
            }
        }
        NeedUpdate();
    }

    SIZE GetExpanderSizeX(Node* node) const
    {
        if( !node || node == _root ) return SIZE{0,0};
        if( node->data()._level >= 3 ) return SIZE{0,0};

        SIZE szExpander = {0};
        szExpander.cx = 6 + 24 * node->data()._level - 4/*适当放大一点*/;
        szExpander.cy = szExpander.cx + 16 + 8/*适当放大一点*/;
        return szExpander;
    }

private:
    Node* _root;

    uint32_t m_timerId;

    long        m_dwDelayDeltaY;
    uint32_t    m_dwDelayNum;
    uint32_t    m_dwDelayLeft;
};

class UIDeskList : public UITileLayout
{
public:

    UIDeskList() : m_uButtonState(0), m_dwDelayDeltaY(0), m_dwDelayNum(0), m_dwDelayLeft(0), m_timerId{0}
    {
        SetItemSize(SIZE{182, 152});
        UIDlgBuilder builder;
        UIContainer* pDesk = dynamic_cast<UIContainer*>(builder.Create(UIString{"desk.xml"}));
        if( pDesk != nullptr ) {
            for(int i = 0; i < 500; ++i)
            {
                if( pDesk == nullptr ) pDesk = static_cast<UIContainer*>(builder.Create(UIString{"desk.xml"}));
                if( pDesk != nullptr ) {
                    this->Add(pDesk);
                    UIString strIndexString {"- "};
                    strIndexString += to_string(i+1).c_str();
                    strIndexString += " -";
                    pDesk->GetItemAt(3)->SetText(strIndexString);
                    pDesk = nullptr;
                }
                else {
                    this->RemoveAll();
                    return;
                }
            }
        }
    }

    void DoEvent(TEventUI& event)
    {
        if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
            if( m_parent != nullptr ) m_parent->DoEvent(event);
            else UITileLayout::DoEvent(event);
            return;
        }

        if( event.Type == UIEVENT_TIMER)
        {
            if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
                POINT pt = m_manager->GetMousePos();
                long cy = (pt.y - m_ptLastMouse.y);
                m_ptLastMouse = pt;
                SIZE sz = GetScrollPos();
                sz.cy -= cy;
                SetScrollPos(sz);
                return;
            }
            else if( m_dwDelayLeft > 0 ) {
                --m_dwDelayLeft;
                SIZE sz = GetScrollPos();
                long lDeltaY =  (long)(CalculateDelay((double)m_dwDelayLeft / m_dwDelayNum) * m_dwDelayDeltaY);
                if( (lDeltaY > 0 && sz.cy != 0)  || (lDeltaY < 0 && sz.cy != GetScrollRange().cy ) ) {
                    sz.cy -= lDeltaY;
                    SetScrollPos(sz);
                    return;
                }
            }
            m_dwDelayDeltaY = 0;
            m_dwDelayNum = 0;
            m_dwDelayLeft = 0;
            if(m_timerId != 0){
                m_manager->KillTimer(this, m_timerId);
                m_timerId = 0;
            }
            return;
        }
        if( event.Type == UIEVENT_BUTTONDOWN && IsEnabled() )
        {
            m_uButtonState |= UISTATE_CAPTURED;
            m_ptLastMouse = event.ptMouse;
            m_dwDelayDeltaY = 0;
            m_dwDelayNum = 0;
            m_dwDelayLeft = 0;
            UILoadCursor(m_manager, UI_IDC_HAND);
            //::SetCursor(::LoadCursor(nullptr, IDC_HAND));
            if(m_timerId == 0){
                m_timerId = m_manager->SetTimer(this,  50U);
            }
            return;
        }
        if( event.Type == UIEVENT_BUTTONUP )
        {
            if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
                m_uButtonState &= ~UISTATE_CAPTURED;
                UILoadCursor(m_manager, UI_IDC_ARROW);
                //::SetCursor(::LoadCursor(nullptr, IDC_ARROW));
                if( m_ptLastMouse.y != event.ptMouse.y ) {
                    m_dwDelayDeltaY = (event.ptMouse.y - m_ptLastMouse.y);
                    if( m_dwDelayDeltaY > 120 ) m_dwDelayDeltaY = 120;
                    else if( m_dwDelayDeltaY < -120 ) m_dwDelayDeltaY = -120;
                    m_dwDelayNum = (uint32_t)sqrt((double)abs(m_dwDelayDeltaY)) * 5;
                    m_dwDelayLeft = m_dwDelayNum;
                }
                else{
                    if(m_timerId != 0){
                        m_manager->KillTimer(this, m_timerId);
                        m_timerId = 0;
                    }
                }
            }
            return;
        }
        if( event.Type == UIEVENT_SCROLLWHEEL )
        {
            long lDeltaY = 0;
            if( m_dwDelayNum > 0 ) lDeltaY =  (long)(CalculateDelay((double)m_dwDelayLeft / m_dwDelayNum) * m_dwDelayDeltaY);
            switch( LOWORD(event.wParam) ) {
                case SB_LINEUP:
                    if( m_dwDelayDeltaY >= 0 ) m_dwDelayDeltaY = lDeltaY + 8;
                    else m_dwDelayDeltaY = lDeltaY + 12;
                    break;
                case SB_LINEDOWN:
                    if( m_dwDelayDeltaY <= 0 ) m_dwDelayDeltaY = lDeltaY - 8;
                    else m_dwDelayDeltaY = lDeltaY - 12;
                    break;
            }
            if( m_dwDelayDeltaY > 100 ) m_dwDelayDeltaY = 100;
            else if( m_dwDelayDeltaY < -100 ) m_dwDelayDeltaY = -100;
            m_dwDelayNum = (uint32_t)sqrt((double)abs(m_dwDelayDeltaY)) * 5;
            m_dwDelayLeft = m_dwDelayNum;
            if(m_timerId == 0){
                m_timerId = m_manager->SetTimer(this, 50U);
            }
            return;
        }
        UITileLayout::DoEvent(event);
    }

private:
    uint32_t m_uButtonState;
    POINT m_ptLastMouse;
    long m_dwDelayDeltaY;
    uint32_t m_dwDelayNum;
    uint32_t m_dwDelayLeft;
    uint32_t    m_timerId;
};


class CDialogBuilderCallbackEx : public IDialogBuilderCallback
{
public:
    UIControl* CreateControl(const char *pstrClass)override
    {
        if( strcasecmp(pstrClass, "GameList") == 0 ) return new UIGameList;
        else if( strcasecmp(pstrClass,"DeskList")==0 ) return new UIDeskList;
        return nullptr;
    }
};

#endif //DIRECTUI_CONTROLEX_H