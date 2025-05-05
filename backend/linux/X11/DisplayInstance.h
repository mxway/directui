#ifndef DIRECTUI_DISPLAYINSTANCE_H
#define DIRECTUI_DISPLAYINSTANCE_H
#include <X11/Xlib.h>

class DisplayInstance {
public:
    ~DisplayInstance();
    DisplayInstance(const DisplayInstance &)=delete;
    DisplayInstance &operator=(const DisplayInstance &)=delete;
    static DisplayInstance &GetInstance();
    Display*        GetDisplay()const;
    int             GetWidth()const;
    int             GetHeight()const;
    int             GetScreenNumber()const;
    int             GetDisplayDepth()const;
    Visual*         GetVisual()const;
    Colormap        GetColormap()const;
private:
    DisplayInstance();
private:
    Display *m_display;
};


#endif //DIRECTUI_DISPLAYINSTANCE_H
