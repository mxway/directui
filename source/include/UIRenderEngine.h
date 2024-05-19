#ifndef DIRECTUI_UIRENDERENGINE_H
#define DIRECTUI_UIRENDERENGINE_H
#include <UIDefine.h>


class UIPaintManager;

class UIRenderEngine
{
public:
    static uint32_t AdjustColor(uint32_t color, short H, short S, short L);
    static void DrawImage(HANDLE_DC hDC, HANDLE_BITMAP hBitmap, const RECT& rc, const RECT& rcPaint,
                          const RECT& rcBmpPart, const RECT& rcScale9, bool alpha, uint8_t uFade = 255,
                          bool hole = false, bool xtiled = false, bool ytiled = false);
    static bool DrawImage(HANDLE_DC hdc, UIPaintManager* manager, const RECT& rcItem, const RECT& rcPaint,
                          TDrawInfo& drawInfo);

    static void DrawColor(HANDLE_DC hDC, const RECT& rc, uint32_t color);
    static void DrawGradient(HANDLE_DC hDC, const RECT& rc, uint32_t dwFirst, uint32_t dwSecond, bool bVertical, int nSteps);

    // 以下函数中的颜色参数alpha值无效
    static void DrawLine(HANDLE_DC hDC, const RECT& rc, int nSize, uint32_t dwPenColor, int nStyle);
    static void DrawRect(HANDLE_DC hDC, const RECT& rc, int nSize, uint32_t dwPenColor, int nStyle );
    static void DrawRoundRect(HANDLE_DC hDC, const RECT& rc, int radiusWeight, int radiusHeight, int nSize, uint32_t dwPenColor, int nStyle);
    static void DrawText(HANDLE_DC hDC, UIPaintManager* pManager, RECT& rc, const UIString &text, \
        uint32_t dwTextColor, int iFont, uint32_t uStyle);
    static void DrawHtmlText(HANDLE_DC hDC, UIPaintManager* pManager, RECT& rc, const UIString &text,
                             uint32_t dwTextColor, RECT* pLinks, UIString* sLinks, int& nLinkRects, int iDefaultFont, uint32_t uStyle);

};

#endif //DIRECTUI_UIRENDERENGINE_H