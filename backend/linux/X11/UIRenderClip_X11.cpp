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

static Region CreateRectRegion(RECT rc) {
    XRectangle paintRectangle = {static_cast<short>(rc.left),
                                 static_cast<short>(rc.top),
                                 static_cast<unsigned short>(rc.right-rc.left),
                                 static_cast<unsigned short>(rc.bottom-rc.top)};
    Region region = XCreateRegion();
    XUnionRectWithRegion(&paintRectangle,region,region);
    return region;
}

void UIRenderClip::GenerateClip(HANDLE_DC hdc, RECT rc, UIRenderClip &clip) {
    clip.m_impl->hOldRgn = GetRegion(hdc);
    XRectangle paintRectangle = {static_cast<short>(rc.left),
                                 static_cast<short>(rc.top),
                                 static_cast<unsigned short>(rc.right-rc.left),
                                 static_cast<unsigned short>(rc.bottom-rc.top)};
    clip.m_impl->hRgn = CreateRectRegion(rc);
    XIntersectRegion(clip.m_impl->hOldRgn,clip.m_impl->hRgn,clip.m_impl->hRgn);
    clip.m_impl->hOldRgn = SelectRegion(hdc, clip.m_impl->hRgn);
    clip.m_impl->hDC = hdc;
    clip.m_impl->rcItem = rc;
}

void UIRenderClip::GenerateRoundClip(HANDLE_DC hdc, RECT rcPaint,RECT rcItem, int width, int height, UIRenderClip &clip) {
    clip.m_impl->hOldRgn = GetRegion(hdc);
    UIRect rect{rcItem.left,rcItem.top,rcItem.right,rcItem.bottom};
    //控件的矩形区域和需要重新绘制的区域可能是不一样的。比如一个List控件，在更新时可能只需要重新绘制list表头。
    //而这里的圆角矩形是指控件创建圆角矩形区域。
    clip.m_impl->hRgn = CreateRoundRectRegion(rect,width); //创建控件的圆角矩形区域
    Region  paintRegion = CreateRectRegion(rcPaint); //创建重绘区域
    XIntersectRegion(clip.m_impl->hRgn, paintRegion, clip.m_impl->hRgn); //两者取交集
    XIntersectRegion(clip.m_impl->hOldRgn,clip.m_impl->hRgn,clip.m_impl->hRgn);
    clip.m_impl->hOldRgn = SelectRegion(hdc, clip.m_impl->hRgn);
    clip.m_impl->hDC = hdc;
    clip.m_impl->rcItem = rcPaint;
    XDestroyRegion(paintRegion);
}

void UIRenderClip::UseOldClipBegin(HANDLE_DC hdc, UIRenderClip &clip) {
    SelectRegion(hdc, clip.m_impl->hOldRgn);
}

void UIRenderClip::UseOldClipEnd(HANDLE_DC hdc, UIRenderClip &clip) {
    SelectRegion(hdc, clip.m_impl->hRgn);
}

