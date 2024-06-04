#ifndef DIRECTUI_UIGROUPS_H
#define DIRECTUI_UIGROUPS_H
#include <UIString.h>
#include <UIList.h>
#include <UIPaintManager.h>
#include "UIListCommonDefine.h"
#include "UIRect.h"
#include <UIDlgBuilder.h>

struct GroupsListItemInfo
{
    bool folder;
    bool empty;
    UIString id;
    UIString logo;
    UIString nick_name;
    UIString description;
};

class UIGroups : public UIList {
public:

    UIGroups(UIPaintManager& paint_manager);

    ~UIGroups();

    bool Add(UIControl* pControl);

    bool AddAt(UIControl* pControl, int iIndex);

    bool Remove(UIControl* pControl, bool bDoNotDestroy=false);

    bool RemoveAt(int iIndex, bool bDoNotDestroy=false);

    void RemoveAll();

    void DoEvent(TEventUI& event);

    Node* GetRoot();

    Node* AddNode(const GroupsListItemInfo& item, Node* parent = NULL);

    bool RemoveNode(Node* node);

    void SetChildVisible(Node* node, bool visible);

    bool CanExpand(Node* node) const;

private:
    Node*	root_node_;
    long	delay_deltaY_;
    uint32_t	delay_number_;
    uint32_t	delay_left_;
    UIRect	text_padding_;
    int level_text_start_pos_;
    UIString level_expand_image_;
    UIString level_collapse_image_;
    UIPaintManager& paint_manager_;
    uint32_t         m_timerId;

    UIDlgBuilder m_dlgBuilder;
};

#endif //DIRECTUI_UIGROUPS_H
