#ifndef DIRECTUI_IMESSAGEFILTERUI_H
#define DIRECTUI_IMESSAGEFILTERUI_H
#include <UIDefine.h>
#include <cstdint>

class IMessageFilterUI
{
public:
    virtual ~IMessageFilterUI()=default;
    virtual long MessageHandler(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) = 0;
};

#endif //DIRECTUI_IMESSAGEFILTERUI_H