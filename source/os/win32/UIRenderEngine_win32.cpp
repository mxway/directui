#include <UIRenderEngine.h>
#include <cassert>
#include <cstdlib>
#include <windows.h>
#include <UIPaintManager.h>
#include <algorithm>
#include "EncodingTransform.h"
#include "UIResourceMgr.h"
#include <UIRect.h>

static int g_iFontID = MAX_FONT_ID;

static COLORREF PixelAlpha(COLORREF clrSrc, double src_darken, COLORREF clrDest, double dest_darken)
{
    return RGB (GetRValue (clrSrc) * src_darken + GetRValue (clrDest) * dest_darken,
                GetGValue (clrSrc) * src_darken + GetGValue (clrDest) * dest_darken,
                GetBValue (clrSrc) * src_darken + GetBValue (clrDest) * dest_darken);
}

static BOOL WINAPI AlphaBitBlt(HDC hDC, int nDestX, int nDestY, int dwWidth, int dwHeight, HDC hSrcDC, \
                        int nSrcX, int nSrcY, int wSrc, int hSrc, BLENDFUNCTION ftn)
{
    HDC hTempDC = ::CreateCompatibleDC(hDC);
    if (nullptr == hTempDC)
        return FALSE;

    //Creates Source DIB
    auto lpbiSrc = (LPBITMAPINFO) new BYTE[sizeof(BITMAPINFOHEADER)];
    lpbiSrc->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    lpbiSrc->bmiHeader.biWidth = dwWidth;
    lpbiSrc->bmiHeader.biHeight = dwHeight;
    lpbiSrc->bmiHeader.biPlanes = 1;
    lpbiSrc->bmiHeader.biBitCount = 32;
    lpbiSrc->bmiHeader.biCompression = BI_RGB;
    lpbiSrc->bmiHeader.biSizeImage = dwWidth * dwHeight;
    lpbiSrc->bmiHeader.biXPelsPerMeter = 0;
    lpbiSrc->bmiHeader.biYPelsPerMeter = 0;
    lpbiSrc->bmiHeader.biClrUsed = 0;
    lpbiSrc->bmiHeader.biClrImportant = 0;

    COLORREF* pSrcBits = nullptr;
    HBITMAP hSrcDib = CreateDIBSection (
            hSrcDC, lpbiSrc, DIB_RGB_COLORS, (void **)&pSrcBits,
            nullptr, 0);

    if ((nullptr == hSrcDib) || (nullptr == pSrcBits))
    {
        delete [] lpbiSrc;
        ::DeleteDC(hTempDC);
        return FALSE;
    }

    auto hOldTempBmp = (HBITMAP)::SelectObject (hTempDC, hSrcDib);
    ::StretchBlt(hTempDC, 0, 0, dwWidth, dwHeight, hSrcDC, nSrcX, nSrcY, wSrc, hSrc, SRCCOPY);
    ::SelectObject (hTempDC, hOldTempBmp);

    //Creates Destination DIB
    auto lpbiDest = (LPBITMAPINFO) new BYTE[sizeof(BITMAPINFOHEADER)];
    // Fill in the BITMAPINFOHEADER

    lpbiDest->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    lpbiDest->bmiHeader.biWidth = dwWidth;
    lpbiDest->bmiHeader.biHeight = dwHeight;
    lpbiDest->bmiHeader.biPlanes = 1;
    lpbiDest->bmiHeader.biBitCount = 32;
    lpbiDest->bmiHeader.biCompression = BI_RGB;
    lpbiDest->bmiHeader.biSizeImage = dwWidth * dwHeight;
    lpbiDest->bmiHeader.biXPelsPerMeter = 0;
    lpbiDest->bmiHeader.biYPelsPerMeter = 0;
    lpbiDest->bmiHeader.biClrUsed = 0;
    lpbiDest->bmiHeader.biClrImportant = 0;

    COLORREF* pDestBits = nullptr;
    HBITMAP hDestDib = CreateDIBSection (
            hDC, lpbiDest, DIB_RGB_COLORS, (void **)&pDestBits,
            nullptr, 0);

    if ((nullptr == hDestDib) || (nullptr == pDestBits))
    {
        delete [] lpbiSrc;
        ::DeleteObject(hSrcDib);
        ::DeleteDC(hTempDC);
        return FALSE;
    }

    ::SelectObject (hTempDC, hDestDib);
    ::BitBlt (hTempDC, 0, 0, dwWidth, dwHeight, hDC, nDestX, nDestY, SRCCOPY);
    ::SelectObject (hTempDC, hOldTempBmp);

    double src_darken;
    BYTE nAlpha;

    for (int pixel = 0; pixel < dwWidth * dwHeight; pixel++, pSrcBits++, pDestBits++)
    {
        nAlpha = LOBYTE(*pSrcBits >> 24);
        src_darken = (double) (nAlpha * ftn.SourceConstantAlpha) / 255.0 / 255.0;
        if( src_darken < 0.0 ) src_darken = 0.0;
        *pDestBits = PixelAlpha(*pSrcBits, src_darken, *pDestBits, 1.0 - src_darken);
    } //for

    ::SelectObject (hTempDC, hDestDib);
    ::BitBlt (hDC, nDestX, nDestY, dwWidth, dwHeight, hTempDC, 0, 0, SRCCOPY);
    ::SelectObject (hTempDC, hOldTempBmp);

    delete [] lpbiDest;
    ::DeleteObject(hDestDib);

    delete [] lpbiSrc;
    ::DeleteObject(hSrcDib);

    ::DeleteDC(hTempDC);
    return TRUE;
}

