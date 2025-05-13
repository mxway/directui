#include "X11HDC.h"
#include "X11Window.h"

HANDLE_DC  CreateHDC(X11Window *window, Drawable draw,int width,int height){
    auto *hdc = new X11WindowHDC;
    hdc->x11Window = window;
    //hdc->draw = draw;
    hdc->drawablePixmap = XCreatePixmap(hdc->x11Window->display,draw,width,height,window->depth);
    hdc->gc = XCreateGC(hdc->x11Window->display,hdc->drawablePixmap,0, nullptr);
    hdc->currentRegion = XCreateRegion();
    XRectangle rect = {0,0,static_cast<unsigned short>(width),static_cast<unsigned short>(height)};
    XUnionRectWithRegion(&rect,hdc->currentRegion,hdc->currentRegion);
    return hdc;
}

Region     SelectRegion(HANDLE_DC hdc,Region newRegion){
    Region oldRegion = hdc->currentRegion;
    hdc->currentRegion = newRegion;
    if (newRegion != nullptr) {
        XSetRegion(hdc->x11Window->display,hdc->gc,newRegion);
    }
    return oldRegion;
}

Region     GetRegion(HANDLE_DC hdc){
    return hdc->currentRegion;
}

void       ReleaseHDC(HANDLE_DC hdc){
    if(hdc->drawablePixmap != 0){
        XFreePixmap(hdc->x11Window->display,hdc->drawablePixmap);
    }
    if(hdc->currentRegion != nullptr){
        XDestroyRegion(hdc->currentRegion);
    }
    if(hdc->gc != nullptr){
        XFreeGC(hdc->x11Window->display,hdc->gc);
    }
    delete hdc;
}
