#ifndef DIRECTUI_X11WINDOW_H
#define DIRECTUI_X11WINDOW_H
#include <X11/Xlib.h>
#include <X11/Xutil.h>

struct X11WindowHDC;

struct X11Window{
    X11Window()
        :display{nullptr},
        screen{-1},
        window{0},
        visual{nullptr},
        colormap{0},
        hdc {nullptr},
        parent{nullptr},
        depth{0},
        x{0},
        y{0},
        width{0},
        height{0}
    {

    }
    ~X11Window(){

    }
    Display     *display;
    int         screen;
    Window      window;
    Visual      *visual;
    Colormap    colormap;
    X11WindowHDC   *hdc;
    X11Window     *parent;
    int         depth;
    int         x;
    int         y;
    int         width;
    int         height;
};
#endif //DIRECTUI_X11WINDOW_H