void
UIRenderEngine::DrawImage(HANDLE_DC hDC, HANDLE_BITMAP hBitmap, const RECT &rc, const RECT &rcPaint, const RECT &rcBmpPart,
                          const RECT &rcScale9, bool alpha, uint8_t uFade, bool hole, bool xtiled, bool ytiled) {

    assert(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);

    typedef BOOL (WINAPI *LPALPHABLEND)(HDC, int, int, int, int,HDC, int, int, int, int, BLENDFUNCTION);
    static auto lpAlphaBlend = (LPALPHABLEND) ::GetProcAddress(::GetModuleHandle("msimg32.dll"), "AlphaBlend");

    if( lpAlphaBlend == nullptr ) lpAlphaBlend = AlphaBitBlt;
    if( hBitmap == nullptr ) return;

    HDC hCloneDC = ::CreateCompatibleDC(hDC);
    auto hOldBitmap = (HBITMAP) ::SelectObject(hCloneDC, hBitmap);
    ::SetStretchBltMode(hDC, COLORONCOLOR);

    RECT rcTemp = {0};
    RECT rcDest = {0};
    if( lpAlphaBlend && (alpha || uFade < 255) ) {
        BLENDFUNCTION bf = { AC_SRC_OVER, 0, uFade, AC_SRC_ALPHA };
        // middle
        if( !hole ) {
            rcDest.left = rc.left + rcScale9.left;
            rcDest.top = rc.top + rcScale9.top;
            rcDest.right = rc.right - rc.left - rcScale9.left - rcScale9.right;
            rcDest.bottom = rc.bottom - rc.top - rcScale9.top - rcScale9.bottom;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                if( !xtiled && !ytiled ) {
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
						rcBmpPart.left + rcScale9.left, rcBmpPart.top + rcScale9.top, \
						rcBmpPart.right - rcBmpPart.left - rcScale9.left - rcScale9.right, \
						rcBmpPart.bottom - rcBmpPart.top - rcScale9.top - rcScale9.bottom, bf);
                }
                else if( xtiled && ytiled ) {
                    LONG lWidth = rcBmpPart.right - rcBmpPart.left - rcScale9.left - rcScale9.right;
                    LONG lHeight = rcBmpPart.bottom - rcBmpPart.top - rcScale9.top - rcScale9.bottom;
                    int iTimesX = (rcDest.right - rcDest.left + lWidth - 1) / lWidth;
                    int iTimesY = (rcDest.bottom - rcDest.top + lHeight - 1) / lHeight;
                    for( int j = 0; j < iTimesY; ++j ) {
                        LONG lDestTop = rcDest.top + lHeight * j;
                        LONG lDestBottom = rcDest.top + lHeight * (j + 1);
                        LONG lDrawHeight = lHeight;
                        if( lDestBottom > rcDest.bottom ) {
                            lDrawHeight -= lDestBottom - rcDest.bottom;
                            lDestBottom = rcDest.bottom;
                        }
                        for( int i = 0; i < iTimesX; ++i ) {
                            LONG lDestLeft = rcDest.left + lWidth * i;
                            LONG lDestRight = rcDest.left + lWidth * (i + 1);
                            LONG lDrawWidth = lWidth;
                            if( lDestRight > rcDest.right ) {
                                lDrawWidth -= lDestRight - rcDest.right;
                                lDestRight = rcDest.right;
                            }
                            lpAlphaBlend(hDC, rcDest.left + lWidth * i, rcDest.top + lHeight * j,
                                         lDestRight - lDestLeft, lDestBottom - lDestTop, hCloneDC,
                                         rcBmpPart.left + rcScale9.left, rcBmpPart.top + rcScale9.top, lDrawWidth, lDrawHeight, bf);
                        }
                    }
                }
                else if( xtiled ) {
                    LONG lWidth = rcBmpPart.right - rcBmpPart.left - rcScale9.left - rcScale9.right;
                    int iTimes = (rcDest.right - rcDest.left + lWidth - 1) / lWidth;
                    for( int i = 0; i < iTimes; ++i ) {
                        LONG lDestLeft = rcDest.left + lWidth * i;
                        LONG lDestRight = rcDest.left + lWidth * (i + 1);
                        LONG lDrawWidth = lWidth;
                        if( lDestRight > rcDest.right ) {
                            lDrawWidth -= lDestRight - rcDest.right;
                            lDestRight = rcDest.right;
                        }
                        rcDest.bottom -= rcDest.top;
                        lpAlphaBlend(hDC, lDestLeft, rcDest.top, lDestRight - lDestLeft, rcDest.bottom,
                                     hCloneDC, rcBmpPart.left + rcScale9.left, rcBmpPart.top + rcScale9.top, \
							lDrawWidth, rcBmpPart.bottom - rcBmpPart.top - rcScale9.top - rcScale9.bottom, bf);
                    }
                }
                else { // bTiledY
                    LONG lHeight = rcBmpPart.bottom - rcBmpPart.top - rcScale9.top - rcScale9.bottom;
                    int iTimes = (rcDest.bottom - rcDest.top + lHeight - 1) / lHeight;
                    for( int i = 0; i < iTimes; ++i ) {
                        LONG lDestTop = rcDest.top + lHeight * i;
                        LONG lDestBottom = rcDest.top + lHeight * (i + 1);
                        LONG lDrawHeight = lHeight;
                        if( lDestBottom > rcDest.bottom ) {
                            lDrawHeight -= lDestBottom - rcDest.bottom;
                            lDestBottom = rcDest.bottom;
                        }
                        rcDest.right -= rcDest.left;
                        lpAlphaBlend(hDC, rcDest.left, rcDest.top + lHeight * i, rcDest.right, lDestBottom - lDestTop,
                                     hCloneDC, rcBmpPart.left + rcScale9.left, rcBmpPart.top + rcScale9.top, \
							rcBmpPart.right - rcBmpPart.left - rcScale9.left - rcScale9.right, lDrawHeight, bf);
                    }
                }
            }
        }

        // left-top
        if( rcScale9.left > 0 && rcScale9.top > 0 ) {
            rcDest.left = rc.left;
            rcDest.top = rc.top;
            rcDest.right = rcScale9.left;
            rcDest.bottom = rcScale9.top;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
					rcBmpPart.left, rcBmpPart.top, rcScale9.left, rcScale9.top, bf);
            }
        }
        // top
        if( rcScale9.top > 0 ) {
            rcDest.left = rc.left + rcScale9.left;
            rcDest.top = rc.top;
            rcDest.right = rc.right - rc.left - rcScale9.left - rcScale9.right;
            rcDest.bottom = rcScale9.top;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
					rcBmpPart.left + rcScale9.left, rcBmpPart.top, rcBmpPart.right - rcBmpPart.left - \
					rcScale9.left - rcScale9.right, rcScale9.top, bf);
            }
        }
        // right-top
        if( rcScale9.right > 0 && rcScale9.top > 0 ) {
            rcDest.left = rc.right - rcScale9.right;
            rcDest.top = rc.top;
            rcDest.right = rcScale9.right;
            rcDest.bottom = rcScale9.top;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
					rcBmpPart.right - rcScale9.right, rcBmpPart.top, rcScale9.right, rcScale9.top, bf);
            }
        }
        // left
        if( rcScale9.left > 0 ) {
            rcDest.left = rc.left;
            rcDest.top = rc.top + rcScale9.top;
            rcDest.right = rcScale9.left;
            rcDest.bottom = rc.bottom - rc.top - rcScale9.top - rcScale9.bottom;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
					rcBmpPart.left, rcBmpPart.top + rcScale9.top, rcScale9.left, rcBmpPart.bottom - \
					rcBmpPart.top - rcScale9.top - rcScale9.bottom, bf);
            }
        }
        // right
        if( rcScale9.right > 0 ) {
            rcDest.left = rc.right - rcScale9.right;
            rcDest.top = rc.top + rcScale9.top;
            rcDest.right = rcScale9.right;
            rcDest.bottom = rc.bottom - rc.top - rcScale9.top - rcScale9.bottom;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
					rcBmpPart.right - rcScale9.right, rcBmpPart.top + rcScale9.top, rcScale9.right, \
					rcBmpPart.bottom - rcBmpPart.top - rcScale9.top - rcScale9.bottom, bf);
            }
        }
        // left-bottom
        if( rcScale9.left > 0 && rcScale9.bottom > 0 ) {
            rcDest.left = rc.left;
            rcDest.top = rc.bottom - rcScale9.bottom;
            rcDest.right = rcScale9.left;
            rcDest.bottom = rcScale9.bottom;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
					rcBmpPart.left, rcBmpPart.bottom - rcScale9.bottom, rcScale9.left, rcScale9.bottom, bf);
            }
        }
        // bottom
        if( rcScale9.bottom > 0 ) {
            rcDest.left = rc.left + rcScale9.left;
            rcDest.top = rc.bottom - rcScale9.bottom;
            rcDest.right = rc.right - rc.left - rcScale9.left - rcScale9.right;
            rcDest.bottom = rcScale9.bottom;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
					rcBmpPart.left + rcScale9.left, rcBmpPart.bottom - rcScale9.bottom, \
					rcBmpPart.right - rcBmpPart.left - rcScale9.left - rcScale9.right, rcScale9.bottom, bf);
            }
        }
        // right-bottom
        if( rcScale9.right > 0 && rcScale9.bottom > 0 ) {
            rcDest.left = rc.right - rcScale9.right;
            rcDest.top = rc.bottom - rcScale9.bottom;
            rcDest.right = rcScale9.right;
            rcDest.bottom = rcScale9.bottom;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
					rcBmpPart.right - rcScale9.right, rcBmpPart.bottom - rcScale9.bottom, rcScale9.right, \
					rcScale9.bottom, bf);
            }
        }
    }
    else
    {
        if (rc.right - rc.left == rcBmpPart.right - rcBmpPart.left \
			&& rc.bottom - rc.top == rcBmpPart.bottom - rcBmpPart.top \
			&& rcScale9.left == 0 && rcScale9.right == 0 && rcScale9.top == 0 && rcScale9.bottom == 0)
        {
            if( ::IntersectRect(&rcTemp, &rcPaint, &rc) ) {
                ::BitBlt(hDC, rcTemp.left, rcTemp.top, rcTemp.right - rcTemp.left, rcTemp.bottom - rcTemp.top, \
					hCloneDC, rcBmpPart.left + rcTemp.left - rc.left, rcBmpPart.top + rcTemp.top - rc.top, SRCCOPY);
            }
        }
        else
        {
            // middle
            if( !hole ) {
                rcDest.left = rc.left + rcScale9.left;
                rcDest.top = rc.top + rcScale9.top;
                rcDest.right = rc.right - rc.left - rcScale9.left - rcScale9.right;
                rcDest.bottom = rc.bottom - rc.top - rcScale9.top - rcScale9.bottom;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                    if( !xtiled && !ytiled ) {
                        rcDest.right -= rcDest.left;
                        rcDest.bottom -= rcDest.top;
                        ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
							rcBmpPart.left + rcScale9.left, rcBmpPart.top + rcScale9.top, \
							rcBmpPart.right - rcBmpPart.left - rcScale9.left - rcScale9.right, \
							rcBmpPart.bottom - rcBmpPart.top - rcScale9.top - rcScale9.bottom, SRCCOPY);
                    }
                    else if( xtiled && ytiled ) {
                        LONG lWidth = rcBmpPart.right - rcBmpPart.left - rcScale9.left - rcScale9.right;
                        LONG lHeight = rcBmpPart.bottom - rcBmpPart.top - rcScale9.top - rcScale9.bottom;
                        int iTimesX = (rcDest.right - rcDest.left + lWidth - 1) / lWidth;
                        int iTimesY = (rcDest.bottom - rcDest.top + lHeight - 1) / lHeight;
                        for( int j = 0; j < iTimesY; ++j ) {
                            LONG lDestTop = rcDest.top + lHeight * j;
                            LONG lDestBottom = rcDest.top + lHeight * (j + 1);
                            LONG lDrawHeight = lHeight;
                            if( lDestBottom > rcDest.bottom ) {
                                lDrawHeight -= lDestBottom - rcDest.bottom;
                                lDestBottom = rcDest.bottom;
                            }
                            for( int i = 0; i < iTimesX; ++i ) {
                                LONG lDestLeft = rcDest.left + lWidth * i;
                                LONG lDestRight = rcDest.left + lWidth * (i + 1);
                                LONG lDrawWidth = lWidth;
                                if( lDestRight > rcDest.right ) {
                                    lDrawWidth -= lDestRight - rcDest.right;
                                    lDestRight = rcDest.right;
                                }
                                ::BitBlt(hDC, rcDest.left + lWidth * i, rcDest.top + lHeight * j, \
									lDestRight - lDestLeft, lDestBottom - lDestTop, hCloneDC, \
									rcBmpPart.left + rcScale9.left, rcBmpPart.top + rcScale9.top, SRCCOPY);
                            }
                        }
                    }
                    else if( ytiled ) {
                        LONG lWidth = rcBmpPart.right - rcBmpPart.left - rcScale9.left - rcScale9.right;
                        int iTimes = (rcDest.right - rcDest.left + lWidth - 1) / lWidth;
                        for( int i = 0; i < iTimes; ++i ) {
                            LONG lDestLeft = rcDest.left + lWidth * i;
                            LONG lDestRight = rcDest.left + lWidth * (i + 1);
                            LONG lDrawWidth = lWidth;
                            if( lDestRight > rcDest.right ) {
                                lDrawWidth -= lDestRight - rcDest.right;
                                lDestRight = rcDest.right;
                            }
                            rcDest.bottom -= rcDest.top;
                            ::StretchBlt(hDC, lDestLeft, rcDest.top, lDestRight - lDestLeft, rcDest.bottom,
                                         hCloneDC, rcBmpPart.left + rcScale9.left, rcBmpPart.top + rcScale9.top, \
								lDrawWidth, rcBmpPart.bottom - rcBmpPart.top - rcScale9.top - rcScale9.bottom, SRCCOPY);
                        }
                    }
                    else { // bTiledY
                        LONG lHeight = rcBmpPart.bottom - rcBmpPart.top - rcScale9.top - rcScale9.bottom;
                        int iTimes = (rcDest.bottom - rcDest.top + lHeight - 1) / lHeight;
                        for( int i = 0; i < iTimes; ++i ) {
                            LONG lDestTop = rcDest.top + lHeight * i;
                            LONG lDestBottom = rcDest.top + lHeight * (i + 1);
                            LONG lDrawHeight = lHeight;
                            if( lDestBottom > rcDest.bottom ) {
                                lDrawHeight -= lDestBottom - rcDest.bottom;
                                lDestBottom = rcDest.bottom;
                            }
                            rcDest.right -= rcDest.left;
                            ::StretchBlt(hDC, rcDest.left, rcDest.top + lHeight * i, rcDest.right, lDestBottom - lDestTop,
                                         hCloneDC, rcBmpPart.left + rcScale9.left, rcBmpPart.top + rcScale9.top, \
								rcBmpPart.right - rcBmpPart.left - rcScale9.left - rcScale9.right, lDrawHeight, SRCCOPY);
                        }
                    }
                }
            }

            // left-top
            if( rcScale9.left > 0 && rcScale9.top > 0 ) {
                rcDest.left = rc.left;
                rcDest.top = rc.top;
                rcDest.right = rcScale9.left;
                rcDest.bottom = rcScale9.top;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
						rcBmpPart.left, rcBmpPart.top, rcScale9.left, rcScale9.top, SRCCOPY);
                }
            }
            // top
            if( rcScale9.top > 0 ) {
                rcDest.left = rc.left + rcScale9.left;
                rcDest.top = rc.top;
                rcDest.right = rc.right - rc.left - rcScale9.left - rcScale9.right;
                rcDest.bottom = rcScale9.top;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
						rcBmpPart.left + rcScale9.left, rcBmpPart.top, rcBmpPart.right - rcBmpPart.left - \
						rcScale9.left - rcScale9.right, rcScale9.top, SRCCOPY);
                }
            }
            // right-top
            if( rcScale9.right > 0 && rcScale9.top > 0 ) {
                rcDest.left = rc.right - rcScale9.right;
                rcDest.top = rc.top;
                rcDest.right = rcScale9.right;
                rcDest.bottom = rcScale9.top;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
						rcBmpPart.right - rcScale9.right, rcBmpPart.top, rcScale9.right, rcScale9.top, SRCCOPY);
                }
            }
            // left
            if( rcScale9.left > 0 ) {
                rcDest.left = rc.left;
                rcDest.top = rc.top + rcScale9.top;
                rcDest.right = rcScale9.left;
                rcDest.bottom = rc.bottom - rc.top - rcScale9.top - rcScale9.bottom;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
						rcBmpPart.left, rcBmpPart.top + rcScale9.top, rcScale9.left, rcBmpPart.bottom - \
						rcBmpPart.top - rcScale9.top - rcScale9.bottom, SRCCOPY);
                }
            }
            // right
            if( rcScale9.right > 0 ) {
                rcDest.left = rc.right - rcScale9.right;
                rcDest.top = rc.top + rcScale9.top;
                rcDest.right = rcScale9.right;
                rcDest.bottom = rc.bottom - rc.top - rcScale9.top - rcScale9.bottom;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
						rcBmpPart.right - rcScale9.right, rcBmpPart.top + rcScale9.top, rcScale9.right, \
						rcBmpPart.bottom - rcBmpPart.top - rcScale9.top - rcScale9.bottom, SRCCOPY);
                }
            }
            // left-bottom
            if( rcScale9.left > 0 && rcScale9.bottom > 0 ) {
                rcDest.left = rc.left;
                rcDest.top = rc.bottom - rcScale9.bottom;
                rcDest.right = rcScale9.left;
                rcDest.bottom = rcScale9.bottom;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
						rcBmpPart.left, rcBmpPart.bottom - rcScale9.bottom, rcScale9.left, rcScale9.bottom, SRCCOPY);
                }
            }
            // bottom
            if( rcScale9.bottom > 0 ) {
                rcDest.left = rc.left + rcScale9.left;
                rcDest.top = rc.bottom - rcScale9.bottom;
                rcDest.right = rc.right - rc.left - rcScale9.left - rcScale9.right;
                rcDest.bottom = rcScale9.bottom;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
						rcBmpPart.left + rcScale9.left, rcBmpPart.bottom - rcScale9.bottom, \
						rcBmpPart.right - rcBmpPart.left - rcScale9.left - rcScale9.right, rcScale9.bottom, SRCCOPY);
                }
            }
            // right-bottom
            if( rcScale9.right > 0 && rcScale9.bottom > 0 ) {
                rcDest.left = rc.right - rcScale9.right;
                rcDest.top = rc.bottom - rcScale9.bottom;
                rcDest.right = rcScale9.right;
                rcDest.bottom = rcScale9.bottom;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
						rcBmpPart.right - rcScale9.right, rcBmpPart.bottom - rcScale9.bottom, rcScale9.right, \
						rcScale9.bottom, SRCCOPY);
                }
            }
        }
    }
    ::SelectObject(hCloneDC, hOldBitmap);
    ::DeleteDC(hCloneDC);

}

