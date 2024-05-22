#include <UIRenderEngine.h>
#include <algorithm>
#include <cstdint>
#include <cassert>
#include <cstring>
#include <UIRect.h>
#include <UIPaintManager.h>
#include "UIResourceMgr.h"

#define RES_TYPE_COLOR "*COLOR*"

#ifndef RGB
#define RGB(r,g,b) ((((uint8_t)(r)|((uint16_t)((uint8_t)(g))<<8))|(((uint32_t)(uint8_t)(b))<<16)))
#endif

#ifndef GetRValue
#define GetRValue(rgb) ((rgb)&0xFF)
#endif

#ifndef GetGValue
#define GetGValue(rgb) (((rgb)>>8)&0xFF)
#endif

#ifndef GetBValue
#define GetBValue(rgb) (((rgb)>>16)&0xFF)
#endif

static const float OneThird = 1.0f / 3;

static void RGBtoHSL(uint32_t ARGB, float* H, float* S, float* L) {
    const float
            R = (float)GetRValue(ARGB),
            G = (float)GetGValue(ARGB),
            B = (float)GetBValue(ARGB),
            nR = (R<0?0:(R>255?255:R))/255,
            nG = (G<0?0:(G>255?255:G))/255,
            nB = (B<0?0:(B>255?255:B))/255,
            m = min(min(nR,nG),nB),
            M = max(max(nR,nG),nB);
    *L = (m + M)/2;
    if (M==m) *H = *S = 0;
    else {
        const float
                f = (nR==m)?(nG-nB):((nG==m)?(nB-nR):(nR-nG)),
                i = (nR==m)?3.0f:((nG==m)?5.0f:1.0f);
        *H = (i-f/(M-m));
        if (*H>=6) *H-=6;
        *H*=60;
        *S = (2*(*L)<=1)?((M-m)/(M+m)):((M-m)/(2-M-m));
    }
}

static void HSLtoRGB(uint32_t* ARGB, float H, float S, float L) {
    const float
            q = 2*L<1?L*(1+S):(L+S-L*S),
            p = 2*L-q,
            h = H/360,
            tr = h + OneThird,
            tg = h,
            tb = h - OneThird,
            ntr = tr<0?tr+1:(tr>1?tr-1:tr),
            ntg = tg<0?tg+1:(tg>1?tg-1:tg),
            ntb = tb<0?tb+1:(tb>1?tb-1:tb),
            B = 255*(6*ntr<1?p+(q-p)*6*ntr:(2*ntr<1?q:(3*ntr<2?p+(q-p)*6*(2.0f*OneThird-ntr):p))),
            G = 255*(6*ntg<1?p+(q-p)*6*ntg:(2*ntg<1?q:(3*ntg<2?p+(q-p)*6*(2.0f*OneThird-ntg):p))),
            R = 255*(6*ntb<1?p+(q-p)*6*ntb:(2*ntb<1?q:(3*ntb<2?p+(q-p)*6*(2.0f*OneThird-ntb):p)));
    *ARGB &= 0xFF000000;
    *ARGB |= RGB( (uint8_t)(R<0?0:(R>255?255:R)), (uint8_t)(G<0?0:(G>255?255:G)), (uint8_t)(B<0?0:(B>255?255:B)) );
}

uint32_t UIRenderEngine::AdjustColor(uint32_t color, short H, short S, short L) {
    if( H == 180 && S == 100 && L == 100 ) return color;
    float fH, fS, fL;
    float S1 = S / 100.0f;
    float L1 = L / 100.0f;
    RGBtoHSL(color, &fH, &fS, &fL);
    fH += (H - 180);
    fH = fH > 0 ? fH : fH + 360;
    fS *= S1;
    fL *= L1;
    HSLtoRGB(&color, fH, fS, fL);
    return color;
}

