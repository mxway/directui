#ifndef DIRECTUI_NOTIFYPUMP_H
#define DIRECTUI_NOTIFYPUMP_H
#include "UIDefine.h"
#include "UIString.h"
#include <UIStringPtrMap.h>

class UINotifyPump
{
public:
    bool AddVirtualWnd(UIString strName, UINotifyPump *pObject);
    bool RemoveVirtualWnd(UIString strName);
    void    NotifyPump(TNotifyUI &msg);
    bool    LoopDispatch(TNotifyUI &msg);
    DUI_DECLARE_MESSAGE_MAP()
private:
    UIStringPtrMap  m_VirtualWndMap;
};

#endif //DIRECTUI_NOTIFYPUMP_H