void UIRenderEngine::DrawColor(HANDLE_DC hDC, const RECT &rc, uint32_t color) {
    //if( color <= 0x00FFFFFF ) return;
    if( color >= 0xFF000000 )
    {
        ::SetBkColor(hDC, RGB(GetBValue(color), GetGValue(color), GetRValue(color)));
        ::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
    }
    else
    {
        // Create a new 32bpp bitmap with room for an alpha channel
        BITMAPINFO bmi = { 0 };
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = 1;
        bmi.bmiHeader.biHeight = 1;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        bmi.bmiHeader.biSizeImage = 1 * 1 * sizeof(DWORD);
        LPDWORD pDest = NULL;
        HBITMAP hBitmap = ::CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, (LPVOID*) &pDest, NULL, 0);
        if( !hBitmap ) return;

        *pDest = color;

        RECT rcBmpPart = {0, 0, 1, 1};
        RECT rcCorners = {0};
        DrawImage(hDC, hBitmap, rc, rc, rcBmpPart, rcCorners, true, 255);
        ::DeleteObject(hBitmap);
    }
}

void UIRenderEngine::DrawGradient(HANDLE_DC hDC, const RECT &rc, uint32_t dwFirst, uint32_t dwSecond, bool bVertical, int nSteps) {
    typedef BOOL (WINAPI *LPALPHABLEND)(HDC, int, int, int, int,HDC, int, int, int, int, BLENDFUNCTION);
    HMODULE  hmodule = ::LoadLibraryA("msimg32.dll");
    static LPALPHABLEND lpAlphaBlend = (LPALPHABLEND) ::GetProcAddress(hmodule, "AlphaBlend");
    if( lpAlphaBlend == NULL ) lpAlphaBlend = AlphaBitBlt;
    typedef BOOL (WINAPI *PGradientFill)(HDC, PTRIVERTEX, ULONG, PVOID, ULONG, ULONG);
    static PGradientFill lpGradientFill = (PGradientFill) ::GetProcAddress(hmodule, "GradientFill");

    BYTE bAlpha = (BYTE)(((dwFirst >> 24) + (dwSecond >> 24)) >> 1);
    if( bAlpha == 0 ) return;
    int cx = rc.right - rc.left;
    int cy = rc.bottom - rc.top;
    RECT rcPaint = rc;
    HDC hPaintDC = hDC;
    HBITMAP hPaintBitmap = NULL;
    HBITMAP hOldPaintBitmap = NULL;
    if( bAlpha < 255 ) {
        rcPaint.left = rcPaint.top = 0;
        rcPaint.right = cx;
        rcPaint.bottom = cy;
        hPaintDC = ::CreateCompatibleDC(hDC);
        hPaintBitmap = ::CreateCompatibleBitmap(hDC, cx, cy);
        assert(hPaintDC);
        assert(hPaintBitmap);
        hOldPaintBitmap = (HBITMAP) ::SelectObject(hPaintDC, hPaintBitmap);
    }
    if( lpGradientFill != NULL )
    {
        TRIVERTEX triv[2] =
                {
                        { rcPaint.left, rcPaint.top, (USHORT)(GetBValue(dwFirst) << 8), (USHORT)(GetGValue(dwFirst) << 8), (USHORT)(GetRValue(dwFirst) << 8), 0xFF00 },
                        { rcPaint.right, rcPaint.bottom, (USHORT)(GetBValue(dwSecond) << 8), (USHORT)(GetGValue(dwSecond) << 8), (USHORT)(GetRValue(dwSecond) << 8), 0xFF00 }
                };
        GRADIENT_RECT grc = { 0, 1 };
        lpGradientFill(hPaintDC, triv, 2, &grc, 1, bVertical ? GRADIENT_FILL_RECT_V : GRADIENT_FILL_RECT_H);
    }
    else
    {
        // Determine how many shades
        int nShift = 1;
        if( nSteps >= 64 ) nShift = 6;
        else if( nSteps >= 32 ) nShift = 5;
        else if( nSteps >= 16 ) nShift = 4;
        else if( nSteps >= 8 ) nShift = 3;
        else if( nSteps >= 4 ) nShift = 2;
        int nLines = 1 << nShift;
        for( int i = 0; i < nLines; i++ ) {
            // Do a little alpha blending
            BYTE bR = (BYTE) ((GetBValue(dwSecond) * (nLines - i) + GetBValue(dwFirst) * i) >> nShift);
            BYTE bG = (BYTE) ((GetGValue(dwSecond) * (nLines - i) + GetGValue(dwFirst) * i) >> nShift);
            BYTE bB = (BYTE) ((GetRValue(dwSecond) * (nLines - i) + GetRValue(dwFirst) * i) >> nShift);
            // ... then paint with the resulting color
            HBRUSH hBrush = ::CreateSolidBrush(RGB(bR,bG,bB));
            RECT r2 = rcPaint;
            if( bVertical ) {
                r2.bottom = rc.bottom - ((i * (rc.bottom - rc.top)) >> nShift);
                r2.top = rc.bottom - (((i + 1) * (rc.bottom - rc.top)) >> nShift);
                if( (r2.bottom - r2.top) > 0 ) ::FillRect(hDC, &r2, hBrush);
            }
            else {
                r2.left = rc.right - (((i + 1) * (rc.right - rc.left)) >> nShift);
                r2.right = rc.right - ((i * (rc.right - rc.left)) >> nShift);
                if( (r2.right - r2.left) > 0 ) ::FillRect(hPaintDC, &r2, hBrush);
            }
            ::DeleteObject(hBrush);
        }
    }
    if( bAlpha < 255 ) {
        BLENDFUNCTION bf = { AC_SRC_OVER, 0, bAlpha, AC_SRC_ALPHA };
        lpAlphaBlend(hDC, rc.left, rc.top, cx, cy, hPaintDC, 0, 0, cx, cy, bf);
        ::SelectObject(hPaintDC, hOldPaintBitmap);
        ::DeleteObject(hPaintBitmap);
        ::DeleteDC(hPaintDC);
    }
    ::FreeLibrary(hmodule);
}

