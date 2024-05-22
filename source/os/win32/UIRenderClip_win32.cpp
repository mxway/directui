#include <UIRenderClip.h>
#include <cassert>

class UIRenderClipPrivate
{
public:
    RECT        rcItem;
    HDC         hDC;
    HRGN        hRgn;
    HRGN        hOldRgn;
};

UIRenderClip::UIRenderClip()
    :m_impl {make_shared<UIRenderClipPrivate>()}
{

}

UIRenderClip::~UIRenderClip() {
    assert(::GetObjectType(m_impl->hDC) == OBJ_DC || ::GetObjectType(m_impl->hDC)==OBJ_MEMDC);
    assert(::GetObjectType(m_impl->hRgn) == OBJ_REGION);
    assert(::GetObjectType(m_impl->hOldRgn) == OBJ_REGION);
    ::SelectObject(m_impl->hDC, m_impl->hOldRgn);
    ::DeleteObject(m_impl->hOldRgn);
    ::DeleteObject(m_impl->hRgn);
}

void UIRenderClip::GenerateClip(HANDLE_DC hdc, RECT rc, UIRenderClip &clip) {
    RECT rcClip = { 0 };
    ::GetClipBox(hdc, &rcClip);
    clip.m_impl->hOldRgn = ::CreateRectRgnIndirect(&rcClip);
    clip.m_impl->hRgn = ::CreateRectRgnIndirect(&rc);
    ::CombineRgn(clip.m_impl->hRgn, clip.m_impl->hRgn, clip.m_impl->hOldRgn, RGN_AND);
    ::SelectClipRgn(hdc, clip.m_impl->hRgn);
    clip.m_impl->hDC = hdc;
    clip.m_impl->rcItem = rc;
}

void UIRenderClip::GenerateRoundClip(HANDLE_DC hdc, RECT rcPaint,RECT rcItem, int width, int height, UIRenderClip &clip) {
    RECT rcClip = { 0 };
    ::GetClipBox(hdc, &rcClip);
    clip.m_impl->hOldRgn = ::CreateRectRgnIndirect(&rcClip);
    clip.m_impl->hRgn = ::CreateRectRgnIndirect( &rcPaint);
    HRGN    hRgnItem = ::CreateRoundRectRgn(rcItem.left, rcItem.top, rcItem.right + 1, rcItem.bottom + 1, width, height);
    ::CombineRgn(clip.m_impl->hRgn, clip.m_impl->hRgn, hRgnItem, RGN_AND);
    ::CombineRgn(clip.m_impl->hRgn, clip.m_impl->hRgn, clip.m_impl->hOldRgn, RGN_AND);
    ::SelectClipRgn(hdc, clip.m_impl->hRgn);
    clip.m_impl->hDC = hdc;
    clip.m_impl->rcItem = rcItem;
    ::DeleteObject(hRgnItem);
}

void UIRenderClip::UseOldClipBegin(HANDLE_DC hdc, UIRenderClip &clip) {
    ::SelectClipRgn(hdc, clip.m_impl->hOldRgn);
}

void UIRenderClip::UseOldClipEnd(HANDLE_DC hdc, UIRenderClip &clip) {
    ::SelectClipRgn(hdc, clip.m_impl->hRgn);
}
