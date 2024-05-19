#ifndef DIRECTUI_UIFILEHELPER_H
#define DIRECTUI_UIFILEHELPER_H
#include <UIString.h>

class UIFileHelper
{
public:
    static bool  IsAbsolutePath(const UIString &fileName);
public:
    static UIString UI_PATH_SEPARATOR;
};

#endif //DIRECTUI_UIFILEHELPER_H
