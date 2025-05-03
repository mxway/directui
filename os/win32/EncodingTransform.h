#ifndef DIRECTUI_ENCODINGTRANSFORM_H
#define DIRECTUI_ENCODINGTRANSFORM_H
#include <windows.h>

wchar_t *Utf8ToUcs2(const char *str, int length=-1);

char    *Ucs2ToUtf8(const wchar_t *wideString, int length=-1);

#endif //DIRECTUI_ENCODINGTRANSFORM_H
