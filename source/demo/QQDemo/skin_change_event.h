#ifndef DIRECTUI_SKIN_CHANGE_EVENT_H
#define DIRECTUI_SKIN_CHANGE_EVENT_H

#include "observer_impl_base.h"
#include <UIString.h>

struct SkinChangedParam
{
    uint32_t bkcolor;
    UIString bgimage;
};

typedef class ObserverImpl<bool, SkinChangedParam> SkinChangedObserver;
typedef class ReceiverImpl<bool, SkinChangedParam> SkinChangedReceiver;

#endif //DIRECTUI_SKIN_CHANGE_EVENT_H