void UIRenderEngine::DrawLine(HANDLE_DC hDC, const RECT &rc, int nSize, uint32_t dwPenColor, int nStyle) {
    assert(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);

    LOGPEN lg;
    lg.lopnColor = RGB(GetBValue(dwPenColor), GetGValue(dwPenColor), GetRValue(dwPenColor));
    lg.lopnStyle = nStyle;
    lg.lopnWidth.x = nSize;
    HPEN hPen = CreatePenIndirect(&lg);
    HPEN hOldPen = (HPEN)::SelectObject(hDC, hPen);
    POINT ptTemp = { 0 };
    ::MoveToEx(hDC, rc.left, rc.top, &ptTemp);
    ::LineTo(hDC, rc.right, rc.bottom);
    ::SelectObject(hDC, hOldPen);
    ::DeleteObject(hPen);
}

void UIRenderEngine::DrawRect(HANDLE_DC hDC, const RECT &rc, int nSize, uint32_t dwPenColor, int nStyle) {
    assert(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
    HPEN hPen = ::CreatePen(nStyle | PS_INSIDEFRAME, nSize, RGB(GetBValue(dwPenColor), GetGValue(dwPenColor), GetRValue(dwPenColor)));
    HPEN hOldPen = (HPEN)::SelectObject(hDC, hPen);
    ::SelectObject(hDC, ::GetStockObject(HOLLOW_BRUSH));
    ::Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);
    ::SelectObject(hDC, hOldPen);
    ::DeleteObject(hPen);
}

