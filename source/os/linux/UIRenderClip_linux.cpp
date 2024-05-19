#include <UIRenderClip.h>
#include <cmath>

struct UIRenderClipPrivate
{
    UIRenderClipPrivate()
        : m_rcItem{0,0,0,0},
        m_roundClip {false},
        m_roundRadius{0},
        m_clipCreated{false},
        m_hDC {nullptr}
    {

    }
    RECT        m_rcPaint;
    RECT        m_rcItem;
    bool        m_roundClip;
    uint32_t    m_roundRadius;
    bool        m_clipCreated;
    HANDLE_DC   m_hDC;
};

UIRenderClip::UIRenderClip()
    :m_impl{make_shared<UIRenderClipPrivate>()}
{

}

UIRenderClip::~UIRenderClip() {
    if(m_impl->m_clipCreated){
        cairo_restore(m_impl->m_hDC);
    }
}

void UIRenderClip::GenerateClip(HANDLE_DC hdc, RECT rc, UIRenderClip &clip) {
    clip.m_impl->m_rcItem = rc;
    clip.m_impl->m_hDC = hdc;
    clip.m_impl->m_clipCreated = true;
    cairo_save(hdc);
    cairo_rectangle(clip.m_impl->m_hDC,
                    clip.m_impl->m_rcItem.left,
                    clip.m_impl->m_rcItem.top,
                    clip.m_impl->m_rcItem.right - clip.m_impl->m_rcItem.left,
                    clip.m_impl->m_rcItem.bottom - clip.m_impl->m_rcItem.top);
    cairo_clip(hdc);
}

static void GenerateRoundSubPath(HANDLE_DC hdc, RECT rcItem, int cornerRadius)
{
    double x                = rcItem.left,        /* parameters like cairo_rectangle */
                y           = rcItem.top,
            width           = rcItem.right - rcItem.left,
            height          = rcItem.bottom - rcItem.top,
            aspect          = 1.0;    /* aspect ratio */
    //corner_radius = 10;   /* and corner curvature radius */

    double radius = cornerRadius / aspect;
    double degrees = 3.1415926 / 180.0;

    cairo_new_sub_path (hdc);
    cairo_arc (hdc, x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
    cairo_arc (hdc, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
    cairo_arc (hdc, x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
    cairo_arc (hdc, x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
    cairo_close_path (hdc);
}

void UIRenderClip::GenerateRoundClip(HANDLE_DC hdc, RECT rcPaint,RECT rcItem, int width, int height, UIRenderClip &clip) {
    clip.m_impl->m_rcPaint = rcPaint;
    clip.m_impl->m_rcItem = rcItem;
    clip.m_impl->m_hDC = hdc;
    clip.m_impl->m_clipCreated = true;
    clip.m_impl->m_roundClip = true;
    clip.m_impl->m_roundRadius = width;
    cairo_save(hdc);
    GenerateRoundSubPath(hdc, rcItem, width);
    cairo_clip(hdc);
    cairo_rectangle(hdc, rcPaint.left, rcPaint.top,
                    rcPaint.right - rcPaint.left,
                    rcPaint.bottom - rcPaint.top);
    cairo_clip(hdc);
}

void UIRenderClip::UseOldClipBegin(HANDLE_DC hdc, UIRenderClip &clip) {
    if(!clip.m_impl->m_clipCreated){
        return;
    }
    cairo_restore(hdc);
}

void UIRenderClip::UseOldClipEnd(HANDLE_DC hdc, UIRenderClip &clip) {
    if(!clip.m_impl->m_clipCreated ){
        return;
    }
    cairo_save(hdc);
    if(clip.m_impl->m_roundClip){
        GenerateRoundSubPath(hdc, clip.m_impl->m_rcItem, clip.m_impl->m_roundRadius);
    }else{
        cairo_rectangle(clip.m_impl->m_hDC,
                        clip.m_impl->m_rcItem.left,
                        clip.m_impl->m_rcItem.top,
                        clip.m_impl->m_rcItem.right - clip.m_impl->m_rcItem.left,
                        clip.m_impl->m_rcItem.bottom - clip.m_impl->m_rcItem.top);
        cairo_clip(hdc);
        cairo_rectangle(clip.m_impl->m_hDC,
                        clip.m_impl->m_rcPaint.left,
                        clip.m_impl->m_rcPaint.top,
                        clip.m_impl->m_rcPaint.right - clip.m_impl->m_rcPaint.left,
                        clip.m_impl->m_rcPaint.bottom - clip.m_impl->m_rcPaint.top);
    }
    cairo_clip(hdc);
}
