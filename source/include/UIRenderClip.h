#ifndef DIRECTUI_UIRENDERCLIP_H
#define DIRECTUI_UIRENDERCLIP_H
#include <UIRect.h>
#include <memory>
#include <UIDefine.h>

using namespace std;

class UIRenderClipPrivate;

class UIRenderClip
{
public:
    UIRenderClip();
    ~UIRenderClip();

    static void GenerateClip(HANDLE_DC hdc, RECT rc, UIRenderClip &clip);
    static void GenerateRoundClip(HANDLE_DC hdc, RECT rcPaint,RECT rcItem, int width, int height, UIRenderClip &clip);
    static void UseOldClipBegin(HANDLE_DC hdc, UIRenderClip &clip);
    static void UseOldClipEnd(HANDLE_DC hdc, UIRenderClip &clip);

private:
    shared_ptr<UIRenderClipPrivate> m_impl;
};

#endif //DIRECTUI_UIRENDERCLIP_H