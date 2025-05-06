#include <UIScrollBar.h>
#include "UIBackend.h"
#include "UIPaintManager.h"

POINT UIScrollBar::GetCursorPos()const {
    POINT pt = { 0 };
    Window root;
    Window child;
    int root_x = 0;
    int root_y = 0;
    int win_x = 0;
    int win_y = 0;
    unsigned int mask = 0;
    X11Window_s *window = this->m_manager->GetPaintWindow();
    XQueryPointer(window->display, window->window, &root, &child,
                      &root_x, &root_y, &win_x, &win_y, &mask);
    pt.x = win_x;
    pt.y = win_y;
    return pt;
}
