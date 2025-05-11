#ifndef DIRECTUI_ROUNDRECTREGION_H
#define DIRECTUI_ROUNDRECTREGION_H
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <UIRect.h>

Region CreateRoundRectRegion(const UIRect &rect,int roundCornerRadius);

void DrawRoundRect_Internal(HANDLE_DC hDC, const RECT &rc, int radiusWeight, int radiusHeight, int nSize, uint32_t dwPenColor,
                   int nStyle);

#endif //DIRECTUI_ROUNDRECTREGION_H
