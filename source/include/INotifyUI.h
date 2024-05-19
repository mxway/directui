#ifndef DIRECTUI_INOTIFYUI_H
#define DIRECTUI_INOTIFYUI_H
#include <UIDefine.h>

class INotifyUI
{
public:
    virtual ~INotifyUI()=default;
    virtual void Notify(TNotifyUI &msg) = 0;
};

#endif //DIRECTUI_INOTIFYUI_H