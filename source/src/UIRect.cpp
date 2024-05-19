#include <UIRect.h>
#include <cassert>
#include <cstring>

#ifndef WIN32
bool SetRectEmpty(
    LPRECT prc)
{
    if (prc == NULL) return FALSE;
    memset(prc, 0, sizeof(RECT));
    return TRUE;
}

bool UnionRect(
    LPRECT prcDst,
    const RECT *prcSrc1,
    const RECT *prcSrc2)
{
    bool frc1Empty, frc2Empty;

    if ((prcDst == NULL) || (prcSrc1 == NULL) || (prcSrc2 == NULL)) return false;

    frc1Empty = ((prcSrc1->left >= prcSrc1->right) ||
            (prcSrc1->top >= prcSrc1->bottom));

    frc2Empty = ((prcSrc2->left >= prcSrc2->right) ||
            (prcSrc2->top >= prcSrc2->bottom));

    if (frc1Empty && frc2Empty) {
        SetRectEmpty(prcDst);
        return false;
    }

    if (frc1Empty) {
        *prcDst = *prcSrc2;
        return true;
    }

    if (frc2Empty) {
        *prcDst = *prcSrc1;
        return true;
    }

    /*
     * form the union of the two non-empty rects
     */
    prcDst->left   = MIN(prcSrc1->left,   prcSrc2->left);
    prcDst->top    = MIN(prcSrc1->top,    prcSrc2->top);
    prcDst->right  = MAX(prcSrc1->right,  prcSrc2->right);
    prcDst->bottom = MAX(prcSrc1->bottom, prcSrc2->bottom);

    return true;
}

#endif

bool UIIntersectRect(
        LPRECT prcDst,
        const RECT *prcSrc1,
        const RECT *prcSrc2)

{
#ifndef WIN32
    if ((prcDst == NULL) || (prcSrc1 == NULL) || (prcSrc2 == NULL))
        return false;

    prcDst->left  = MAX(prcSrc1->left, prcSrc2->left);
    prcDst->right = MIN(prcSrc1->right, prcSrc2->right);

    /*
     * check for empty rect
     */
    if (prcDst->left < prcDst->right) {

        prcDst->top = MAX(prcSrc1->top, prcSrc2->top);
        prcDst->bottom = MIN(prcSrc1->bottom, prcSrc2->bottom);

        /*
         * check for empty rect
         */
        if (prcDst->top < prcDst->bottom) {
            return true;        // not empty
        }
    }

    /*
     * empty rect
     */
    SetRectEmpty(prcDst);

    return false;
#else
    return IntersectRect(prcDst, prcSrc1, prcSrc2);
#endif
}

uint64_t UIGetTickCount()
{
#ifndef WIN32
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
#else
    return GetTickCount();
#endif

}

UIRect::UIRect()
    : RECT()
{

}

UIRect::UIRect(const RECT &src)
    :UIRect()
{
    left     = src.left;
    top      = src.top;
    right    = src.right;
    bottom   = src.bottom;
}

UIRect::UIRect(long left, long top, long right, long bottom)
    :UIRect()
{
    this->left     = left;
    this->top      = top;
    this->right    = right;
    this->bottom   = bottom;
}

UIRect::UIRect(const UIString &value)
    :UIRect()
{
    if(value.IsEmpty()){
        return;
    }
    const char *pstrValue = value.GetData();
    char *pstr = nullptr;
    left = top = right = bottom = strtol(pstrValue, &pstr, 10);assert(pstr);
    top = bottom = strtol(pstr+1, &pstr, 10);assert(pstr);
    right = strtol(pstr+1, &pstr, 10);assert(pstr);
    bottom = strtol(pstr+1, &pstr, 10);assert(pstr);
}

UIString UIRect::ToString() const {
    UIString    sRect;
    sRect.Format("（%ld,%ld,%ld,%ld)",left, top, right, bottom);
    return sRect;
}

bool UIRect::IsEmpty() {
    return right<=left || bottom<=top;
}

int UIRect::GetWidth() const {
    return right - left;
}

int UIRect::GetHeight() const {
    return bottom - top;
}

void UIRect::Empty() {
    left = right = top = bottom = 0;
}

bool UIRect::IsNull() const {
    return (left==0 && right==0 && top==0 && bottom==0);
}

void UIRect::Join(const RECT &rc) {
    if(rc.left < left){
        left = rc.left;
    }
    if(rc.top < top){
        top = rc.top;
    }
    if(rc.right > right){
        right = rc.right;
    }
    if(rc.bottom > bottom){
        bottom = rc.bottom;
    }
}

void UIRect::ResetOffset() {
    this->Offset(-left, -top);
}

void UIRect::Normalize() {
    if(left > right){
        long temp = left;
        left = right;
        right = temp;
    }
    if(top > bottom){
        long temp = top;
        top = bottom;
        bottom = temp;
    }
}

void UIRect::Offset(int cx, int cy) {
    left += cx;
    right += cx;
    bottom += cy;
    top += cy;
}

void UIRect::Inflate(int cx, int cy) {
    left     -= cx;
    right    += cx;
    top      -= cy;
    bottom   += cy;
}

void UIRect::Deflate(int cx, int cy) {
    this->Inflate(-cx, -cy);
}

void UIRect::Union(UIRect &rc) {
    UnionRect(this, this, &rc);
}

bool UIRect::IsPtIn(POINT pt)const {
    return ((pt.x>=left) && (pt.x<right)&&
            (pt.y>=top) && (pt.y<bottom));
}
