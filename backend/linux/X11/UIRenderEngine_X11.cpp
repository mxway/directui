#include "UIRenderEngine.h"
#include "X11HDC.h"
#include "UIResourceMgr.h"
#include "UIRect.h"
#include <cassert>
#include <pango/pango-layout.h>
#include <pango/pangoxft.h>
#include "X11Bitmap.h"
#include <UIPaintManager.h>
#include <cctype>

static int g_iFontID = MAX_FONT_ID;

static bool AlphaBlend(HANDLE_DC hdc, Pixmap pixmap, int dx, int dy, int dWidth, int dHeight,
                       int sx, int sy, int sWidth, int sHeight, int alpha)
{
    Pixmap subPixmap = XCreatePixmap(hdc->x11Window->display,pixmap,sWidth,sHeight,32);
    GC gc = XCreateGC(hdc->x11Window->display,subPixmap,0,nullptr);
    XCopyArea(hdc->x11Window->display,pixmap,subPixmap,gc,sx,sy,sWidth,sHeight,0,0);
    XFreeGC(hdc->x11Window->display,gc);
    XRenderPictFormat *pixmapFormat = XRenderFindStandardFormat(hdc->x11Window->display,PictStandardARGB32);
    XRenderPictFormat *offscreenFormat = hdc->x11Window->depth==32?XRenderFindStandardFormat(hdc->x11Window->display,PictStandardARGB32)
                                                                 : XRenderFindStandardFormat(hdc->x11Window->display,PictStandardRGB24);
    Picture picture = XRenderCreatePicture(hdc->x11Window->display,subPixmap,pixmapFormat,0,nullptr);
    XTransform transform = {{
                                    { XDoubleToFixed(sWidth*1.0/dWidth), XDoubleToFixed(0), XDoubleToFixed(0) },
                                    { XDoubleToFixed(0), XDoubleToFixed(sHeight*1.0/dHeight), XDoubleToFixed(0) },
                                    { XDoubleToFixed(0), XDoubleToFixed(0), XDoubleToFixed(1) }
                            }};
    XRenderSetPictureTransform(hdc->x11Window->display, picture, &transform);
    Picture offscreenPicture = XRenderCreatePicture(hdc->x11Window->display,hdc->drawablePixmap,offscreenFormat,0,nullptr);

    XRenderComposite(hdc->x11Window->display,PictOpOver,picture,None,offscreenPicture,0,0,0,0,dx,dy,dWidth,dHeight);
    XFreePixmap(hdc->x11Window->display,subPixmap);
    XRenderFreePicture(hdc->x11Window->display,picture);
    XRenderFreePicture(hdc->x11Window->display,offscreenPicture);

    return true;
}

void UIRenderEngine::DrawImage(HANDLE_DC hDC, HANDLE_BITMAP hBitmap, const RECT &rc, const RECT &rcPaint,
                               const RECT &rcBmpPart, const RECT &rcScale9, bool alpha, uint8_t uFade, bool hole,
                               bool xtiled, bool ytiled) {
    RECT rcDest;
    RECT rcTemp;
    Pixmap pixmap = XCreatePixmap(hDC->x11Window->display,hDC->drawablePixmap,hBitmap->width,
                                  hBitmap->height,32);
    GC pixmapGC = XCreateGC(hDC->x11Window->display,pixmap,0,nullptr);
    auto *data = static_cast<unsigned char *>(malloc(hBitmap->bufferSize));
    memcpy(data, hBitmap->buffer,hBitmap->bufferSize);
    XImage *ximage = XCreateImage(hDC->x11Window->display, hDC->x11Window->visual,32, ZPixmap, 0,
                 reinterpret_cast<char *>(data),
                 hBitmap->width,hBitmap->height, 32, 0);
    XPutImage(hDC->x11Window->display,pixmap,pixmapGC,ximage,0,0,0,0,hBitmap->width,hBitmap->height);
    XFreeGC(hDC->x11Window->display,pixmapGC);
    XDestroyImage(ximage);
    //
    // middle
    //

    if(!hole){
        rcDest.left = rc.left + rcScale9.left;
        rcDest.top = rc.top + rcScale9.top;
        rcDest.right = rc.right - rc.left - rcScale9.left - rcScale9.right;
        rcDest.bottom = rc.bottom - rc.top - rcScale9.top - rcScale9.bottom;
        rcDest.right += rcDest.left;
        rcDest.bottom += rcDest.top;
        if(UIIntersectRect(&rcTemp, &rcPaint, &rcDest)){
            if(!xtiled && !ytiled) {
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                AlphaBlend(hDC, pixmap, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, \
                    rcBmpPart.left + rcScale9.left, rcBmpPart.top + rcScale9.top, \
                    rcBmpPart.right - rcBmpPart.left - rcScale9.left - rcScale9.right, \
                    rcBmpPart.bottom - rcBmpPart.top - rcScale9.top - rcScale9.bottom, uFade);

            }else if(xtiled && ytiled){
                long lWidth = rcBmpPart.right - rcBmpPart.left - rcScale9.left - rcScale9.right;
                long lHeight = rcBmpPart.bottom - rcBmpPart.top - rcScale9.top - rcScale9.bottom;
                int iTimesX = (rcDest.right - rcDest.left + lWidth - 1) / lWidth;
                int iTimesY = (rcDest.bottom - rcDest.top + lHeight - 1) / lHeight;
                for(int j = 0; j < iTimesY; ++j){
                    long lDestTop = rcDest.top + lHeight * j;
                    long lDestBottom = rcDest.top + lHeight * (j + 1);
                    long lDrawHeight = lHeight;
                    if(lDestBottom > rcDest.bottom){
                        lDrawHeight -= lDestBottom - rcDest.bottom;
                        lDestBottom = rcDest.bottom;
                    }
                    for(int i = 0; i < iTimesX; ++i){
                        long lDestLeft = rcDest.left + lWidth * i;
                        long lDestRight = rcDest.left + lWidth * (i + 1);
                        long lDrawWidth = lWidth;
                        if( lDestRight > rcDest.right ) {
                            lDrawWidth -= lDestRight - rcDest.right;
                            lDestRight = rcDest.right;
                        }
                        AlphaBlend(hDC, pixmap, rcDest.left + lWidth * i, rcDest.top + lHeight * j,
                                   lDestRight - lDestLeft, lDestBottom - lDestTop,
                                   rcBmpPart.left + rcScale9.left, rcBmpPart.top + rcScale9.top, lDrawWidth, lDrawHeight, uFade);
                    }
                }
            }else if(xtiled){
                long lWidth = rcBmpPart.right - rcBmpPart.left - rcScale9.left - rcScale9.right;
                int iTimes = (rcDest.right - rcDest.left + lWidth - 1) / lWidth;
                for(int i = 0; i < iTimes; ++i){
                    long lDestLeft = rcDest.left + lWidth * i;
                    long lDestRight = rcDest.left + lWidth * (i + 1);
                    long lDrawWidth = lWidth;
                    if(lDestRight > rcDest.right){
                        lDrawWidth -= lDestRight - rcDest.right;
                        lDestRight = rcDest.right;
                    }
                    AlphaBlend(hDC, pixmap, lDestLeft, rcDest.top, lDestRight - lDestLeft, rcDest.bottom,
                               rcBmpPart.left + rcScale9.left, rcBmpPart.top + rcScale9.top, \
                        lDrawWidth, rcBmpPart.bottom - rcBmpPart.top - rcScale9.top - rcScale9.bottom, uFade);
                }
            }else{

                //
                // ytiled
                //

                long lHeight = rcBmpPart.bottom - rcBmpPart.top - rcScale9.top - rcScale9.bottom;
                int iTimes = (rcDest.bottom - rcDest.top + lHeight - 1) / lHeight;
                for(int i = 0; i < iTimes; ++i){
                    long lDestTop = rcDest.top + lHeight * i;
                    long lDestBottom = rcDest.top + lHeight * (i + 1);
                    long lDrawHeight = lHeight;
                    if(lDestBottom > rcDest.bottom){
                        lDrawHeight -= lDestBottom - rcDest.bottom;
                        lDestBottom = rcDest.bottom;
                    }
                    AlphaBlend(hDC, pixmap, rcDest.left, rcDest.top + lHeight * i, rcDest.right, lDestBottom - lDestTop,
                               rcBmpPart.left + rcScale9.left, rcBmpPart.top + rcScale9.top, \
                        rcBmpPart.right - rcBmpPart.left - rcScale9.left - rcScale9.right, lDrawHeight, uFade);
                }
            }
        }
    }

    //
    // left-top
    //

    if(rcScale9.left > 0 && rcScale9.top > 0){
        rcDest.left = rc.left;
        rcDest.top = rc.top;
        rcDest.right = rcScale9.left;
        rcDest.bottom = rcScale9.top;
        rcDest.right += rcDest.left;
        rcDest.bottom += rcDest.top;
        if( UIIntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
            rcDest.right -= rcDest.left;
            rcDest.bottom -= rcDest.top;
            AlphaBlend(hDC, pixmap, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, \
                rcBmpPart.left, rcBmpPart.top, rcScale9.left, rcScale9.top, uFade);
        }
    }

    //
    // top
    //

    if(rcScale9.top > 0){
        rcDest.left = rc.left + rcScale9.left;
        rcDest.top = rc.top;
        rcDest.right = rc.right - rc.left - rcScale9.left - rcScale9.right;
        rcDest.bottom = rcScale9.top;
        rcDest.right += rcDest.left;
        rcDest.bottom += rcDest.top;
        if(UIIntersectRect(&rcTemp, &rcPaint, &rcDest)) {
            rcDest.right -= rcDest.left;
            rcDest.bottom -= rcDest.top;
            AlphaBlend(hDC, pixmap, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, \
                rcBmpPart.left + rcScale9.left, rcBmpPart.top, rcBmpPart.right - rcBmpPart.left - \
                rcScale9.left - rcScale9.right, rcScale9.top, uFade);
        }
    }

    //
    // right-top
    //

    if(rcScale9.right > 0 && rcScale9.top > 0){
        rcDest.left = rc.right - rcScale9.right;
        rcDest.top = rc.top;
        rcDest.right = rcScale9.right;
        rcDest.bottom = rcScale9.top;
        rcDest.right += rcDest.left;
        rcDest.bottom += rcDest.top;
        if(UIIntersectRect(&rcTemp, &rcPaint, &rcDest)){
            rcDest.right -= rcDest.left;
            rcDest.bottom -= rcDest.top;
            AlphaBlend(hDC, pixmap, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, \
                rcBmpPart.right - rcScale9.right, rcBmpPart.top, rcScale9.right, rcScale9.top, uFade);
        }
    }

    //
    // left
    //

    if(rcScale9.left > 0){
        rcDest.left = rc.left;
        rcDest.top = rc.top + rcScale9.top;
        rcDest.right = rcScale9.left;
        rcDest.bottom = rc.bottom - rc.top - rcScale9.top - rcScale9.bottom;
        rcDest.right += rcDest.left;
        rcDest.bottom += rcDest.top;
        if(UIIntersectRect(&rcTemp, &rcPaint, &rcDest)){
            rcDest.right -= rcDest.left;
            rcDest.bottom -= rcDest.top;
            AlphaBlend(hDC, pixmap, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, \
                rcBmpPart.left, rcBmpPart.top + rcScale9.top, rcScale9.left, rcBmpPart.bottom - \
                rcBmpPart.top - rcScale9.top - rcScale9.bottom, uFade);
        }
    }

    //
    // right
    //

    if(rcScale9.right > 0){
        rcDest.left = rc.right - rcScale9.right;
        rcDest.top = rc.top + rcScale9.top;
        rcDest.right = rcScale9.right;
        rcDest.bottom = rc.bottom - rc.top - rcScale9.top - rcScale9.bottom;
        rcDest.right += rcDest.left;
        rcDest.bottom += rcDest.top;
        if(UIIntersectRect(&rcTemp, &rcPaint, &rcDest)){
            rcDest.right -= rcDest.left;
            rcDest.bottom -= rcDest.top;
            AlphaBlend(hDC, pixmap, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, \
                rcBmpPart.right - rcScale9.right, rcBmpPart.top + rcScale9.top, rcScale9.right, \
                rcBmpPart.bottom - rcBmpPart.top - rcScale9.top - rcScale9.bottom, uFade);
        }
    }

    //
    // left-bottom
    //

    if(rcScale9.left > 0 && rcScale9.bottom > 0){
        rcDest.left = rc.left;
        rcDest.top = rc.bottom - rcScale9.bottom;
        rcDest.right = rcScale9.left;
        rcDest.bottom = rcScale9.bottom;
        rcDest.right += rcDest.left;
        rcDest.bottom += rcDest.top;
        if( UIIntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
            rcDest.right -= rcDest.left;
            rcDest.bottom -= rcDest.top;
            AlphaBlend(hDC, pixmap, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, \
                rcBmpPart.left, rcBmpPart.bottom - rcScale9.bottom, rcScale9.left, rcScale9.bottom, uFade);
        }
    }

    //
    // bottom
    //

    if(rcScale9.bottom > 0){
        rcDest.left = rc.left + rcScale9.left;
        rcDest.top = rc.bottom - rcScale9.bottom;
        rcDest.right = rc.right - rc.left - rcScale9.left - rcScale9.right;
        rcDest.bottom = rcScale9.bottom;
        rcDest.right += rcDest.left;
        rcDest.bottom += rcDest.top;
        if(UIIntersectRect(&rcTemp, &rcPaint, &rcDest)){
            rcDest.right -= rcDest.left;
            rcDest.bottom -= rcDest.top;
            AlphaBlend(hDC, pixmap, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, \
                rcBmpPart.left + rcScale9.left, rcBmpPart.bottom - rcScale9.bottom, \
                rcBmpPart.right - rcBmpPart.left - rcScale9.left - rcScale9.right, rcScale9.bottom, uFade);
        }
    }

    //
    // right-bottom
    //

    if(rcScale9.right > 0 && rcScale9.bottom > 0){
        rcDest.left = rc.right - rcScale9.right;
        rcDest.top = rc.bottom - rcScale9.bottom;
        rcDest.right = rcScale9.right;
        rcDest.bottom = rcScale9.bottom;
        rcDest.right += rcDest.left;
        rcDest.bottom += rcDest.top;
        if(UIIntersectRect(&rcTemp, &rcPaint, &rcDest)){
            rcDest.right -= rcDest.left;
            rcDest.bottom -= rcDest.top;
            AlphaBlend(hDC, pixmap, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, \
                rcBmpPart.right - rcScale9.right, rcBmpPart.bottom - rcScale9.bottom, rcScale9.right, \
                rcScale9.bottom, uFade);
        }
    }
    XFreePixmap(hDC->x11Window->display,pixmap);
}

