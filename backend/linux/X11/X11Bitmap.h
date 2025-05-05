#ifndef DIRECTUI_X11BITMAP_H
#define DIRECTUI_X11BITMAP_H
#include <cstdint>

struct X11Bitmap
{
    X11Bitmap():bufferSize{0},
                buffer{nullptr}
    {

    }
    ~X11Bitmap(){
        delete buffer;
    }
    uint32_t bufferSize;
    unsigned char *buffer;
};

#endif //DIRECTUI_X11BITMAP_H