bool UIRenderEngine::DrawImage(HANDLE_DC hdc, UIPaintManager *manager, const RECT &rcItem, const RECT &rcPaint,
                               TDrawInfo &drawInfo) {
    // 1、aaa.jpg
    // 2、file='aaa.jpg' res='' restype='0' dest='0,0,0,0' source='0,0,0,0' scale9='0,0,0,0'
    // mask='#FF0000' fade='255' hole='false' xtiled='false' ytiled='false' hsl='false'
    if( manager == nullptr ) return true;
    if( drawInfo.pImageInfo == nullptr ) {
        if( drawInfo.bLoaded ) return false;
        drawInfo.bLoaded = true;
        if( drawInfo.sDrawString.IsEmpty() ) return false;

        bool bUseRes = false;
        UIString sImageName = drawInfo.sDrawString;
        UIString sImageResType;
        uint32_t dwMask = 0;
        bool bUseHSL = false;

        UIString sItem;
        UIString sValue;
        char *pstr = nullptr;
        const char *pstrImage = drawInfo.sDrawString.GetData();
        while( *pstrImage != '\0' ) {
            sItem.Empty();
            sValue.Empty();
            while( *pstrImage > '\0' && *pstrImage <= ' ' ) pstrImage = CharNext(pstrImage);
            while( *pstrImage != '\0' && *pstrImage != '=' && *pstrImage > ' ' ) {
                const char * pstrTemp = CharNext(pstrImage);
                while( pstrImage < pstrTemp) {
                    sItem += *pstrImage++;
                }
            }
            while( *pstrImage > '\0' && *pstrImage <= ' ' ) pstrImage = CharNext(pstrImage);
            if( *pstrImage++ != '=' ) break;
            while( *pstrImage > '\0' && *pstrImage <= ' ' ) pstrImage = CharNext(pstrImage);
            if( *pstrImage++ != '\'' ) break;
            while( *pstrImage != '\0' && *pstrImage != '\'' ) {
                const char *pstrTemp = CharNext(pstrImage);
                while( pstrImage < pstrTemp) {
                    sValue += *pstrImage++;
                }
            }
            if( *pstrImage++ != '\'' ) break;
            if( !sValue.IsEmpty() ) {
                if( sItem == "file" ) {
                    sImageName = sValue;
                }
                else if( sItem == "res" ) {
                    bUseRes = true;
                    sImageName = sValue;
                }
                else if( sItem == "restype" ) {
                    sImageResType = sValue;
                }
                else if (sItem == "color") {
                    bUseRes = true;
                    sImageResType = RES_TYPE_COLOR;
                    sImageName = sValue;
                }
                else if( sItem == "dest" ) {
                    drawInfo.rcDestOffset.left = strtol(sValue.GetData(), &pstr, 10);  assert(pstr);
                    drawInfo.rcDestOffset.top = strtol(pstr + 1, &pstr, 10);    assert(pstr);
                    drawInfo.rcDestOffset.right = strtol(pstr + 1, &pstr, 10);  assert(pstr);
                    drawInfo.rcDestOffset.bottom = strtol(pstr + 1, &pstr, 10); assert(pstr);
                }
                else if( sItem == "source" ) {
                    drawInfo.rcBmpPart.left = strtol(sValue.GetData(), &pstr, 10);  assert(pstr);
                    drawInfo.rcBmpPart.top = strtol(pstr + 1, &pstr, 10);    assert(pstr);
                    drawInfo.rcBmpPart.right = strtol(pstr + 1, &pstr, 10);  assert(pstr);
                    drawInfo.rcBmpPart.bottom = strtol(pstr + 1, &pstr, 10); assert(pstr);
                }
                else if( sItem == "corner" || sItem == "scale9") {
                    drawInfo.rcScale9.left = strtol(sValue.GetData(), &pstr, 10);  assert(pstr);
                    drawInfo.rcScale9.top = strtol(pstr + 1, &pstr, 10);    assert(pstr);
                    drawInfo.rcScale9.right = strtol(pstr + 1, &pstr, 10);  assert(pstr);
                    drawInfo.rcScale9.bottom = strtol(pstr + 1, &pstr, 10); assert(pstr);
                }
                else if( sItem == "mask" ) {
                    if( sValue[0] == '#') dwMask = strtoul(sValue.GetData() + 1, &pstr, 16);
                    else dwMask = strtoul(sValue.GetData(), &pstr, 16);
                }
                else if( sItem == "fade" ) {
                    drawInfo.uFade = (uint8_t)strtoul(sValue.GetData(), &pstr, 10);
                }
                else if( sItem == "hole" ) {
                    drawInfo.bHole = (strcasecmp(sValue.GetData(), "true") == 0);
                }
                else if( sItem == "xtiled" ) {
                    drawInfo.bTiledX = (strcasecmp(sValue.GetData(), "true") == 0);
                }
                else if( sItem == "ytiled" ) {
                    drawInfo.bTiledY = (strcasecmp(sValue.GetData(), "true") == 0);
                }
                else if( sItem == "hsl" ) {
                    bUseHSL = (strcasecmp(sValue.GetData(), "true") == 0);
                }
            }
            if( *pstrImage++ != ' ' ) break;
        }
        drawInfo.sImageName = sImageName;

        const TImageInfo* data = nullptr;
        data = UIResourceMgr::GetInstance().GetImage(sImageName, true);
        /*if(!bUseRes) {

        }
        else {
            data = manager->GetImageEx(sImageName, sImageResType, dwMask, bUseHSL);
        }*/
        if( !data ) return false;

        drawInfo.pImageInfo = data;
        if( drawInfo.rcBmpPart.left == 0 && drawInfo.rcBmpPart.right == 0 &&
            drawInfo.rcBmpPart.top == 0 && drawInfo.rcBmpPart.bottom == 0 ) {
            drawInfo.rcBmpPart.right = data->nX;
            drawInfo.rcBmpPart.bottom = data->nY;
        }
    }
    if( drawInfo.rcBmpPart.right > drawInfo.pImageInfo->nX ) drawInfo.rcBmpPart.right = drawInfo.pImageInfo->nX;
    if( drawInfo.rcBmpPart.bottom > drawInfo.pImageInfo->nY ) drawInfo.rcBmpPart.bottom = drawInfo.pImageInfo->nY;

    if( hdc == nullptr ) return true;

    RECT rcDest = rcItem;
    if( drawInfo.rcDestOffset.left != 0 || drawInfo.rcDestOffset.top != 0 ||
        drawInfo.rcDestOffset.right != 0 || drawInfo.rcDestOffset.bottom != 0 ) {
        rcDest.left = rcItem.left + drawInfo.rcDestOffset.left;
        rcDest.top = rcItem.top + drawInfo.rcDestOffset.top;
        rcDest.right = rcItem.left + drawInfo.rcDestOffset.right;
        if( rcDest.right > rcItem.right ) rcDest.right = rcItem.right;
        rcDest.bottom = rcItem.top + drawInfo.rcDestOffset.bottom;
        if( rcDest.bottom > rcItem.bottom ) rcDest.bottom = rcItem.bottom;
    }

    RECT rcTemp;
    if( !::UIIntersectRect(&rcTemp, &rcDest, &rcItem) ) return true;
    if( !::UIIntersectRect(&rcTemp, &rcDest, &rcPaint) ) return true;
    DrawImage(hdc, drawInfo.pImageInfo->hBitmap, rcDest, rcPaint, drawInfo.rcBmpPart, drawInfo.rcScale9,
              drawInfo.pImageInfo->bAlpha, drawInfo.uFade, drawInfo.bHole, drawInfo.bTiledX, drawInfo.bTiledY);
    return true;
}
