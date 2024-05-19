#ifndef DIRECTUI_UIRECT_H
#define DIRECTUI_UIRECT_H
#include <UIDefine.h>

bool UIIntersectRect(
        LPRECT prcDst,
        const RECT *prcSrc1,
        const RECT *prcSrc2);

uint64_t UIGetTickCount();

class UIRect : public RECT
{
public:
    UIRect();
    explicit UIRect(const RECT &src);
    explicit UIRect(long left, long top,long right, long bottom);
    explicit UIRect(const UIString &value);
    UIString    ToString()const;

    bool    IsEmpty();

    int     GetWidth()const;
    int     GetHeight()const;
    void    Empty();
    bool    IsNull()const;
    void    Join(const RECT &rc);
    void    ResetOffset();
    void    Normalize();
    void    Offset(int cx,int cy);
    void    Inflate(int cx, int cy);
    void    Deflate(int cx, int cy);
    void    Union(UIRect &rc);
    bool    IsPtIn(POINT pt)const;
};

#endif //DIRECTUI_UIRECT_H