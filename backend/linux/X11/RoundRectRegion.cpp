#include "RoundRectRegion.h"
#include <vector>
#include <cmath>
#include "X11HDC.h"

using namespace std;

enum CircleCorner{
    TopLeft,
    TopRight,
    BottomRight,
    BottomLeft
};

static bool IsPointExistsInVector(vector<XPoint> &points, XPoint point){
    return std::find_if(points.begin(),points.end(),[&](const XPoint &iter){
        return iter.x == point.x && iter.y == point.y;
    }) != points.end();
}

#define DISTANCE() \
        sqrt(((px)-(cx))*((px)-(cx)) + ((py)-(cy))*((py)-(cy)))



#define TopLeft1()                         \
        do{                                             \
             px =    (cx) - (x) + (i);                        \
             py =    (cy) - (y) + (j);                        \
             point.x = px;                              \
             point.y = py;                              \
             double dist = DISTANCE(); \
             coverage = fmax(0, fmin(1, 0.5 - fabs(dist - r)));\
        }while(0);


#define TopLeft2()                         \
        do{                                             \
              px =       (cx) - (y) + (i);                    \
              py =       (cy) - (x) + (j);                    \
              point.x  = px;                            \
              point.y =  py;                            \
              double dist = DISTANCE();      \
              coverage = fmax(0, fmin(1, 0.5 - fabs(dist - r))); \
        }while(0);


//{cx + x + i, cy - y + j}, // 右-上－下
//                        {cx + y + i, cy - x + j}, // 右－上－上

#define TopRight1()                    \
        do{                                         \
              px = (cx) + (x) + (i);                \
              py = (cy) - (y) + (j);                \
              point.x = px;                         \
              point.y = py;                         \
              double dist = DISTANCE();  \
              coverage = fmax(0, fmin(1, 0.5 - fabs(dist - r)));\
        }while(0)


#define TopRight2()                            \
        do{                                                 \
             px = cx+y+i;                                   \
             py = cy-x+j;                                   \
             point.x = px;                                  \
             point.y = py;                                      \
             double dist = DISTANCE();               \
             coverage = fmax(0, fmin(1, 0.5 - fabs(dist - r))); \
        }while(0)


//{cx + x + i, cy + y + j}, // 右-下-下
//                        {cx + y + i, cy + x + j}, // 右－下－上

#define BottomRight1()              \
        do{                         \
            px = cx+x+i;            \
            py = cy+y+j;            \
            point.x = px;              \
            point.y = py;              \
            double dist = DISTANCE(); \
            coverage = fmax(0, fmin(1, 0.5 - fabs(dist - r)));\
        }while(0)


#define BottomRight2()              \
        do{                         \
            px = cx + y + i;         \
            py = cy + x + j;         \
            point.x = px;              \
            point.y = py;              \
            double dist = DISTANCE(); \
            coverage = fmax(0, fmin(1, 0.5 - fabs(dist - r)));\
        }while(0)


//{cx - x + i, cy + y + j}, // 左－下－下
//                        {cx - y + i, cy + x + j} // 左－下－上

#define BottomLeft1() \
        do{                         \
            px = cx -x + i;         \
            py = cy + y +j;         \
            point.x = px;              \
            point.y = py;              \
            double dist = DISTANCE(); \
            coverage = fmax(0, fmin(1, 0.5 - fabs(dist - r)));\
        }while(0)

#define BottomLeft2()           \
        do{                         \
            px = cx -y +i;         \
            py = cy + x +j;         \
            point.x = px;              \
            point.y = py;              \
            double dist = DISTANCE(); \
            coverage = fmax(0, fmin(1, 0.5 - fabs(dist - r)));\
        }while(0)   \


#define CONCAT(x,y) x ## y

#define CreateCorner(CornerType) \
        CONCAT(CornerType,1)();    \
        if(coverage>=0.30 && (!IsPointExistsInVector(circlePoints,point))){ \
            circlePoints.push_back(point);                    \
        }                                                                      \
        CONCAT(CornerType,2)();                                               \
        if(coverage>=0.30 && (!IsPointExistsInVector(circlePoints,point))){   \
            circlePoints.push_back(point);                                    \
        }


static void GetPoints(int cx,int cy,int r, CircleCorner cornerType,vector<XPoint> &circlePoints){
    int x = 0, y = r;
    int d = 1 - r;
    int deltaE = 3, deltaSE = -2 * r + 5;

    while (x <= y) {
        // 处理当前点及其对称点
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {

                int px,py;
                XPoint point;
                double coverage;
                if(cornerType == TopLeft){
                    CreateCorner(TopLeft);
                }else if(cornerType == TopRight){
                    CreateCorner(TopRight);
                }else if(cornerType == BottomRight){
                    CreateCorner(BottomRight);
                }else if(cornerType == BottomLeft){
                    CreateCorner(BottomLeft);
                }
            }
        }

        // 更新决策变量
        if (d < 0) {
            d += deltaE;
            deltaE += 2;
            deltaSE += 2;
        } else {
            d += deltaSE;
            deltaE += 2;
            deltaSE += 4;
            y--;
        }
        x++;
    }
    auto begin = circlePoints.begin();
    //第一个为顶点元素，不能参与排序。否则创建的区域不正常。
    begin++;
    std::sort(begin,circlePoints.end(),[&](const XPoint &point1, const XPoint &point2 ){
        if(point1.x == point2.x){
            return point1.y < point2.y;
        }
        return point1.x < point2.x;
    });
}

