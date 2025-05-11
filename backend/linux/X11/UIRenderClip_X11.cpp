#include "UIRenderClip.h"
#include "X11HDC.h"
#include "RoundRectRegion.h"

class UIRenderClipPrivate
{
public:
    RECT          rcItem;
    HANDLE_DC     hDC;
    Region        hRgn;
    Region        hOldRgn;
};

UIRenderClip::UIRenderClip()
        :m_impl{make_shared<UIRenderClipPrivate>()}
{

}

UIRenderClip::~UIRenderClip() {
    //XSetRegion(m_impl->hDC->x11Window->display,m_impl->hDC->gc,m_impl->hOldRgn);
    if(m_impl->hDC!=nullptr && m_impl->hOldRgn!=nullptr){
        SelectRegion(m_impl->hDC,m_impl->hOldRgn);
    }
    //SelectRegion(m_impl->hDC,m_impl->hOldRgn);
    if(m_impl->hRgn!=nullptr){
        XDestroyRegion(m_impl->hRgn);
    }
}

static Region CreateRectRegion(XRectangle rect){
    Region region = XCreateRegion();
    XUnionRectWithRegion(&rect,region,region);
    return region;
}

void UIRenderClip::GenerateClip(HANDLE_DC hdc, RECT rc, UIRenderClip &clip) {
    clip.m_impl->hOldRgn = GetRegion(hdc);
    XRectangle paintRectangle = {static_cast<short>(rc.left),
                                 static_cast<short>(rc.top),
                                 static_cast<unsigned short>(rc.right-rc.left),
                                 static_cast<unsigned short>(rc.bottom-rc.top)};
    clip.m_impl->hRgn = CreateRectRegion(paintRectangle);
    XIntersectRegion(clip.m_impl->hOldRgn,clip.m_impl->hRgn,clip.m_impl->hRgn);
    clip.m_impl->hOldRgn = SelectRegion(hdc, clip.m_impl->hRgn);
    clip.m_impl->hDC = hdc;
    clip.m_impl->rcItem = rc;
}

void UIRenderClip::GenerateRoundClip(HANDLE_DC hdc, RECT rcPaint,RECT rcItem, int width, int height, UIRenderClip &clip) {
    clip.m_impl->hOldRgn = GetRegion(hdc);
    UIRect rect{rcPaint.left,rcPaint.top,rcPaint.right,rcPaint.bottom};
    clip.m_impl->hRgn = CreateRoundRectRegion(rect,width);
    XIntersectRegion(clip.m_impl->hOldRgn,clip.m_impl->hRgn,clip.m_impl->hRgn);
    clip.m_impl->hOldRgn = SelectRegion(hdc, clip.m_impl->hRgn);
    clip.m_impl->hDC = hdc;
    clip.m_impl->rcItem = rcPaint;
}

void UIRenderClip::UseOldClipBegin(HANDLE_DC hdc, UIRenderClip &clip) {
    SelectRegion(hdc, clip.m_impl->hOldRgn);
}

void UIRenderClip::UseOldClipEnd(HANDLE_DC hdc, UIRenderClip &clip) {
    SelectRegion(hdc, clip.m_impl->hRgn);
}