void UIRenderEngine::DrawRoundRect(HANDLE_DC hDC, const RECT &rc, int width, int height, int nSize, uint32_t dwPenColor,
                                   int nStyle) {
    assert(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
    HPEN hPen = ::CreatePen(nStyle | PS_INSIDEFRAME, nSize, RGB(GetBValue(dwPenColor), GetGValue(dwPenColor), GetRValue(dwPenColor)));
    HPEN hOldPen = (HPEN)::SelectObject(hDC, hPen);
    ::SelectObject(hDC, ::GetStockObject(HOLLOW_BRUSH));
    ::RoundRect(hDC, rc.left, rc.top, rc.right, rc.bottom, width, height);
    ::SelectObject(hDC, hOldPen);
    ::DeleteObject(hPen);
}

void UIRenderEngine::DrawText(HANDLE_DC hDC, UIPaintManager* pManager, RECT& rc, const UIString &text, \
        uint32_t dwTextColor, int iFont, uint32_t uStyle)
{
    assert(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
    if( text.IsEmpty() || pManager == NULL ) return;

    //CDuiString sText = pstrText;
    //CPaintManagerUI::ProcessMultiLanguageTokens(sText);
    //pstrText = sText;
    wchar_t *wideText = Utf8ToUcs2(text.GetData(),-1);

    ::SetBkMode(hDC, TRANSPARENT);
    ::SetTextColor(hDC, RGB(GetBValue(dwTextColor), GetGValue(dwTextColor), GetRValue(dwTextColor)));
    HFONT hOldFont = (HFONT)::SelectObject(hDC, UIResourceMgr::GetInstance().GetFont(iFont)->GetHandle());
    ::DrawTextW(hDC, wideText, -1, &rc, uStyle | DT_NOPREFIX);
    ::SelectObject(hDC, hOldFont);
    delete []wideText;
}

static void GetFontTextMetrics(HANDLE_DC hDC, HANDLE_FONT fontHandle, TEXTMETRICW *tm)
{
    auto hOldFont = (HFONT)::SelectObject(hDC, fontHandle);
    ::GetTextMetricsW(hDC, tm);
    ::SelectObject(hDC, hOldFont);
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

    assert(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
    if( text.IsEmpty() || pManager == NULL ) return;
    if( ::IsRectEmpty(&rc) ) return;

    bool bDraw = (uStyle & DT_CALCRECT) == 0;

    UIPtrArray aFontArray(10);
    UIPtrArray aColorArray(10);
    UIPtrArray aPIndentArray(10);
    UIPtrArray aVAlignArray(10);

    RECT rcClip = { 0 };
    ::GetClipBox(hDC, &rcClip);
    HRGN hOldRgn = ::CreateRectRgnIndirect(&rcClip);
    HRGN hRgn = ::CreateRectRgnIndirect(&rc);
    if( bDraw ) ::ExtSelectClipRgn(hDC, hRgn, RGN_AND);

    //UIString sText = text;
    //CPaintManagerUI::ProcessMultiLanguageTokens(sText);
    //pstrText = sText;
    //const char *pstrText = text.GetData();
    const wchar_t *wideString = Utf8ToUcs2(text.GetData());
    const wchar_t *pstrText = wideString;

    TEXTMETRICW tm = {0};
    GetFontTextMetrics(hDC,
                       UIResourceMgr::GetInstance().GetFont(iDefaultFont)->GetHandle(),
                        &tm);
    HFONT hOldFont = (HFONT) ::SelectObject(hDC, UIResourceMgr::GetInstance().GetFont(iDefaultFont)->GetHandle());
    ::SetBkMode(hDC, TRANSPARENT);
    ::SetTextColor(hDC, RGB(GetBValue(dwTextColor), GetGValue(dwTextColor), GetRValue(dwTextColor)));
    DWORD dwBkColor = pManager->GetDefaultSelectedBkColor();
    ::SetBkColor(hDC, RGB(GetBValue(dwBkColor), GetGValue(dwBkColor), GetRValue(dwBkColor)));

    // If the drawstyle include a alignment, we'll need to first determine the text-size so
    // we can draw it at the correct position...
    if( ((uStyle & DT_CENTER) != 0 || (uStyle & DT_RIGHT) != 0 || (uStyle & DT_VCENTER) != 0 || (uStyle & DT_BOTTOM) != 0) && (uStyle & DT_CALCRECT) == 0 ) {
        RECT rcText = { 0, 0, 9999, 100 };
        if ((uStyle & DT_SINGLELINE) == 0) {
            rcText.right = rc.right - rc.left;
            rcText.bottom = rc.bottom - rc.top;
        }
        int nLinks = 0;
        const char *utf8String = Ucs2ToUtf8(pstrText);
        DrawHtmlText(hDC, pManager, rcText, UIString{utf8String}, dwTextColor, nullptr, nullptr, nLinks, iDefaultFont, uStyle | DT_CALCRECT & ~DT_CENTER & ~DT_RIGHT & ~DT_VCENTER & ~DT_BOTTOM);
        delete []utf8String;
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
        if( ::PtInRect(prcLinks + i, ptMouse) ) {
            sHoverLink = *(UIString*)(sLinks + i);
            bHoverLink = true;
        }
    }

    POINT pt = { rc.left, rc.top };
    int iLinkIndex = 0;
    int cxLine = 0;
    int cyLine = tm.tmHeight + tm.tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1);
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
    const wchar_t *pstrLineBegin = pstrText;
    bool bLineInRaw = false;
    bool bLineInLink = false;
    bool bLineInSelected = false;
    UINT iVAlign = DT_BOTTOM;
    int cxLineWidth = 0;
    int cyLineHeight = 0;
    int cxOffset = 0;
    bool bLineDraw = false; // 行的第二阶段：绘制
    while( *pstrText != L'\0' ) {
        if( pt.x >= rc.right || *pstrText == L'\n' || bLineEnd ) {
            if( *pstrText == L'\n' ) pstrText++;
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
            cyLine = tm.tmHeight + tm.tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1);
            if( pt.x >= rc.right )
                break;
        }
        else if( !bInRaw && ( *pstrText == L'<' || *pstrText == L'{' )
                 && ( pstrText[1] >= L'a' && pstrText[1] <= L'z' )
                 && ( pstrText[2] == L' ' || pstrText[2] == L'>' || pstrText[2] == L'}' ) ) {
            pstrText++;
            const wchar_t *pstrNextStart = nullptr;
            switch( *pstrText ) {
                case L'a':  // Link
                {
                    pstrText++;
                    while( *pstrText > L'\0' && *pstrText <= L' ' ) pstrText = CharNextW(pstrText);
                    if( iLinkIndex < nLinkRects && !bLineDraw ) {
                        auto *pStr = (UIString*)(sLinks + iLinkIndex);
                        pStr->Empty();
                        while( *pstrText != L'\0' && *pstrText != L'>' && *pstrText != L'}' ) {
                            const wchar_t *pstrTemp = CharNextW(pstrText);
                            while( pstrText < pstrTemp) {
                                const char *utf8TempText = Ucs2ToUtf8(pstrText,1);
                                *pStr += utf8TempText;
                                pstrText++;
                                delete []utf8TempText;
                                //*pStr += *pstrText++;
                            }
                        }
                    }

                    DWORD clrColor = pManager->GetDefaultLinkFontColor();
                    if( bHoverLink && iLinkIndex < nLinkRects ) {
                        auto *pStr = (UIString*)(sLinks + iLinkIndex);
                        if( sHoverLink == *pStr ) clrColor = pManager->GetDefaultLinkHoverFontColor();
                    }

                    aColorArray.Add((LPVOID)clrColor);
                    ::SetTextColor(hDC,  RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
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
                        GetFontTextMetrics(hDC,pFontInfo->GetHandle(),&tm);
                        ::SelectObject(hDC, pFontInfo->GetHandle());
                        cyLine = MAX((LONG)cyLine, tm.tmHeight + tm.tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                    }
                    ptLinkStart = pt;
                    bInLink = true;
                }
                    break;
                case L'b':  // Bold
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
                        GetFontTextMetrics(hDC,pFontInfo->GetHandle(),&tm);
                        //pTm = &pFontInfo->tm;
                        ::SelectObject(hDC, pFontInfo->GetHandle());
                        cyLine = MAX((LONG)cyLine, tm.tmHeight + tm.tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                    }
                }
                    break;
                case L'c':  // Color
                {
                    pstrText++;
                    while( *pstrText > L'\0' && *pstrText <= L' ' ) pstrText = CharNextW(pstrText);
                    if( *pstrText == L'#') pstrText++;
                    DWORD clrColor = wcstol(pstrText, const_cast<wchar_t**>(&pstrText), 16);
                    aColorArray.Add((LPVOID)clrColor);
                    ::SetTextColor(hDC, RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
                }
                    break;
                case L'f':  // Font
                {
                    pstrText++;
                    while( *pstrText > L'\0' && *pstrText <= L' ' ) pstrText = CharNextW(pstrText);
                    const wchar_t *pstrTemp = pstrText;
                    int iFont = (int) wcstol(pstrText, const_cast<wchar_t**>(&pstrText), 10);
                    //if( isdigit(*pstrText) ) { // debug版本会引起异常
                    if( pstrTemp != pstrText ) {
                        UIFont* pFontInfo = UIResourceMgr::GetInstance().GetFont(iFont);//pManager->GetFontInfo(iFont);
                        aFontArray.Add(pFontInfo);
                        ::GetFontTextMetrics(hDC, pFontInfo->GetHandle(), &tm);
                        ::SelectObject(hDC, pFontInfo->GetHandle());
                    }
                    else {
                        UIString sFontName;
                        int iFontSize = 10;
                        UIString sFontAttr;
                        bool bBold = false;
                        bool bUnderline = false;
                        bool bItalic = false;
                        while( *pstrText != L'\0' && *pstrText != L'>' && *pstrText != L'}' && *pstrText != L' ' ) {
                            pstrTemp = CharNextW(pstrText);
                            while( pstrText < pstrTemp) {
                                const char *utf8TempText = Ucs2ToUtf8(pstrText,1);
                                sFontName += utf8TempText;
                                delete []utf8TempText;
                                pstrText++;
                                //sFontName += *pstrText++;
                            }
                        }
                        while( *pstrText > L'\0' && *pstrText <= L' ' ) pstrText = CharNextW(pstrText);
                        if( *pstrText>=L'0' && *pstrText<=L'9' ) {
                            iFontSize = (int) wcstol(pstrText, const_cast<wchar_t **>(&pstrText), 10);
                        }
                        while( *pstrText > L'\0' && *pstrText <= L' ' ) pstrText = CharNextW(pstrText);
                        while( *pstrText != L'\0' && *pstrText != L'>' && *pstrText != L'}' ) {
                            pstrTemp = CharNextW(pstrText);
                            while( pstrText < pstrTemp) {
                                const char *utf8TempText = Ucs2ToUtf8(pstrText,1);
                                sFontAttr += utf8TempText;
                                delete []utf8TempText;
                                pstrText++;
                                //sFontAttr += *pstrText++;
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
                        GetFontTextMetrics(hDC, pFontInfo->GetHandle(), &tm);
                        ::SelectObject(hDC, pFontInfo->GetHandle());
                    }
                    cyLine = MAX((LONG)cyLine, tm.tmHeight + tm.tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                }
                    break;
                case L'i':  // Italic or Image
                {
                    pstrNextStart = pstrText - 1;
                    pstrText++;
                    const char *utf8TempText = Ucs2ToUtf8(pstrText);
                    UIString sImageString {utf8TempText};
                    delete []utf8TempText;
                    int iWidth = 0;
                    int iHeight = 0;
                    while( *pstrText > L'\0' && *pstrText <= L' ' ) pstrText = CharNextW(pstrText);
                    const TImageInfo* pImageInfo = NULL;
                    UIString sName;
                    while( *pstrText != L'\0' && *pstrText != L'>' && *pstrText != L'}' && *pstrText != L' ' ) {
                        const wchar_t *pstrTemp = CharNextW(pstrText);
                        while( pstrText < pstrTemp) {
                            const char *utf8TempName = Ucs2ToUtf8(pstrText,1);
                            sName += utf8TempName;
                            delete []utf8TempName;
                            pstrText++;
                            //sName += *pstrText++;
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
                            GetFontTextMetrics(hDC,pFontInfo->GetHandle(),&tm);
                            ::SelectObject(hDC, pFontInfo->GetHandle());
                            cyLine = MAX((LONG)cyLine, tm.tmHeight + tm.tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                        }
                    }
                    else {
                        while( *pstrText > L'\0' && *pstrText <= L' ' ) pstrText = CharNextW(pstrText);
                        int iImageListNum = (int) wcstol(pstrText, const_cast<wchar_t **>(&pstrText), 10);
                        if( iImageListNum <= 0 ) iImageListNum = 1;
                        while( *pstrText > L'\0' && *pstrText <= L' ' ) pstrText = CharNextW(pstrText);
                        int iImageListIndex = (int) wcstol(pstrText, const_cast<wchar_t **>(&pstrText), 10);
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
                                    LPTSTR pstrTemp = CharNext(pStrImage);
                                    while( pStrImage < pstrTemp) {
                                        sItem += *pStrImage++;
                                    }
                                }
                                while( *pStrImage > '\0' && *pStrImage <= ' ' ) pStrImage = CharNext(pStrImage);
                                if( *pStrImage++ != '=' ) break;
                                while( *pStrImage > '\0' && *pStrImage <= ' ' ) pStrImage = CharNext(pStrImage);
                                if( *pStrImage++ != '\'' ) break;
                                while( *pStrImage != '\0' && *pStrImage != '\'' ) {
                                    LPTSTR pstrTemp = CharNext(pStrImage);
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
                                    if (aVAlignArray.GetSize() > 0) iVAlign = (UINT)aVAlignArray.GetAt(aVAlignArray.GetSize() - 1);
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
                                cxMaxWidth = MAX((LONG)cxMaxWidth, pt.x);
                                cxLine = pt.x - rc.left;
                                cyMinHeight = pt.y + iHeight;
                            }
                        }
                        else pstrNextStart = nullptr;
                    }
                }
                    break;
                case L'n':  // Newline
                {
                    pstrText++;
                    if( (uStyle & DT_SINGLELINE) != 0 ) break;
                    bLineEnd = true;
                }
                    break;
                case L'p':  // Paragraph
                {
                    pstrText++;
                    if( pt.x > rc.left ) bLineEnd = true;
                    while( *pstrText > L'\0' && *pstrText <= L' ' ) pstrText = CharNextW(pstrText);
                    int cyLineExtra = (int)wcstol(pstrText, const_cast<wchar_t **>(&pstrText), 10);
                    aPIndentArray.Add((LPVOID)cyLineExtra);
                    cyLine = MAX((LONG)cyLine, tm.tmHeight + tm.tmExternalLeading + cyLineExtra);
                }
                    break;
                case L'v':  // Vertical Align
                {
                    pstrText++;
                    while( *pstrText > L'\0' && *pstrText <= L' ' ) pstrText = CharNextW(pstrText);
                    UIString sVAlignStyle;
                    while( *pstrText != L'\0' && *pstrText != L'>' && *pstrText != L'}' ) {
                        const wchar_t *pstrTemp = CharNextW(pstrText);
                        while( pstrText < pstrTemp) {
                            const char *utf8AlignValue = Ucs2ToUtf8(pstrText,1);
                            sVAlignStyle += utf8AlignValue;
                            delete []utf8AlignValue;
                            pstrText++;
                            //sVAlignStyle += *pstrText++;
                        }
                    }

                    UINT iVAlign = DT_BOTTOM;
                    if (sVAlignStyle.CompareNoCase(UIString{"center"}) == 0) iVAlign = DT_VCENTER;
                    else if (sVAlignStyle.CompareNoCase(UIString{"top"}) == 0) iVAlign = DT_TOP;
                    aVAlignArray.Add((LPVOID)iVAlign);
                }
                    break;
                case L'r':  // Raw Text
                {
                    pstrText++;
                    bInRaw = true;
                }
                    break;
                case L's':  // Selected text background color
                {
                    pstrText++;
                    bInSelected = !bInSelected;
                    if( bDraw && bLineDraw ) {
                        if( bInSelected ) ::SetBkMode(hDC, OPAQUE);
                        else ::SetBkMode(hDC, TRANSPARENT);
                    }
                }
                    break;
                case L'u':  // Underline text
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
                        GetFontTextMetrics(hDC,pFontInfo->GetHandle(),&tm);
                        ::SelectObject(hDC, pFontInfo->GetHandle());
                        cyLine = MAX((LONG)cyLine, tm.tmHeight + tm.tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                    }
                }
                    break;
                case L'x':  // X Indent
                {
                    pstrText++;
                    while( *pstrText > L'\0' && *pstrText <= L' ' ) pstrText = CharNextW(pstrText);
                    int iWidth = (int) wcstol(pstrText, const_cast<wchar_t **>(&pstrText), 10);
                    pt.x += iWidth;
                }
                    break;
                case L'y':  // Y Indent
                {
                    pstrText++;
                    while( *pstrText > L'\0' && *pstrText <= L' ' ) pstrText = CharNextW(pstrText);
                    cyLine = (int) wcstol(pstrText, const_cast<wchar_t**>(&pstrText), 10);
                }
                    break;
            }
            if( pstrNextStart != nullptr ) pstrText = pstrNextStart;
            else {
                while( *pstrText != L'\0' && *pstrText != L'>' && *pstrText != L'}' ) pstrText = CharNextW(pstrText);
                pstrText = ::CharNextW(pstrText);
            }
        }
        else if( !bInRaw && ( *pstrText == L'<' || *pstrText == L'{' ) && pstrText[1] == L'/' )
        {
            pstrText++;
            pstrText++;
            switch( *pstrText )
            {
                case L'c':
                {
                    pstrText++;
                    aColorArray.Remove(aColorArray.GetSize() - 1);
                    DWORD clrColor = dwTextColor;
                    if( aColorArray.GetSize() > 0 ) clrColor = (int)aColorArray.GetAt(aColorArray.GetSize() - 1);
                    ::SetTextColor(hDC, RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
                }
                    break;
                case L'p':
                    pstrText++;
                    if( pt.x > rc.left ) bLineEnd = true;
                    aPIndentArray.Remove(aPIndentArray.GetSize() - 1);
                    cyLine = MAX((LONG)cyLine, tm.tmHeight + tm.tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                    break;
                case L'v':
                    pstrText++;
                    aVAlignArray.Remove(aVAlignArray.GetSize() - 1);
                    break;
                case L's':
                {
                    pstrText++;
                    bInSelected = !bInSelected;
                    if( bDraw && bLineDraw ) {
                        if( bInSelected ) ::SetBkMode(hDC, OPAQUE);
                        else ::SetBkMode(hDC, TRANSPARENT);
                    }
                }
                    break;
                case L'a':
                {
                    if( iLinkIndex < nLinkRects ) {
                        if( !bLineDraw ) ::SetRect(&prcLinks[iLinkIndex], ptLinkStart.x, ptLinkStart.y, MIN(pt.x, rc.right), pt.y + tm.tmHeight + tm.tmExternalLeading);
                        iLinkIndex++;
                    }
                    aColorArray.Remove(aColorArray.GetSize() - 1);
                    DWORD clrColor = dwTextColor;
                    if( aColorArray.GetSize() > 0 ) clrColor = (int)aColorArray.GetAt(aColorArray.GetSize() - 1);
                    ::SetTextColor(hDC, RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
                    bInLink = false;
                }
                case L'b':
                case L'f':
                case L'i':
                case L'u':
                {
                    pstrText++;
                    aFontArray.Remove(aFontArray.GetSize() - 1);
                    UIFont* pFontInfo = (UIFont*)aFontArray.GetAt(aFontArray.GetSize() - 1);
                    if( pFontInfo == NULL ) pFontInfo = UIResourceMgr::GetInstance().GetFont(iDefaultFont);
                    if( tm.tmItalic && (!pFontInfo->GetItalic()) ) {
                        ABC abc;
                        ::GetCharABCWidths(hDC, ' ', ' ', &abc);
                        pt.x += abc.abcC / 2; // 简单修正一下斜体混排的问题, 正确做法应该是http://support.microsoft.com/kb/244798/en-us
                    }
                    GetFontTextMetrics(hDC,pFontInfo->GetHandle(),&tm);
                    ::SelectObject(hDC, pFontInfo->GetHandle());
                    cyLine = MAX((LONG)cyLine, tm.tmHeight + tm.tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                }
                    break;
            }
            while( *pstrText != L'\0' && *pstrText != L'>' && *pstrText != L'}' ) pstrText = CharNextW(pstrText);
            pstrText = CharNextW(pstrText);
        }
        else if( !bInRaw &&  *pstrText == L'<' && pstrText[2] == L'>' && (pstrText[1] == L'{'  || pstrText[1] == L'}') )
        {
            SIZE szSpace = { 0 };
            ::GetTextExtentPoint32W(hDC, &pstrText[1], 1, &szSpace);
            if( bDraw && bLineDraw ) {
                iVAlign = DT_BOTTOM;
                if (aVAlignArray.GetSize() > 0) iVAlign = (UINT)aVAlignArray.GetAt(aVAlignArray.GetSize() - 1);
                if (iVAlign == DT_VCENTER)
                    ::TextOutW(hDC, pt.x + cxOffset, pt.y + (cyLineHeight - tm.tmHeight - tm.tmExternalLeading) / 2,
                                &pstrText[1], 1);
                else if (iVAlign == DT_TOP) ::TextOutW(hDC, pt.x + cxOffset, pt.y, &pstrText[1], 1);
                else
                    ::TextOutW(hDC, pt.x + cxOffset, pt.y + cyLineHeight - tm.tmHeight - tm.tmExternalLeading,
                                &pstrText[1], 1);
            }
            pt.x += szSpace.cx;
            cxMaxWidth = MAX((LONG)cxMaxWidth, pt.x);
            cxLine = pt.x - rc.left;
            pstrText++;pstrText++;pstrText++;
        }
        else if( !bInRaw &&  *pstrText == L'{' && pstrText[2] == L'}' && (pstrText[1] == L'<'  || pstrText[1] == L'>') )
        {
            SIZE szSpace = { 0 };
            ::GetTextExtentPoint32W(hDC, &pstrText[1], 1, &szSpace);
            if( bDraw && bLineDraw ) {
                iVAlign = DT_BOTTOM;
                if (aVAlignArray.GetSize() > 0) iVAlign = (UINT)aVAlignArray.GetAt(aVAlignArray.GetSize() - 1);
                if (iVAlign == DT_VCENTER)
                    ::TextOutW(hDC, pt.x + cxOffset, pt.y + (cyLineHeight - tm.tmHeight - tm.tmExternalLeading) / 2,
                                &pstrText[1], 1);
                else if (iVAlign == DT_TOP) ::TextOutW(hDC, pt.x + cxOffset, pt.y, &pstrText[1], 1);
                else
                    ::TextOutW(hDC, pt.x + cxOffset, pt.y + cyLineHeight - tm.tmHeight - tm.tmExternalLeading,
                                &pstrText[1], 1);
            }
            pt.x += szSpace.cx;
            cxMaxWidth = MAX((LONG)cxMaxWidth, pt.x);
            cxLine = pt.x - rc.left;
            pstrText++;pstrText++;pstrText++;
        }
        else if( !bInRaw &&  *pstrText == L' ' )
        {
            SIZE szSpace = { 0 };
            ::GetTextExtentPoint32(hDC, " ", 1, &szSpace);
            // Still need to paint the space because the font might have
            // underline formatting.
            if( bDraw && bLineDraw ) {
                iVAlign = DT_BOTTOM;
                if (aVAlignArray.GetSize() > 0) iVAlign = (UINT)aVAlignArray.GetAt(aVAlignArray.GetSize() - 1);
                if (iVAlign == DT_VCENTER)
                    ::TextOut(hDC, pt.x + cxOffset, pt.y + (cyLineHeight - tm.tmHeight - tm.tmExternalLeading) / 2,
                                " ", 1);
                else if (iVAlign == DT_TOP) ::TextOut(hDC, pt.x + cxOffset, pt.y, " ", 1);
                else ::TextOut(hDC, pt.x + cxOffset, pt.y + cyLineHeight - tm.tmHeight - tm.tmExternalLeading, " ", 1);
            }
            pt.x += szSpace.cx;
            cxMaxWidth = MAX((LONG)cxMaxWidth, pt.x);
            cxLine = pt.x - rc.left;
            pstrText++;
        }
        else
        {
            int cchChars = 0;
            int cchSize = 0;
            int cchLastGoodWord = 0;
            int cchLastGoodSize = 0;
            const wchar_t *p = pstrText;
            const wchar_t *pstrNext;
            SIZE szText = { 0 };
            if( !bInRaw && *p == L'<' || *p == L'{' ) p++, cchChars++, cchSize++;
            while( *p != L'\0' && *p != L'\n' ) {
                // This part makes sure that we're word-wrapping if needed or providing support
                // for DT_END_ELLIPSIS. Unfortunately the GetTextExtentPoint32() call is pretty
                // slow when repeated so often.
                // TODO: Rewrite and use GetTextExtentExPoint() instead!
                if( bInRaw ) {
                    if( ( *p == L'<' || *p == L'{' ) && p[1] == L'/'
                        && p[2] == L'r' && ( p[3] == L'>' || p[3] == L'}' ) ) {
                        p += 4;
                        bInRaw = false;
                        break;
                    }
                }
                else {
                    if( *p == L'<' || *p == L'{' ) break;
                }
                pstrNext = CharNextW(p);
                cchChars++;
                cchSize += (int)(pstrNext - p);
                szText.cx = cchChars * tm.tmMaxCharWidth;
                if( pt.x + szText.cx >= rc.right ) {
                    ::GetTextExtentPoint32W(hDC, pstrText, cchSize, &szText);
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
                        const wchar_t *pstrPrev = ::CharPrevW(pstrText, p);
                        if( cchChars > 0 ) {
                            cchChars -= 1;
                            pstrPrev = ::CharPrevW(pstrText, pstrPrev);
                            cchSize -= (int)(p - pstrPrev);
                        }
                        else
                            cchSize -= (int)(p - pstrPrev);
                        pt.x = rc.right;
                    }
                    bLineEnd = true;
                    cxMaxWidth = MAX((LONG)cxMaxWidth, pt.x);
                    cxLine = pt.x - rc.left;
                    break;
                }
                if (!( ( p[0] >= L'a' && p[0] <= L'z' ) || ( p[0] >= L'A' && p[0] <= L'Z' ) )) {
                    cchLastGoodWord = cchChars;
                    cchLastGoodSize = cchSize;
                }
                if( *p == L' ' ) {
                    cchLastGoodWord = cchChars;
                    cchLastGoodSize = cchSize;
                }
                p = ::CharNextW(p);
            }

            ::GetTextExtentPoint32W(hDC, pstrText, cchSize, &szText);
            if( bDraw && bLineDraw ) {
                iVAlign = DT_BOTTOM;
                if (aVAlignArray.GetSize() > 0) iVAlign = (UINT)aVAlignArray.GetAt(aVAlignArray.GetSize() - 1);
                if (iVAlign == DT_VCENTER)
                    ::TextOutW(hDC, pt.x + cxOffset, pt.y + (cyLineHeight - tm.tmHeight - tm.tmExternalLeading) / 2,
                                pstrText, cchSize);
                else if (iVAlign == DT_TOP) ::TextOutW(hDC, pt.x + cxOffset, pt.y, pstrText, cchSize);
                else
                    ::TextOutW(hDC, pt.x + cxOffset, pt.y + cyLineHeight - tm.tmHeight - tm.tmExternalLeading,
                                pstrText, cchSize);

                if( pt.x >= rc.right && (uStyle & DT_END_ELLIPSIS) != 0 ) {
                    if (iVAlign == DT_VCENTER) ::TextOut(hDC, pt.x + cxOffset + szText.cx, pt.y + (cyLineHeight -
                                                                                                     tm.tmHeight -
                                                                                                     tm.tmExternalLeading) /
                                                                                                    2, "...", 3);
                    else if (iVAlign == DT_TOP) ::TextOut(hDC, pt.x + cxOffset + szText.cx, pt.y, "...", 3);
                    else
                        ::TextOut(hDC, pt.x + cxOffset + szText.cx,
                                    pt.y + cyLineHeight - tm.tmHeight - tm.tmExternalLeading, "...", 3);
                }
            }
            pt.x += szText.cx;
            cxMaxWidth = MAX((LONG)cxMaxWidth, pt.x);
            cxLine = pt.x - rc.left;
            pstrText += cchSize;
        }

        if( pt.x >= rc.right || *pstrText == L'\n' || *pstrText == L'\0' ) bLineEnd = true;
        if( bDraw && bLineEnd ) {
            if( !bLineDraw ) {
                aFontArray.Resize(aLineFontArray.GetSize());
                ::CopyMemory(aFontArray.GetData(), aLineFontArray.GetData(), aLineFontArray.GetSize() * sizeof(LPVOID));
                aColorArray.Resize(aLineColorArray.GetSize());
                ::CopyMemory(aColorArray.GetData(), aLineColorArray.GetData(), aLineColorArray.GetSize() * sizeof(LPVOID));
                aPIndentArray.Resize(aLinePIndentArray.GetSize());
                ::CopyMemory(aPIndentArray.GetData(), aLinePIndentArray.GetData(), aLinePIndentArray.GetSize() * sizeof(LPVOID));
                aVAlignArray.Resize(aLineVAlignArray.GetSize());
                ::CopyMemory(aVAlignArray.GetData(), aLineVAlignArray.GetData(), aLineVAlignArray.GetSize() * sizeof(LPVOID));

                cxLineWidth = cxLine;
                cyLineHeight = cyLine;
                pstrText = pstrLineBegin;
                bInRaw = bLineInRaw;
                bInSelected = bLineInSelected;

                DWORD clrColor = dwTextColor;
                if( aColorArray.GetSize() > 0 ) clrColor = (int)aColorArray.GetAt(aColorArray.GetSize() - 1);
                ::SetTextColor(hDC, RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
                auto* pFontInfo = (UIFont*)aFontArray.GetAt(aFontArray.GetSize() - 1);
                if( pFontInfo == NULL ) pFontInfo = UIResourceMgr::GetInstance().GetFont(iDefaultFont);
                GetFontTextMetrics(hDC, pFontInfo->GetHandle(), &tm);
                ::SelectObject(hDC, pFontInfo->GetHandle());
                if( bInSelected ) ::SetBkMode(hDC, OPAQUE);
            }
            else {
                aLineFontArray.Resize(aFontArray.GetSize());
                ::CopyMemory(aLineFontArray.GetData(), aFontArray.GetData(), aFontArray.GetSize() * sizeof(LPVOID));
                aLineColorArray.Resize(aColorArray.GetSize());
                ::CopyMemory(aLineColorArray.GetData(), aColorArray.GetData(), aColorArray.GetSize() * sizeof(LPVOID));
                aLinePIndentArray.Resize(aPIndentArray.GetSize());
                ::CopyMemory(aLinePIndentArray.GetData(), aPIndentArray.GetData(), aPIndentArray.GetSize() * sizeof(LPVOID));
                aLineVAlignArray.Resize(aVAlignArray.GetSize());
                ::CopyMemory(aLineVAlignArray.GetData(), aVAlignArray.GetData(), aVAlignArray.GetSize() * sizeof(LPVOID));

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
        rc.bottom = MAX((LONG)cyMinHeight, pt.y + cyLine);
        rc.right = MIN(rc.right, (LONG)cxMaxWidth);
    }

    if( bDraw ) ::SelectClipRgn(hDC, hOldRgn);
    ::DeleteObject(hOldRgn);
    ::DeleteObject(hRgn);

    ::SelectObject(hDC, hOldFont);
    delete []wideString;
}
