#ifndef DIRECTUI_X11BITMAP_H
#define DIRECTUI_X11BITMAP_H
#include <cstdint>

struct X11Bitmap
{
    X11Bitmap():bufferSize{0},
                buffer{nullptr},
                width{0},
                height{0}
    {

    }
    ~X11Bitmap(){
        delete buffer;
    }
    uint32_t bufferSize;
    unsigned char *buffer;
    uint32_t width;
    uint32_t height;
};

#endif //DIRECTUI_X11BITMAP_H
