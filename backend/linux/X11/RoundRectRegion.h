#ifndef DIRECTUI_ROUNDRECTREGION_H
#define DIRECTUI_ROUNDRECTREGION_H
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <UIRect.h>

Region CreateRoundRectRegion(const UIRect &rect,int roundCornerRadius);

#endif //DIRECTUI_ROUNDRECTREGION_H
