#ifndef DIRECTUI_X11HDC_H
#define DIRECTUI_X11HDC_H
#include <UIBackend.h>

struct X11WindowHDC{
    //Display     *display;
    X11Window   *x11Window;
    //Drawable    draw;
    Pixmap      drawablePixmap;
    Region      currentRegion;
    GC          gc;
};

HANDLE_DC  CreateHDC(X11Window *window, Drawable draw,int width,int height);

//设置绘制区域,newRegion为新的绘制区域，返回原来的绘制区域
Region     SelectRegion(HANDLE_DC hdc,Region newRegion);

Region     GetRegion(HANDLE_DC hdc);

void       ReleaseHDC(HANDLE_DC hdc);


#endif //DIRECTUI_X11HDC_H