void UIRenderEngine::DrawColor(HANDLE_DC hDC, const RECT &rc, uint32_t color) {
    if(color >= 0xff000000){
        XSetForeground(hDC->x11Window->display,hDC->gc,color);
        XFillRectangle(hDC->x11Window->display,hDC->drawablePixmap,hDC->gc,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top);
        return;
    }

    Display  *display = hDC->x11Window->display;
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;
    double fade = ((color>>24)&0xff)/255.0;
    XRenderColor alpha = {.alpha = (ushort)(0xffff*fade)};
    XRenderColor iAlpha = {.alpha = (ushort)(0xffff*(1-fade))};
    Picture a = XRenderCreateSolidFill(display,&alpha);
    Picture ia = XRenderCreateSolidFill(display,&iAlpha);

    Pixmap  originPixmap = XCreatePixmap(display,hDC->drawablePixmap,width,height,hDC->x11Window->depth);
    Pixmap  colorPixmap = XCreatePixmap(display,hDC->drawablePixmap,width,height,hDC->x11Window->depth);

    GC originPixmapGC = XCreateGC(display,originPixmap,0,nullptr);
    XCopyArea(display,hDC->drawablePixmap,originPixmap,originPixmapGC,rc.left,rc.top,width,
        height,0,0);
    XFreeGC(display,originPixmapGC);

    GC colorGC = XCreateGC(display,colorPixmap,0,nullptr);
    XSetForeground(display,colorGC,color&0xffffff);
    XFillRectangle(display,colorPixmap,colorGC,0,0,width,height);
    XFreeGC(display,colorGC);

    Picture dstPicture = XRenderCreatePicture(display,hDC->drawablePixmap,XRenderFindVisualFormat(display, hDC->x11Window->visual),0,nullptr);
    Picture originPicture = XRenderCreatePicture(display,originPixmap,XRenderFindVisualFormat(display, hDC->x11Window->visual),0,nullptr);
    Picture colorPicture = XRenderCreatePicture(display,colorPixmap,XRenderFindVisualFormat(display, hDC->x11Window->visual),0,nullptr);

    XRenderComposite(display,PictOpSrc,colorPicture,a,dstPicture,0,0,0,0,rc.left,rc.top,width,height);
    XRenderComposite(display,PictOpAdd,originPicture,ia,dstPicture,0,0,0,0,rc.left,rc.top,width,height);

    XRenderFreePicture(display,colorPicture);
    XRenderFreePicture(display,originPicture);
    XRenderFreePicture(display,dstPicture);
    XFreePixmap(display,originPixmap);
    XFreePixmap(display,colorPixmap);
    XRenderFreePicture(display,a);
    XRenderFreePicture(display,ia);
}

void UIRenderEngine::DrawGradient(HANDLE_DC hDC, const RECT &rc, uint32_t dwFirst, uint32_t dwSecond, bool bVertical,
                                  int nSteps) {
    // 获取默认的视觉和格式
    XRenderPictFormat *format = XRenderFindVisualFormat(hDC->x11Window->display, hDC->x11Window->visual);
    if (!format) {
        fprintf(stderr, "无法找到合适的 XRender 格式\n");
    }

    // 创建窗口的图片（Picture）
    Picture window_picture = XRenderCreatePicture(hDC->x11Window->display, hDC->drawablePixmap, format, 0, NULL);

    // 定义线性渐变的起点和终点
    XLinearGradient gradient;
    gradient.p1.x = XDoubleToFixed(rc.left);   // 起点 x 坐标
    gradient.p1.y = XDoubleToFixed(rc.top);   // 起点 y 坐标
    if(bVertical){
        gradient.p2.x = XDoubleToFixed(rc.left);
        gradient.p2.y = XDoubleToFixed(rc.bottom);
    }else{
        gradient.p2.x = XDoubleToFixed(rc.right);
        gradient.p2.y = XDoubleToFixed(rc.top);
    }

    XFixed stops[2];
    XRenderColor colors[2];

    stops[0] = XDoubleToFixed(0.0);
    colors[0].red = XDoubleToFixed( ((dwFirst>>16) & 0xff)/256.0);
    colors[0].green = XDoubleToFixed( ((dwFirst>>8) & 0xff)/256.0);
    colors[0].blue = XDoubleToFixed( (dwFirst&0xff)/256.0);
    colors[0].alpha = XDoubleToFixed( ((dwFirst>>24) & 0xff)/256.0);

    stops[1] = XDoubleToFixed(1.0);
    colors[1].red = XDoubleToFixed( ((dwSecond>>16) & 0xff)/256.0);
    colors[1].green = XDoubleToFixed( ((dwSecond>>8) & 0xff)/256.0);
    colors[1].blue = XDoubleToFixed( (dwSecond&0xff)/256.0);
    colors[1].alpha = XDoubleToFixed( ((dwSecond>>24) & 0xff)/256.0);
    //colors[1].alpha = XDoubleToFixed( ((dwSecond>>24) & 0xff)/255.0);

    // 创建线性渐变图案
    Picture gradient_picture = XRenderCreateLinearGradient(hDC->x11Window->display, &gradient, stops, colors, 2);
    XRenderComposite(hDC->x11Window->display, PictOpSrc, gradient_picture, None, window_picture,
                     rc.left, rc.top, 0, 0, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top);
    XRenderFreePicture(hDC->x11Window->display, gradient_picture);
    XRenderFreePicture(hDC->x11Window->display, window_picture);
}

