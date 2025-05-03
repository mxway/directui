#ifndef DIRECTUI_CHAT_DIALOG_H
#define DIRECTUI_CHAT_DIALOG_H
#include <UIWindowImpBase.h>
#include "skin_change_event.h"
#include "UIFriends.h"

class ChatDialog : public UIWindowImpBase, public SkinChangedReceiver  {
public:
    ChatDialog(const UIString &bgimage, uint32_t bkcolor, const FriendListItemInfo &myself_info,const FriendListItemInfo &friend_info);
    ~ChatDialog();
    void    OnFinalMessage(HANDLE_WND wnd)override;
};

#endif //DIRECTUI_CHAT_DIALOG_H
