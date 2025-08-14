#ifndef DIRECTUI_X11BITMAP_H
#define DIRECTUI_X11BITMAP_H
#include <cstdint>
#include "DisplayInstance.h"

struct X11Bitmap
{
    X11Bitmap():bufferSize{0},
                buffer{nullptr},
                pixmap{0},
                width{0},
                height{0}
    {

    }
    ~X11Bitmap(){
        free(buffer);
        if (pixmap != 0) {
            XFreePixmap(DisplayInstance::GetInstance().GetDisplay(),pixmap);
        }
    }
    uint32_t bufferSize;
    unsigned char *buffer;
    Pixmap   pixmap;
    uint32_t width;
    uint32_t height;
};

#endif //DIRECTUI_X11BITMAP_H