void UIRenderEngine::DrawLine(HANDLE_DC hDC, const RECT &rc, int nSize, uint32_t dwPenColor, int nStyle) {
    XSetForeground(hDC->x11Window->display,hDC->gc,dwPenColor);
    XSetLineAttributes(hDC->x11Window->display,hDC->gc,nSize,nStyle,CapButt, JoinMiter);
    XDrawLine(hDC->x11Window->display,hDC->drawablePixmap,hDC->gc,rc.left,rc.top,rc.right,rc.bottom);
}

void UIRenderEngine::DrawRect(HANDLE_DC hDC, const RECT &rc, int nSize, uint32_t dwPenColor, int nStyle) {
    XSetForeground(hDC->x11Window->display,hDC->gc, dwPenColor);
    XSetLineAttributes(hDC->x11Window->display,hDC->gc, nSize,nStyle,CapButt,JoinMiter);
    XDrawRectangle(hDC->x11Window->display,hDC->drawablePixmap,hDC->gc,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top);
}

void UIRenderEngine::DrawRoundRect(HANDLE_DC hDC, const RECT &rc, int radiusWeight, int radiusHeight, int nSize, uint32_t dwPenColor,
                                   int nStyle) {

}

void UIRenderEngine::DrawText(HANDLE_DC hDC, UIPaintManager* pManager, RECT& rc, const UIString &text, \
        uint32_t dwTextColor, int fontId, uint32_t uStyle)
{
    int nFixY = rc.top;
    int nWidth = rc.right - rc.left;
    int nHeight = rc.bottom - rc.top;
    // 创建 PangoXft 字体映射
    PangoFontMap *font_map = pango_xft_get_font_map(hDC->x11Window->display,hDC->x11Window->screen);
    PangoContext *context = pango_font_map_create_context(font_map);

    // 创建 Pango 布局对象
    PangoLayout *layout = pango_layout_new(context);
    PangoFontDescription *FontDesc;

    Region region = XCreateRegion();
    XRectangle  regionRect = {static_cast<short>(rc.left),
        static_cast<short>(rc.top),
        static_cast<unsigned short>(nWidth),
        static_cast<unsigned short>(nHeight)};
    XUnionRectWithRegion(&regionRect,region,region);

    // 设置文本内容
    pango_layout_set_text(layout, text.GetData(), text.GetLength());

    UIFont  *font = UIResourceMgr::GetInstance().GetFont(fontId);
    if (font){
        FontDesc = (PangoFontDescription *) font->GetHandle();
    }else{
        FontDesc = (PangoFontDescription *)UIResourceMgr::GetInstance().GetDefaultFont()->GetHandle();
    }

    pango_layout_set_font_description(layout, FontDesc);

    //
    // set alignment
    //

    if (uStyle & DT_LEFT){
        pango_layout_set_alignment(layout, PANGO_ALIGN_LEFT);
    }else if (uStyle & DT_CENTER){
        pango_layout_set_alignment(layout, PANGO_ALIGN_CENTER);
    }else if (uStyle & DT_RIGHT){
        pango_layout_set_alignment(layout, PANGO_ALIGN_RIGHT);
    }

    if (uStyle & DT_END_ELLIPSIS){
        pango_layout_set_ellipsize(layout, PANGO_ELLIPSIZE_END);
    }
    UIString strText{text};

    if (uStyle & DT_SINGLELINE){
        pango_layout_set_single_paragraph_mode(layout, TRUE);
    }else{

        //
        // 替换掉字符串中的\r
        //
        pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);
        strText.Replace("\r\n", "\n");
        pango_layout_set_single_paragraph_mode(layout, FALSE);
    }

    bool shouldReleaseAttrList = false;
    PangoAttrList  *attrList = pango_layout_get_attributes(layout);
    if(font->GetUnderline()){
        if(attrList == nullptr){
            attrList = pango_attr_list_new();
            shouldReleaseAttrList = true;
        }
        pango_attr_list_insert(attrList, pango_attr_underline_new(PANGO_UNDERLINE_SINGLE));
        pango_layout_set_attributes(layout, attrList);
    }

    pango_layout_set_width(layout, nWidth * PANGO_SCALE/*-1*/);
    // 设置换行模式为自动换行
    //pango_layout_set_width(layout, 400 * PANGO_SCALE); // 宽度限制（以 Pango 单位表示）
    //pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR); // 自动换行

    int textWidth, textHeigth;
    pango_layout_get_pixel_size(layout, &textWidth, &textHeigth);

    if (uStyle & DT_TOP){

        //
        // do not change y
        //

        nFixY = rc.top;
    }else if (uStyle & DT_BOTTOM){
        nFixY = rc.top + MAX(nHeight - textHeigth, 0);
    }else if (uStyle & DT_VCENTER){
        nFixY = rc.top + MAX((nHeight - textHeigth) / 2, 0);
    }

    //
    // move to draw start point
    //

    if(shouldReleaseAttrList){
        pango_attr_list_unref(attrList);
    }

    XftColor  color;
    color.color.red = XDoubleToFixed( ( (dwTextColor>>16)&0xff)/256.0);
    color.color.green = XDoubleToFixed( ((dwTextColor>>8)&0xff)/256.0);
    color.color.blue = XDoubleToFixed( (dwTextColor&0xff)/256.0);
    color.color.alpha = XDoubleToFixed( ((dwTextColor>>24)&0xff)/256.0);

    XftDraw *draw = XftDrawCreate(hDC->x11Window->display, hDC->drawablePixmap,
                                  hDC->x11Window->visual,
                                  hDC->x11Window->colormap);
    XftDrawSetClip(draw,region);
    //XftDrawSetClipRectangles(draw,)
    pango_xft_render_layout(draw, &color,layout,rc.left*PANGO_SCALE,(nFixY-1)*PANGO_SCALE);

    XDestroyRegion(region);
    XftDrawDestroy(draw);
    // 清理资源
    g_object_unref(layout);
    g_object_unref(context);
}

static void SetRect(RECT *rect, int left, int top, int right, int bottom)
{
    rect->left = left;
    rect->top = top;
    rect->right = right;
    rect->bottom = bottom;
}

static void GetTextExtentPoint(HANDLE_DC hDC, const char *text, int cchSize, UIFont *font, SIZE *szText)
{
    PangoFontDescription *fontDesc = nullptr;
    memset(szText,0, sizeof(SIZE));
    PangoFontMap *font_map = pango_xft_get_font_map(hDC->x11Window->display,hDC->x11Window->screen);
    PangoContext *context = pango_font_map_create_context(font_map);

    // 创建 Pango 布局对象
    PangoLayout *layout = pango_layout_new(context);
    //PangoLayout *Layout = pango_cairo_create_layout(hDC);

    if (font){
        fontDesc = (PangoFontDescription *) font->GetHandle();
    }else{
        fontDesc = (PangoFontDescription *)UIResourceMgr::GetInstance().GetDefaultFont()->GetHandle();
    }


    //
    // set font to layout
    //

    pango_layout_set_font_description(layout, fontDesc);
    pango_layout_set_text(layout,text,cchSize);
    int width = 0;
    int height = 0;
    pango_layout_get_pixel_size(layout, &width, &height);
    szText->cx = width;
    szText->cy = height;
    g_object_unref(layout);
    g_object_unref(context);
}