Region CreateRoundRegion(int cx, int cy, int r, CircleCorner cornerType){
    std::vector<XPoint> circlePoints;
    if(cornerType == TopLeft){
        circlePoints.push_back({static_cast<short>(cx-r),static_cast<short>(cy-r)});
    }else if(cornerType == TopRight){
        circlePoints.push_back({ static_cast<short>(cx+r), static_cast<short>(cy-r)});
    }else if(cornerType == BottomRight){
        circlePoints.push_back({static_cast<short>(cx + r), static_cast<short>(cy+r)});
    }else if(cornerType == BottomLeft){
        circlePoints.push_back({static_cast<short>(cx-r), static_cast<short>(cy+r)});
    }
    GetPoints(cx,cy,r,cornerType,circlePoints);
    return XPolygonRegion(circlePoints.data(),circlePoints.size(),EvenOddRule);
}

Region CreateRoundRectRegion(const UIRect &rect,int roundCornerRadius){
    Region region = XCreateRegion();
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    XRectangle  regionRect = {static_cast<short>(rect.left),static_cast<short>(rect.top),
                              static_cast<unsigned short>(width),
                              static_cast<unsigned short>(height)};
    XUnionRectWithRegion(&regionRect, region,region);

    Region topLeftRegion = CreateRoundRegion(rect.left + roundCornerRadius,
                                             rect.top + roundCornerRadius,roundCornerRadius,TopLeft);
    XSubtractRegion(region, topLeftRegion, region);
    XDestroyRegion(topLeftRegion);

    Region topRightRegion = CreateRoundRegion(rect.left + width-roundCornerRadius,
                                              rect.top + roundCornerRadius,roundCornerRadius,TopRight);
    XSubtractRegion(region, topRightRegion, region);
    XDestroyRegion(topRightRegion);

    Region bottomRightRegion = CreateRoundRegion(rect.left + width-roundCornerRadius,
                                                 rect.top + height-roundCornerRadius, roundCornerRadius, BottomRight);
    XSubtractRegion(region, bottomRightRegion, region);
    XDestroyRegion(bottomRightRegion);

    Region bottomLeftRegion = CreateRoundRegion(rect.left + roundCornerRadius,
                                                rect.top + height-roundCornerRadius,roundCornerRadius,BottomLeft);
    XSubtractRegion(region, bottomLeftRegion, region);
    XDestroyRegion(bottomLeftRegion);
    return region;
}

void DrawRoundRect_Internal(HANDLE_DC hDC, const RECT &rc, int radiusWeight, int radiusHeight, int nSize, uint32_t dwPenColor,
                            int nStyle)
{
    XSetForeground(hDC->x11Window->display,hDC->gc,dwPenColor);
    XSetLineAttributes(hDC->x11Window->display,hDC->gc,nSize,nStyle,CapButt, JoinMiter);
    std::vector<XPoint> circlePoints;
    GetPoints(rc.left + radiusWeight,
              rc.top + radiusWeight,radiusWeight,TopLeft,circlePoints);
    XDrawLines(hDC->x11Window->display,hDC->drawablePixmap,hDC->gc,circlePoints.data(),circlePoints.size(),CoordModeOrigin);

    circlePoints.clear();
    GetPoints(rc.left + radiusWeight,
              rc.top + radiusWeight,radiusWeight,TopRight,circlePoints);
    XDrawLines(hDC->x11Window->display,hDC->drawablePixmap,hDC->gc,circlePoints.data(),circlePoints.size(),CoordModeOrigin);

    circlePoints.clear();
    GetPoints(rc.left + radiusWeight,
              rc.top + radiusWeight,radiusWeight,BottomLeft,circlePoints);
    XDrawLines(hDC->x11Window->display,hDC->drawablePixmap,hDC->gc,circlePoints.data(),circlePoints.size(),CoordModeOrigin);

    circlePoints.clear();
    GetPoints(rc.left + radiusWeight,
              rc.top + radiusWeight,radiusWeight,BottomRight,circlePoints);
    XDrawLines(hDC->x11Window->display,hDC->drawablePixmap,hDC->gc,circlePoints.data(),circlePoints.size(),CoordModeOrigin);

    XDrawLine(hDC->x11Window->display,hDC->drawablePixmap,
              hDC->gc,rc.left + radiusWeight,rc.top,
              rc.right-radiusWeight,rc.top);
    XDrawLine(hDC->x11Window->display,hDC->drawablePixmap,
              hDC->gc, rc.right,rc.top,rc.right,rc.bottom-radiusWeight);
    XDrawLine(hDC->x11Window->display,hDC->drawablePixmap,hDC->gc,
              rc.left+radiusWeight,rc.bottom,rc.right-radiusWeight,rc.bottom);
    XDrawLine(hDC->x11Window->display,hDC->drawablePixmap,hDC->gc,
              rc.left,rc.top + radiusWeight,rc.left,rc.bottom-radiusWeight);
}