static void TextOut(XftDraw *xftDraw,HANDLE_DC hDC, uint32_t textColor, long x, long y, const char *text, int cchSize, UIFont *font)
{
    PangoFontDescription *FontDesc;

    PangoFontMap *font_map = pango_xft_get_font_map(hDC->x11Window->display,hDC->x11Window->screen);
    PangoContext *context = pango_font_map_create_context(font_map);

    // 创建 Pango 布局对象
    PangoLayout *layout = pango_layout_new(context);
    //Layout = pango_cairo_create_layout(hDC);

    PangoAttrList  *attrList = pango_layout_get_attributes(layout);
    //PangoAttribute  *underLineAttribute = nullptr;
    bool shouldReleaseAttrList = false;
    if(font->GetUnderline()){
        if(attrList == nullptr){
            attrList = pango_attr_list_new();
            shouldReleaseAttrList = true;
        }
        pango_attr_list_insert(attrList, pango_attr_underline_new(PANGO_UNDERLINE_SINGLE));
        pango_layout_set_attributes(layout, attrList);
    }

    if (font){
        FontDesc = (PangoFontDescription *) font->GetHandle();
    }else{
        FontDesc = (PangoFontDescription *)UIResourceMgr::GetInstance().GetDefaultFont()->GetHandle();
    }

    pango_layout_set_font_description(layout, FontDesc);

    pango_layout_set_text(layout, text, cchSize);

    XftColor  color;
    color.color.red = XDoubleToFixed( ( (textColor>>16)&0xff)/256.0);
    color.color.green = XDoubleToFixed( ((textColor>>8)&0xff)/256.0);
    color.color.blue = XDoubleToFixed( (textColor&0xff)/256.0);
    color.color.alpha = XDoubleToFixed( ((textColor>>24)&0xff)/256.0);
    if (color.color.alpha == 0) {
        color.color.alpha = 65535;
    }

    pango_xft_render_layout(xftDraw, &color,layout,x*PANGO_SCALE,y*PANGO_SCALE);

    if(shouldReleaseAttrList){
        pango_attr_list_unref(attrList);
    }

    g_object_unref(layout);
    g_object_unref(context);
}

void UIRenderEngine::DrawHtmlText(HANDLE_DC hDC, UIPaintManager* pManager, RECT& rc, const UIString &text,
                                  uint32_t dwTextColor, RECT* prcLinks, UIString* sLinks, int& nLinkRects, int iDefaultFont, uint32_t uStyle)
{
// 考虑到在xml编辑器中使用<>符号不方便，可以使用{}符号代替
    // 支持标签嵌套（如<l><b>text</b></l>），但是交叉嵌套是应该避免的（如<l><b>text</l></b>）
    // The string formatter supports a kind of "mini-html" that consists of various short tags:
    //
    //   Bold:             <b>text</b>
    //   Color:            <c #xxxxxx>text</c>  where x = RGB in hex
    //   Font:             <f x>text</f>        where x = font id
    //   Italic:           <i>text</i>
    //   Image:            <i x y z>            where x = image name and y = imagelist num and z(optional) = imagelist id
    //   Link:             <a x>text</a>        where x(optional) = link content, normal like app:notepad or http:www.xxx.com
    //   NewLine           <n>
    //   Paragraph:        <p x>text</p>        where x = extra pixels indent in p
    //   Raw Text:         <r>text</r>
    //   Selected:         <s>text</s>
    //   Underline:        <u>text</u>
    //   X Indent:         <x i>                where i = hor indent in pixels
    //   Y Indent:         <y i>                where i = ver indent in pixels
    //   Vertical align    <v x>				where x = top or x = center or x = bottom

    if( text.IsEmpty() || pManager == nullptr ) return;
    UIRect rect{rc};
    if( rect.IsEmpty() ) return;

    bool bDraw = (uStyle & DT_CALCRECT) == 0;

    UIPtrArray aFontArray(10);
    UIPtrArray aColorArray(10);
    UIPtrArray aPIndentArray(10);
    UIPtrArray aVAlignArray(10);

    XRectangle rcClip = { 0 };
    if (hDC->currentRegion == nullptr) {
        rcClip.x = hDC->x11Window->x;
        rcClip.y = hDC->x11Window->y;
        rcClip.width = hDC->x11Window->width;
        rcClip.height = hDC->x11Window->height;
    }else {
        XClipBox(hDC->currentRegion,&rcClip);
    }

    XRectangle  currentRectangle = {static_cast<short>(rc.left),
        static_cast<short>(rc.top),
        static_cast<ushort>(rc.right-rc.left),
        static_cast<ushort>(rc.bottom-rc.top)};
    Region region = XCreateRegion();
    XUnionRectWithRegion(&currentRectangle,region,region);
    XftDraw *xftDraw = XftDrawCreate(hDC->x11Window->display, hDC->drawablePixmap,
                                  hDC->x11Window->visual,
                                  hDC->x11Window->colormap);
    if(bDraw){
        if (hDC->currentRegion != nullptr) {
            XIntersectRegion(region,hDC->currentRegion,region);
        }
        XftDrawSetClip(xftDraw,region);
    }

    const char *pstrText = text.GetData();

    uint32_t fontHeight = UIResourceMgr::GetInstance().GetFontHeight(iDefaultFont,hDC);
    //SelectFont(hDC, UIResourceMgr::GetInstance().GetFont(iDefaultFont));
    UIFont *selectedFont = UIResourceMgr::GetInstance().GetFont(iDefaultFont);
    uint32_t textColor = dwTextColor;
    //cairo_set_source_rgb(hDC, UIGetRValue(dwTextColor)/255.0,
    //                     UIGetGValue(dwTextColor)/255.0, UIGetBValue(dwTextColor)/255.0);
    uint32_t dwBkColor = pManager->GetDefaultSelectedBkColor();

    // If the drawstyle include a alignment, we'll need to first determine the text-size so
    // we can draw it at the correct position...
    if( ((uStyle & DT_CENTER) != 0 || (uStyle & DT_RIGHT) != 0 || (uStyle & DT_VCENTER) != 0 || (uStyle & DT_BOTTOM) != 0) && (uStyle & DT_CALCRECT) == 0 ) {
        RECT rcText = { 0, 0, 9999, 100 };
        if ((uStyle & DT_SINGLELINE) == 0) {
            rcText.right = rc.right - rc.left;
            rcText.bottom = rc.bottom - rc.top;
        }
        int nLinks = 0;
        DrawHtmlText(hDC, pManager, rcText, UIString{pstrText}, dwTextColor, nullptr, nullptr, nLinks, iDefaultFont, uStyle | DT_CALCRECT & ~DT_CENTER & ~DT_RIGHT & ~DT_VCENTER & ~DT_BOTTOM);
        if( (uStyle & DT_SINGLELINE) != 0 ){
            if( (uStyle & DT_CENTER) != 0 ) {
                rc.left = rc.left + ((rc.right - rc.left) / 2) - ((rcText.right - rcText.left) / 2);
                rc.right = rc.left + (rcText.right - rcText.left);
            }
            if( (uStyle & DT_RIGHT) != 0 ) {
                rc.left = rc.right - (rcText.right - rcText.left);
            }
        }
        if( (uStyle & DT_VCENTER) != 0 ) {
            rc.top = rc.top + ((rc.bottom - rc.top) / 2) - ((rcText.bottom - rcText.top) / 2);
            rc.bottom = rc.top + (rcText.bottom - rcText.top);
        }
        if( (uStyle & DT_BOTTOM) != 0 ) {
            rc.top = rc.bottom - (rcText.bottom - rcText.top);
        }
    }

    bool bHoverLink = false;
    UIString sHoverLink;
    POINT ptMouse = pManager->GetMousePos();
    for( int i = 0; !bHoverLink && i < nLinkRects; i++ ) {
        UIRect rcItem{*(prcLinks + i)};
        if( rcItem.IsPtIn(ptMouse) ) {
            sHoverLink = *(UIString*)(sLinks + i);
            bHoverLink = true;
        }
    }

    POINT pt = { rc.left, rc.top };
    int iLinkIndex = 0;
    int cxLine = 0;
    int cyLine = fontHeight + (long)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1);
    int cyMinHeight = 0;
    int cxMaxWidth = 0;
    POINT ptLinkStart = { 0 };
    bool bLineEnd = false;
    bool bInRaw = false;
    bool bInLink = false;
    bool bInSelected = false;
    int iLineLinkIndex = 0;

    // 排版习惯是图文底部对齐，所以每行绘制都要分两步，先计算高度，再绘制
    UIPtrArray aLineFontArray;
    UIPtrArray aLineColorArray;
    UIPtrArray aLinePIndentArray;
    UIPtrArray aLineVAlignArray;
    const char *pstrLineBegin = pstrText;
    bool bLineInRaw = false;
    bool bLineInLink = false;
    bool bLineInSelected = false;
    uint32_t iVAlign = DT_BOTTOM;
    int cxLineWidth = 0;
    int cyLineHeight = 0;
    int cxOffset = 0;
    bool bLineDraw = false; // 行的第二阶段：绘制
    while( *pstrText != '\0' ) {
        if( pt.x >= rc.right || *pstrText == '\n' || bLineEnd ) {
            if( *pstrText == '\n' ) pstrText++;
            if( bLineEnd ) bLineEnd = false;
            if( !bLineDraw ) {
                if( bInLink && iLinkIndex < nLinkRects ) {
                    ::SetRect(&prcLinks[iLinkIndex++], ptLinkStart.x, ptLinkStart.y, MIN(pt.x, rc.right), pt.y + cyLine);
                    auto *pStr1 = (UIString*)(sLinks + iLinkIndex - 1);
                    auto *pStr2 = (UIString*)(sLinks + iLinkIndex);
                    *pStr2 = *pStr1;
                }
                for( int i = iLineLinkIndex; i < iLinkIndex; i++ ) {
                    prcLinks[i].bottom = pt.y + cyLine;
                }
                if( bDraw ) {
                    bInLink = bLineInLink;
                    iLinkIndex = iLineLinkIndex;
                }
            }
            else {
                if( bInLink && iLinkIndex < nLinkRects ) iLinkIndex++;
                bLineInLink = bInLink;
                iLineLinkIndex = iLinkIndex;
            }
            if( (uStyle & DT_SINGLELINE) != 0 && (!bDraw || bLineDraw) )
                break;
            if( bDraw ) bLineDraw = !bLineDraw; // !
            pt.x = rc.left;
            cxOffset = 0;
            if (bLineDraw) {
                if( (uStyle & DT_SINGLELINE) == 0 && (uStyle & DT_CENTER) != 0 ) {
                    cxOffset = (rc.right - rc.left - cxLineWidth)/2;
                }
                else if( (uStyle & DT_SINGLELINE) == 0 && (uStyle & DT_RIGHT) != 0) {
                    cxOffset = rc.right - rc.left - cxLineWidth;
                }
            }
            else {
                pt.y += cyLine;
            }
            if( pt.y > rc.bottom && bDraw )
                break;
            ptLinkStart = pt;
            cyLine = fontHeight + (long)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1);
            if( pt.x >= rc.right )
                break;
        }
        else if( !bInRaw && ( *pstrText == '<' || *pstrText == '{' )
                 && ( pstrText[1] >= 'a' && pstrText[1] <= 'z' )
                 && ( pstrText[2] == ' ' || pstrText[2] == '>' || pstrText[2] == '}' ) ) {
            pstrText++;
            const char *pstrNextStart = nullptr;
            switch( *pstrText ) {
                case 'a':  // Link
                {
                    pstrText++;
                    while( *pstrText > '\0' && *pstrText <= ' ' ) pstrText = CharNext(pstrText);
                    if( iLinkIndex < nLinkRects && !bLineDraw ) {
                        auto *pStr = (UIString*)(sLinks + iLinkIndex);
                        pStr->Empty();
                        while( *pstrText != '\0' && *pstrText != '>' && *pstrText != '}' ) {
                            const char *pstrTemp = CharNext(pstrText);
                            while( pstrText < pstrTemp) {
                                *pStr += *pstrText++;
                            }
                        }
                    }

                    uint32_t clrColor = pManager->GetDefaultLinkFontColor();
                    if( bHoverLink && iLinkIndex < nLinkRects ) {
                        auto *pStr = (UIString*)(sLinks + iLinkIndex);
                        if( sHoverLink == *pStr ) clrColor = pManager->GetDefaultLinkHoverFontColor();
                    }

                    aColorArray.Add((LPVOID)(long)clrColor);
                    textColor = clrColor;
                    //cairo_set_source_rgb(hDC, UIGetRValue(clrColor)/255.0, UIGetGValue(clrColor)/255.0,
                    //                     UIGetBValue(clrColor)/255.0);
                    UIFont *pFontInfo = UIResourceMgr::GetInstance().GetFont(iDefaultFont);
                    if( aFontArray.GetSize() > 0 ) pFontInfo = (UIFont*)aFontArray.GetAt(aFontArray.GetSize() - 1);
                    if( !pFontInfo->GetUnderline() ) {
                        UIFont *existsFont = UIResourceMgr::GetInstance().GetFont(pFontInfo->GetFontName(),
                                                                                  pFontInfo->GetSize(),
                                                                                  pFontInfo->GetBold(),
                                                                                  true,
                                                                                  pFontInfo->GetItalic());
                        if( existsFont == nullptr ) {
                            UIResourceMgr::GetInstance().AddFont(g_iFontID,
                                                                 pFontInfo->GetFontName(),
                                                                 false,
                                                                 pFontInfo->GetSize(),
                                                                 pFontInfo->GetBold(),
                                                                 true,pFontInfo->GetItalic());
                            g_iFontID += 1;
                        }
                        pFontInfo = UIResourceMgr::GetInstance().GetFont(pFontInfo->GetFontName(),
                                                                         pFontInfo->GetSize(),
                                                                         pFontInfo->GetBold(),
                                                                         true,
                                                                         pFontInfo->GetItalic());
                        aFontArray.Add(pFontInfo);
                        fontHeight = pFontInfo->GetFontHeight(hDC);
                        selectedFont = pFontInfo;
                        //::SelectObject(hDC, pFontInfo->GetHandle());
                        cyLine = MAX((long)cyLine, fontHeight + (long)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                    }
                    ptLinkStart = pt;
                    bInLink = true;
                }
                    break;
                case 'b':  // Bold
                {
                    pstrText++;
                    UIFont *pFontInfo = UIResourceMgr::GetInstance().GetFont(iDefaultFont);
                    if( aFontArray.GetSize() > 0 ) pFontInfo = (UIFont*)aFontArray.GetAt(aFontArray.GetSize() - 1);
                    if( !pFontInfo->GetBold() ) {
                        UIFont *existsFont = UIResourceMgr::GetInstance().GetFont(pFontInfo->GetFontName(),
                                                                                  pFontInfo->GetSize(),
                                                                                  true,
                                                                                  pFontInfo->GetUnderline(),
                                                                                  pFontInfo->GetItalic());
                        if( existsFont == nullptr ) {
                            UIResourceMgr::GetInstance().AddFont(g_iFontID,
                                                                 pFontInfo->GetFontName(),
                                                                 false,
                                                                 pFontInfo->GetSize(),
                                                                 true,
                                                                 pFontInfo->GetUnderline(),
                                                                 pFontInfo->GetItalic());
                            g_iFontID += 1;
                        }
                        pFontInfo = UIResourceMgr::GetInstance().GetFont(pFontInfo->GetFontName(),
                                                                         pFontInfo->GetSize(),
                                                                         true,
                                                                         pFontInfo->GetUnderline(),
                                                                         pFontInfo->GetItalic());
                        //pFontInfo = pManager->GetFontInfo(hFont);
                        aFontArray.Add(pFontInfo);
                        //GetFontTextMetrics(hDC,pFontInfo->GetHandle(),&tm);
                        //pTm = &pFontInfo->tm;
                        fontHeight = pFontInfo->GetFontHeight(hDC);
                        selectedFont = pFontInfo;
                        //::SelectObject(hDC, pFontInfo->GetHandle());
                        cyLine = MAX((long)cyLine, fontHeight + (long)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                    }
                }
                    break;
                case 'c':  // Color
                {
                    pstrText++;
                    while( *pstrText > '\0' && *pstrText <= ' ' ) pstrText = CharNext(pstrText);
                    if( *pstrText == '#') pstrText++;
                    uint32_t clrColor = strtol(pstrText, const_cast<char**>(&pstrText), 16);
                    aColorArray.Add((LPVOID)(long)clrColor);
                    textColor = clrColor;
                    //cairo_set_source_rgb(hDC, UIGetRValue(clrColor)/255.0, UIGetGValue(clrColor)/255.0, UIGetBValue(clrColor)/255.0);
                }
                    break;
                case 'f':  // Font
                {
                    pstrText++;
                    while( *pstrText > '\0' && *pstrText <= ' ' ) pstrText = CharNext(pstrText);
                    const char *pstrTemp = pstrText;
                    int iFont = (int) strtol(pstrText, const_cast<char**>(&pstrText), 10);
                    //if( isdigit(*pstrText) ) { // debug版本会引起异常
                    if( pstrTemp != pstrText ) {
                        UIFont* pFontInfo = UIResourceMgr::GetInstance().GetFont(iFont);//pManager->GetFontInfo(iFont);
                        aFontArray.Add(pFontInfo);
                        fontHeight = pFontInfo->GetFontHeight(hDC);
                        //::GetFontTextMetrics(hDC, pFontInfo->GetHandle(), &tm);
                        selectedFont = pFontInfo;
                        //::SelectObject(hDC, pFontInfo->GetHandle());
                    }
                    else {
                        UIString sFontName;
                        int iFontSize = 10;
                        UIString sFontAttr;
                        bool bBold = false;
                        bool bUnderline = false;
                        bool bItalic = false;
                        while( *pstrText != '\0' && *pstrText != '>' && *pstrText != '}' && *pstrText != ' ' ) {
                            pstrTemp = CharNext(pstrText);
                            while( pstrText < pstrTemp) {
                                sFontName += *pstrText++;
                            }
                        }
                        while( *pstrText > '\0' && *pstrText <= ' ' ) pstrText = CharNext(pstrText);
                        if( isdigit(*pstrText) ) {
                            iFontSize = (int) strtol(pstrText, const_cast<char**>(&pstrText), 10);
                        }
                        while( *pstrText > '\0' && *pstrText <= ' ' ) pstrText = CharNext(pstrText);
                        while( *pstrText != '\0' && *pstrText != '>' && *pstrText != '}' ) {
                            pstrTemp = CharNext(pstrText);
                            while( pstrText < pstrTemp) {
                                sFontAttr += *pstrText++;
                            }
                        }
                        sFontAttr.MakeLower();
                        if( sFontAttr.Find("bold") >= 0 ) bBold = true;
                        if( sFontAttr.Find("underline") >= 0 ) bUnderline = true;
                        if( sFontAttr.Find("italic") >= 0 ) bItalic = true;
                        UIFont *pFontInfo = UIResourceMgr::GetInstance().GetFont(UIString{sFontName},
                                                                                 iFontSize,bBold,bUnderline,bItalic);
                        if( pFontInfo == nullptr ) {
                            UIResourceMgr::GetInstance().AddFont(g_iFontID,UIString{sFontName},
                                                                 false,
                                                                 iFontSize,
                                                                 bBold,
                                                                 bUnderline,
                                                                 bItalic);
                            pFontInfo = UIResourceMgr::GetInstance().GetFont(UIString{sFontName},
                                                                             iFontSize,bBold,bUnderline,bItalic);
                            //hFont = pManager->AddFont(g_iFontID, sFontName, iFontSize, bBold, bUnderline, bItalic);
                            g_iFontID += 1;
                        }
                        aFontArray.Add(pFontInfo);
                        //GetFontTextMetrics(hDC, pFontInfo->GetHandle(), &tm);
                        fontHeight = pFontInfo->GetFontHeight(hDC);
                        selectedFont = pFontInfo;
                        //::SelectObject(hDC, pFontInfo->GetHandle());
                    }
                    cyLine = MAX((long)cyLine, fontHeight + (long)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                }
                    break;
                case 'i':  // Italic or Image
                {
                    pstrNextStart = pstrText - 1;
                    pstrText++;
                    UIString sImageString {pstrText};
                    int iWidth = 0;
                    int iHeight = 0;
                    while( *pstrText > '\0' && *pstrText <= ' ' ) pstrText = CharNext(pstrText);
                    const TImageInfo* pImageInfo = NULL;
                    UIString sName;
                    while( *pstrText != '\0' && *pstrText != '>' && *pstrText != '}' && *pstrText != ' ' ) {
                        const char *pstrTemp = CharNext(pstrText);
                        while( pstrText < pstrTemp) {
                            sName += *pstrText++;
                        }
                    }
                    if( sName.IsEmpty() ) { // Italic
                        pstrNextStart = nullptr;
                        UIFont *pFontInfo = UIResourceMgr::GetInstance().GetFont(iDefaultFont);
                        if( aFontArray.GetSize() > 0 ) pFontInfo = (UIFont*)aFontArray.GetAt(aFontArray.GetSize() - 1);
                        if( !pFontInfo->GetItalic() ) {
                            UIFont *existsFont = UIResourceMgr::GetInstance().GetFont(pFontInfo->GetFontName(),
                                                                                      pFontInfo->GetSize(),
                                                                                      pFontInfo->GetBold(),
                                                                                      pFontInfo->GetUnderline(),
                                                                                      true);
                            if( existsFont == nullptr ) {
                                UIResourceMgr::GetInstance().AddFont(g_iFontID,
                                                                     pFontInfo->GetFontName(),
                                                                     false,
                                                                     pFontInfo->GetSize(),
                                                                     pFontInfo->GetBold(),
                                                                     pFontInfo->GetUnderline(),true);
                                g_iFontID += 1;
                            }
                            pFontInfo = UIResourceMgr::GetInstance().GetFont(pFontInfo->GetFontName(),
                                                                             pFontInfo->GetSize(),
                                                                             pFontInfo->GetBold(),
                                                                             pFontInfo->GetUnderline(),
                                                                             true);
                            aFontArray.Add(pFontInfo);
                            //GetFontTextMetrics(hDC,pFontInfo->GetHandle(),&tm);
                            fontHeight = pFontInfo->GetFontHeight(hDC);
                            selectedFont = pFontInfo;
                            //::SelectObject(hDC, pFontInfo->GetHandle());
                            cyLine = MAX((long)cyLine, fontHeight + (long)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                        }
                    }
                    else {
                        while( *pstrText > '\0' && *pstrText <= ' ' ) pstrText = CharNext(pstrText);
                        int iImageListNum = (int) strtol(pstrText, const_cast<char**>(&pstrText), 10);
                        if( iImageListNum <= 0 ) iImageListNum = 1;
                        while( *pstrText > '\0' && *pstrText <= ' ' ) pstrText = CharNext(pstrText);
                        int iImageListIndex = (int) strtol(pstrText, const_cast<char**>(&pstrText), 10);
                        if( iImageListIndex < 0 || iImageListIndex >= iImageListNum ) iImageListIndex = 0;

                        if( strstr(sImageString.GetData(), "file=\'") != NULL || strstr(sImageString.GetData(), "res=\'") != NULL ) {
                            UIString sImageResType;
                            UIString sImageName;
                            const char *pStrImage = sImageString.GetData();
                            UIString sItem;
                            UIString sValue;
                            while( *pStrImage != '\0' ) {
                                sItem.Empty();
                                sValue.Empty();
                                while( *pStrImage > '\0' && *pStrImage <= ' ' ) pStrImage = CharNext(pStrImage);
                                while( *pStrImage != '\0' && *pStrImage != '=' && *pStrImage > ' ' ) {
                                    const char *pstrTemp = CharNext(pStrImage);
                                    while( pStrImage < pstrTemp) {
                                        sItem += *pStrImage++;
                                    }
                                }
                                while( *pStrImage > '\0' && *pStrImage <= ' ' ) pStrImage = CharNext(pStrImage);
                                if( *pStrImage++ != '=' ) break;
                                while( *pStrImage > '\0' && *pStrImage <= ' ' ) pStrImage = CharNext(pStrImage);
                                if( *pStrImage++ != '\'' ) break;
                                while( *pStrImage != '\0' && *pStrImage != '\'' ) {
                                    const char *pstrTemp = CharNext(pStrImage);
                                    while( pStrImage < pstrTemp) {
                                        sValue += *pStrImage++;
                                    }
                                }
                                if( *pStrImage++ != '\'' ) break;
                                if( !sValue.IsEmpty() ) {
                                    if( sItem == "file" || sItem == "res" ) {
                                        sImageName = sValue;
                                    }
                                    else if( sItem == "restype" ) {
                                        sImageResType = sValue;
                                    }
                                }
                                if( *pStrImage++ != ' ' ) break;
                            }
                            pImageInfo = UIResourceMgr::GetInstance().GetImage(UIString{sImageName},true);
                        }
                        else
                            pImageInfo = UIResourceMgr::GetInstance().GetImage(UIString{sName},true);

                        if( pImageInfo ) {
                            iWidth = pImageInfo->nX;
                            iHeight = pImageInfo->nY;
                            if( iImageListNum > 1 ) iWidth /= iImageListNum;

                            if( pt.x + iWidth > rc.right && pt.x > rc.left && (uStyle & DT_SINGLELINE) == 0 ) {
                                bLineEnd = true;
                                cxLine = pt.x - rc.left;
                            }
                            else {
                                pstrNextStart = nullptr;
                                if( bDraw && bLineDraw ) {
                                    UIRect rcImage(pt.x + cxOffset, pt.y + cyLineHeight - iHeight, pt.x + + cxOffset + iWidth, pt.y + cyLineHeight);
                                    iVAlign = DT_BOTTOM;
                                    if (aVAlignArray.GetSize() > 0) iVAlign = (uint32_t)(long)aVAlignArray.GetAt(aVAlignArray.GetSize() - 1);
                                    if (iVAlign == DT_VCENTER) {
                                        if( iHeight < cyLineHeight ) {
                                            rcImage.bottom -= (cyLineHeight - iHeight) / 2;
                                            rcImage.top = rcImage.bottom -  iHeight;
                                        }
                                    }
                                    else if (iVAlign == DT_TOP) {
                                        if( iHeight < cyLineHeight ) {
                                            rcImage.bottom = pt.y + iHeight;
                                            rcImage.top = pt.y;
                                        }
                                    }

                                    UIRect rcBmpPart(0, 0, iWidth, iHeight);
                                    rcBmpPart.left = iWidth * iImageListIndex;
                                    rcBmpPart.right = iWidth * (iImageListIndex + 1);
                                    UIRect rcCorner(0, 0, 0, 0);
                                    DrawImage(hDC, pImageInfo->hBitmap, rcImage, rcImage, rcBmpPart, rcCorner, \
                                        pImageInfo->bAlpha, 255);
                                }

                                cyLine = MAX(iHeight, cyLine);
                                pt.x += iWidth;
                                cxMaxWidth = MAX((long)cxMaxWidth, pt.x);
                                cxLine = pt.x - rc.left;
                                cyMinHeight = pt.y + iHeight;
                            }
                        }
                        else pstrNextStart = nullptr;
                    }
                }
                    break;
                case 'n':  // Newline
                {
                    pstrText++;
                    if( (uStyle & DT_SINGLELINE) != 0 ) break;
                    bLineEnd = true;
                }
                    break;
                case 'p':  // Paragraph
                {
                    pstrText++;
                    if( pt.x > rc.left ) bLineEnd = true;
                    while( *pstrText > '\0' && *pstrText <= ' ' ) pstrText = CharNext(pstrText);
                    int cyLineExtra = (int)strtol(pstrText, const_cast<char**>(&pstrText), 10);
                    aPIndentArray.Add((LPVOID)(long)cyLineExtra);
                    cyLine = MAX((long)cyLine, fontHeight + cyLineExtra);
                }
                    break;
                case 'v':  // Vertical Align
                {
                    pstrText++;
                    while( *pstrText > '\0' && *pstrText <= ' ' ) pstrText = CharNext(pstrText);
                    UIString sVAlignStyle;
                    while( *pstrText != '\0' && *pstrText != '>' && *pstrText != '}' ) {
                        const char *pstrTemp = CharNext(pstrText);
                        while( pstrText < pstrTemp) {
                            sVAlignStyle += *pstrText++;
                        }
                    }

                    uint32_t iVAlign = DT_BOTTOM;
                    if (sVAlignStyle.CompareNoCase(UIString{"center"}) == 0) iVAlign = DT_VCENTER;
                    else if (sVAlignStyle.CompareNoCase(UIString{"top"}) == 0) iVAlign = DT_TOP;
                    aVAlignArray.Add((LPVOID)(long)iVAlign);
                }
                    break;
                case 'r':  // Raw Text
                {
                    pstrText++;
                    bInRaw = true;
                }
                    break;
                case 's':  // Selected text background color
                {
                    pstrText++;
                    bInSelected = !bInSelected;
                    if( bDraw && bLineDraw ) {
                        //TODO bkMode transparent OPAQUE
                        //if( bInSelected ) ::SetBkMode(hDC, OPAQUE);
                        //else ::SetBkMode(hDC, TRANSPARENT);
                    }
                }
                    break;
                case 'u':  // Underline text
                {
                    pstrText++;
                    UIFont *pFontInfo = UIResourceMgr::GetInstance().GetFont(iDefaultFont);
                    if( aFontArray.GetSize() > 0 ) pFontInfo = (UIFont*)aFontArray.GetAt(aFontArray.GetSize() - 1);
                    if( !pFontInfo->GetUnderline() ) {
                        UIFont *existsFont = UIResourceMgr::GetInstance().GetFont(pFontInfo->GetFontName(),
                                                                                  pFontInfo->GetSize(),
                                                                                  pFontInfo->GetBold(),
                                                                                  true,
                                                                                  pFontInfo->GetItalic());
                        if( existsFont == nullptr ) {
                            UIResourceMgr::GetInstance().AddFont(g_iFontID,
                                                                 pFontInfo->GetFontName(),
                                                                 false,
                                                                 pFontInfo->GetSize(),
                                                                 pFontInfo->GetBold(),
                                                                 true,pFontInfo->GetItalic());
                            g_iFontID += 1;
                        }
                        pFontInfo = UIResourceMgr::GetInstance().GetFont(pFontInfo->GetFontName(),
                                                                         pFontInfo->GetSize(),
                                                                         pFontInfo->GetBold(),
                                                                         true,
                                                                         pFontInfo->GetItalic());
                        aFontArray.Add(pFontInfo);
                        fontHeight = pFontInfo->GetFontHeight(hDC);
                        //GetFontTextMetrics(hDC,pFontInfo->GetHandle(),&tm);
                        selectedFont = pFontInfo;
                        //::SelectObject(hDC, pFontInfo->GetHandle());
                        cyLine = MAX((long)cyLine, fontHeight + (long)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                    }
                }
                    break;
                case 'x':  // X Indent
                {
                    pstrText++;
                    while( *pstrText > '\0' && *pstrText <= ' ' ) pstrText = CharNext(pstrText);
                    int iWidth = (int) strtol(pstrText, const_cast<char**>(&pstrText), 10);
                    pt.x += iWidth;
                }
                    break;
                case 'y':  // Y Indent
                {
                    pstrText++;
                    while( *pstrText > '\0' && *pstrText <= ' ' ) pstrText = CharNext(pstrText);
                    cyLine = (int) strtol(pstrText, const_cast<char**>(&pstrText), 10);
                }
                    break;
            }
            if( pstrNextStart != NULL ) pstrText = pstrNextStart;
            else {
                while( *pstrText != '\0' && *pstrText != '>' && *pstrText != '}' ) pstrText = CharNext(pstrText);
                pstrText = CharNext(pstrText);
            }
        }
        else if( !bInRaw && ( *pstrText == '<' || *pstrText == '{' ) && pstrText[1] == '/' )
        {
            pstrText++;
            pstrText++;
            switch( *pstrText )
            {
                case 'c':
                {
                    pstrText++;
                    aColorArray.Remove(aColorArray.GetSize() - 1);
                    uint32_t clrColor = dwTextColor;
                    if( aColorArray.GetSize() > 0 ) clrColor = (int)(long)aColorArray.GetAt(aColorArray.GetSize() - 1);
                    textColor = clrColor;
                    //cairo_set_source_rgb(hDC, UIGetRValue(clrColor)/255.0, UIGetGValue(clrColor)/255.0, UIGetBValue(clrColor)/255.0);
                }
                    break;
                case 'p':
                    pstrText++;
                    if( pt.x > rc.left ) bLineEnd = true;
                    aPIndentArray.Remove(aPIndentArray.GetSize() - 1);
                    cyLine = MAX((long)cyLine, fontHeight + (long)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                    break;
                case 'v':
                    pstrText++;
                    aVAlignArray.Remove(aVAlignArray.GetSize() - 1);
                    break;
                case 's':
                {
                    pstrText++;
                    bInSelected = !bInSelected;
                    if( bDraw && bLineDraw ) {
                        // TODO Set bk Mode Transaprent OPAQUE
                        //if( bInSelected ) ::SetBkMode(hDC, OPAQUE);
                        //else ::SetBkMode(hDC, TRANSPARENT);
                    }
                }
                    break;
                case 'a':
                {
                    if( iLinkIndex < nLinkRects ) {
                        if( !bLineDraw ) ::SetRect(&prcLinks[iLinkIndex], ptLinkStart.x, ptLinkStart.y, MIN(pt.x, rc.right), pt.y + fontHeight);
                        iLinkIndex++;
                    }
                    aColorArray.Remove(aColorArray.GetSize() - 1);
                    uint32_t clrColor = dwTextColor;
                    if( aColorArray.GetSize() > 0 ) clrColor = (int)(long)aColorArray.GetAt(aColorArray.GetSize() - 1);
                    textColor = clrColor;
                    //cairo_set_source_rgb(hDC, UIGetRValue(clrColor)/255.0, UIGetGValue(clrColor)/255.0, UIGetBValue(clrColor)/255.0);
                    bInLink = false;
                }
                case 'b':
                case 'f':
                case 'i':
                case 'u':
                {
                    pstrText++;
                    aFontArray.Remove(aFontArray.GetSize() - 1);
                    UIFont* pFontInfo = (UIFont*)aFontArray.GetAt(aFontArray.GetSize() - 1);
                    if( pFontInfo == NULL ) pFontInfo = UIResourceMgr::GetInstance().GetFont(iDefaultFont);
                    /*if( tm.tmItalic && (!pFontInfo->GetItalic()) ) {
                        ABC abc;
                        ::GetCharABCWidths(hDC, ' ', ' ', &abc);
                        pt.x += abc.abcC / 2; // 简单修正一下斜体混排的问题, 正确做法应该是http://support.microsoft.com/kb/244798/en-us
                    }*/
                    fontHeight = pFontInfo->GetFontHeight(hDC);
                    //GetFontTextMetrics(hDC,pFontInfo->GetHandle(),&tm);
                    selectedFont = pFontInfo;
                    //::SelectObject(hDC, pFontInfo->GetHandle());
                    cyLine = MAX((long)cyLine, fontHeight + (long)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                }
                    break;
            }
            while( *pstrText != '\0' && *pstrText != '>' && *pstrText != '}' ) pstrText = CharNext(pstrText);
            pstrText = CharNext(pstrText);
        }
        else if( !bInRaw &&  *pstrText == '<' && pstrText[2] == '>' && (pstrText[1] == '{'  || pstrText[1] == '}') )
        {
            SIZE szSpace = { 0 };
            GetTextExtentPoint(hDC,&pstrText[1], 1,selectedFont,&szSpace);
            //::GetTextExtentPoint32(hDC, &pstrText[1], 1, &szSpace);
            if( bDraw && bLineDraw ) {
                iVAlign = DT_BOTTOM;
                if (aVAlignArray.GetSize() > 0) iVAlign = (uint32_t)(long)aVAlignArray.GetAt(aVAlignArray.GetSize() - 1);
                if (iVAlign == DT_VCENTER)
                    ::TextOut(xftDraw,hDC, textColor, pt.x + cxOffset, pt.y + (cyLineHeight - fontHeight) / 2,
                              &pstrText[1], 1,selectedFont);
                else if (iVAlign == DT_TOP) ::TextOut(xftDraw,hDC,textColor, pt.x + cxOffset, pt.y, &pstrText[1], 1,selectedFont);
                else
                    ::TextOut(xftDraw,hDC, textColor, pt.x + cxOffset, pt.y + cyLineHeight - fontHeight,
                              &pstrText[1], 1,selectedFont);
            }
            pt.x += szSpace.cx;
            cxMaxWidth = MAX((long)cxMaxWidth, pt.x);
            cxLine = pt.x - rc.left;
            pstrText++;pstrText++;pstrText++;
        }
        else if( !bInRaw &&  *pstrText == '{' && pstrText[2] == '}' && (pstrText[1] == '<'  || pstrText[1] == '>') )
        {
            SIZE szSpace = { 0 };
            ::GetTextExtentPoint(hDC,&pstrText[1], 1,selectedFont,&szSpace);
            if( bDraw && bLineDraw ) {
                iVAlign = DT_BOTTOM;
                if (aVAlignArray.GetSize() > 0) iVAlign = (uint32_t)(long)aVAlignArray.GetAt(aVAlignArray.GetSize() - 1);
                if (iVAlign == DT_VCENTER)
                    ::TextOut(xftDraw,hDC, textColor, pt.x + cxOffset, pt.y + (cyLineHeight - fontHeight) / 2,
                              &pstrText[1], 1,selectedFont);
                else if (iVAlign == DT_TOP) ::TextOut(xftDraw,hDC, textColor, pt.x + cxOffset, pt.y, &pstrText[1], 1,selectedFont);
                else
                    ::TextOut(xftDraw,hDC, textColor, pt.x + cxOffset, pt.y + cyLineHeight - fontHeight,
                              &pstrText[1], 1,selectedFont);
            }
            pt.x += szSpace.cx;
            cxMaxWidth = MAX((long)cxMaxWidth, pt.x);
            cxLine = pt.x - rc.left;
            pstrText++;pstrText++;pstrText++;
        }
        else if( !bInRaw &&  *pstrText == ' ' )
        {
            SIZE szSpace = { 0 };
            ::GetTextExtentPoint(hDC, " ", 1, selectedFont,&szSpace);
            // Still need to paint the space because the font might have
            // underline formatting.
            if( bDraw && bLineDraw ) {
                iVAlign = DT_BOTTOM;
                if (aVAlignArray.GetSize() > 0) iVAlign = (uint32_t)(long)aVAlignArray.GetAt(aVAlignArray.GetSize() - 1);
                if (iVAlign == DT_VCENTER)
                    ::TextOut(xftDraw,hDC, textColor, pt.x + cxOffset, pt.y + (cyLineHeight - fontHeight) / 2,
                              " ", 1,selectedFont);
                else if (iVAlign == DT_TOP) ::TextOut(xftDraw,hDC, textColor, pt.x + cxOffset, pt.y, " ", 1,selectedFont);
                else ::TextOut(xftDraw,hDC, textColor, pt.x + cxOffset, pt.y + cyLineHeight - fontHeight, " ", 1,selectedFont);
            }
            pt.x += szSpace.cx;
            cxMaxWidth = MAX((long)cxMaxWidth, pt.x);
            cxLine = pt.x - rc.left;
            pstrText++;
        }
        else
        {
            int cchChars = 0;
            int cchSize = 0;
            int cchLastGoodWord = 0;
            int cchLastGoodSize = 0;
            const char *p = pstrText;
            const char *pstrNext;
            SIZE szText = { 0 };
            if( !bInRaw && *p == '<' || *p == '{' ) p++, cchChars++, cchSize++;
            while( *p != '\0' && *p != '\n' ) {
                // This part makes sure that we're word-wrapping if needed or providing support
                // for DT_END_ELLIPSIS. Unfortunately the GetTextExtentPoint32() call is pretty
                // slow when repeated so often.
                // TODO: Rewrite and use GetTextExtentExPoint() instead!
                if( bInRaw ) {
                    if( ( *p == '<' || *p == '{' ) && p[1] == '/'
                        && p[2] == 'r' && ( p[3] == '>' || p[3] == '}' ) ) {
                        p += 4;
                        bInRaw = false;
                        break;
                    }
                }
                else {
                    if( *p == '<' || *p == '{' ) break;
                }
                pstrNext = CharNext(p);
                cchChars++;
                cchSize += (int)(pstrNext - p);
                szText.cx = cchChars * fontHeight;
                if( pt.x + szText.cx >= rc.right ) {
                    ::GetTextExtentPoint(hDC, pstrText, cchSize, selectedFont,&szText);
                }
                if( pt.x + szText.cx > rc.right ) {
                    if( pt.x + szText.cx > rc.right && cchChars > 1) {
                        cchChars--;
                        cchSize -= (int)(pstrNext - p);
                    }
                    if( (uStyle & DT_WORDBREAK) != 0 && cchLastGoodWord > 0 ) {
                        cchChars = cchLastGoodWord;
                        cchSize = cchLastGoodSize;
                    }
                    if( (uStyle & DT_END_ELLIPSIS) != 0 && cchChars > 0 ) {
                        cchChars -= 1;
                        const char *pstrPrev = ::CharPrev(pstrText, p);
                        if( cchChars > 0 ) {
                            cchChars -= 1;
                            pstrPrev = ::CharPrev(pstrText, pstrPrev);
                            cchSize -= (int)(p - pstrPrev);
                        }
                        else
                            cchSize -= (int)(p - pstrPrev);
                        pt.x = rc.right;
                    }
                    bLineEnd = true;
                    cxMaxWidth = MAX((long)cxMaxWidth, pt.x);
                    cxLine = pt.x - rc.left;
                    break;
                }
                if (!( ( p[0] >= 'a' && p[0] <= 'z' ) || ( p[0] >= 'A' && p[0] <= 'Z' ) )) {
                    cchLastGoodWord = cchChars;
                    cchLastGoodSize = cchSize;
                }
                if( *p == ' ' ) {
                    cchLastGoodWord = cchChars;
                    cchLastGoodSize = cchSize;
                }
                p = CharNext(p);
            }

            ::GetTextExtentPoint(hDC, pstrText, cchSize,selectedFont, &szText);
            if( bDraw && bLineDraw ) {
                iVAlign = DT_BOTTOM;
                if (aVAlignArray.GetSize() > 0) iVAlign = (uint32_t)(long)aVAlignArray.GetAt(aVAlignArray.GetSize() - 1);
                if (iVAlign == DT_VCENTER)
                    ::TextOut(xftDraw,hDC, textColor, pt.x + cxOffset, pt.y + (cyLineHeight - fontHeight) / 2,
                              pstrText, cchSize,selectedFont);
                else if (iVAlign == DT_TOP) ::TextOut(xftDraw,hDC, textColor, pt.x + cxOffset, pt.y, pstrText, cchSize,selectedFont);
                else
                    ::TextOut(xftDraw,hDC, textColor, pt.x + cxOffset, pt.y + cyLineHeight - fontHeight,
                              pstrText, cchSize,selectedFont);

                if( pt.x >= rc.right && (uStyle & DT_END_ELLIPSIS) != 0 ) {
                    if (iVAlign == DT_VCENTER) ::TextOut(xftDraw,hDC, textColor, pt.x + cxOffset + szText.cx, pt.y + (cyLineHeight -
                                                                                                              fontHeight) /
                                                                                                             2, "...", 3,selectedFont);
                    else if (iVAlign == DT_TOP) ::TextOut(xftDraw,hDC, textColor, pt.x + cxOffset + szText.cx, pt.y, "...", 3,selectedFont);
                    else
                        ::TextOut(xftDraw,hDC, textColor, pt.x + cxOffset + szText.cx,
                                  pt.y + cyLineHeight - fontHeight, "...", 3,selectedFont);
                }
            }
            pt.x += szText.cx;
            cxMaxWidth = MAX((long)cxMaxWidth, pt.x);
            cxLine = pt.x - rc.left;
            pstrText += cchSize;
        }

        if( pt.x >= rc.right || *pstrText == '\n' || *pstrText == '\0' ) bLineEnd = true;
        if( bDraw && bLineEnd ) {
            if( !bLineDraw ) {
                aFontArray.Resize(aLineFontArray.GetSize());
                memmove(aFontArray.GetData(), aLineFontArray.GetData(), aLineFontArray.GetSize() * sizeof(LPVOID));
                aColorArray.Resize(aLineColorArray.GetSize());
                memmove(aColorArray.GetData(), aLineColorArray.GetData(), aLineColorArray.GetSize() * sizeof(LPVOID));
                aPIndentArray.Resize(aLinePIndentArray.GetSize());
                memmove(aPIndentArray.GetData(), aLinePIndentArray.GetData(), aLinePIndentArray.GetSize() * sizeof(LPVOID));
                aVAlignArray.Resize(aLineVAlignArray.GetSize());
                memmove(aVAlignArray.GetData(), aLineVAlignArray.GetData(), aLineVAlignArray.GetSize() * sizeof(LPVOID));

                cxLineWidth = cxLine;
                cyLineHeight = cyLine;
                pstrText = pstrLineBegin;
                bInRaw = bLineInRaw;
                bInSelected = bLineInSelected;

                uint32_t clrColor = dwTextColor;
                if( aColorArray.GetSize() > 0 ) clrColor = (uint32_t)(long)aColorArray.GetAt(aColorArray.GetSize() - 1);
                textColor = clrColor;
                //cairo_set_source_rgb(hDC, UIGetRValue(clrColor)/255.0, UIGetGValue(clrColor)/255.0, UIGetBValue(clrColor)/255.0);
                auto* pFontInfo = (UIFont*)aFontArray.GetAt(aFontArray.GetSize() - 1);
                if( pFontInfo == NULL ) pFontInfo = UIResourceMgr::GetInstance().GetFont(iDefaultFont);
                //GetFontTextMetrics(hDC, pFontInfo->GetHandle(), &tm);
                fontHeight = pFontInfo->GetFontHeight(hDC);
                selectedFont = pFontInfo;
                //::SelectObject(hDC, pFontInfo->GetHandle());
                //TODO BKMode
                //if( bInSelected ) ::SetBkMode(hDC, OPAQUE);
            }
            else {
                aLineFontArray.Resize(aFontArray.GetSize());
                memmove(aLineFontArray.GetData(), aFontArray.GetData(), aFontArray.GetSize() * sizeof(LPVOID));
                aLineColorArray.Resize(aColorArray.GetSize());
                memmove(aLineColorArray.GetData(), aColorArray.GetData(), aColorArray.GetSize() * sizeof(LPVOID));
                aLinePIndentArray.Resize(aPIndentArray.GetSize());
                memmove(aLinePIndentArray.GetData(), aPIndentArray.GetData(), aPIndentArray.GetSize() * sizeof(LPVOID));
                aLineVAlignArray.Resize(aVAlignArray.GetSize());
                memmove(aLineVAlignArray.GetData(), aVAlignArray.GetData(), aVAlignArray.GetSize() * sizeof(LPVOID));

                pstrLineBegin = pstrText;
                bLineInSelected = bInSelected;
                bLineInRaw = bInRaw;
            }
        }

        assert(iLinkIndex<=nLinkRects);
    }

    nLinkRects = iLinkIndex;

    // Return size of text when requested
    if( (uStyle & DT_CALCRECT) != 0 ) {
        rc.bottom = MAX((long)cyMinHeight, pt.y + cyLine);
        rc.right = MIN(rc.right, (long)cxMaxWidth);
    }

    XDestroyRegion(region);
    XftDrawDestroy(xftDraw);
}