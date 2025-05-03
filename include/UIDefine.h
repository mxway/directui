#ifndef DIRECTUI_UIDEFINE_H
#define DIRECTUI_UIDEFINE_H
#include "UIString.h"
#include <cstdint>
#include <algorithm>

#ifdef WIN32
#include <windows.h>
#else
typedef struct tagSIZE
{
    long    cx;
    long    cy;
}SIZE, *PSIZE, *LPSIZE;

typedef struct tagRECT
{
    long left;
    long top;
    long right;
    long bottom;
}RECT, *PRECT, *LPRECT;

typedef struct tagPOINT {
  long x;
  long y;
} POINT,*PPOINT,*NPPOINT,*LPPOINT;

typedef void   *LPVOID;
typedef LPVOID WPARAM;
typedef LPVOID LPARAM;

typedef unsigned char *LPBYTE;

#if !defined(CharNext)
inline const char* CharNext(const char *p){
    u_char  character = *p;
    if(*p == 0){
        return p;
    }
    if( (character & 0x80) == 0){
        return (p+1);
    }else if( (character >> 5) == 0B110){
        return (p+2);
    }else if( (character>>4) == 0B1110){
        return (p+3);
    }else if( (character>>3) == 0B11110){
        return (p+4);
    }else if( (character>>2) == 0B111110){
        return (p+5);
    }else if( (character>>1) == 0B1111110){
        return (p+6);
    }
    return p+1;
}
#endif

#if !defined(CharPrev)
inline const char* CharPrev(const char *start, const char *current)
{
    if(start == current){
        return start;
    }
    const char *result = current - 1;
    while(result != start){
        u_char character = *result;
        if( (character>>6) == 0B10){
            result = result - 1;
        }else{
            break;
        }
    }
    return result;
}
#endif

#define LOWORD(l)           ((uint16_t )(((unsigned long)(l)) & 0xffff))
#define HIWORD(l)           ((uint16_t)((((unsigned long)(l)) >> 16) & 0xffff))
//#define LOBYTE(w)           ((uint8_t)(((unsigned long)(w)) & 0xff))
#define HIBYTE(w)           ((uint8_t)((((unsigned long)(w)) >> 8) & 0xff))
#endif

enum DuiSig
{
    DuiSig_end = 0,
    DuiSig_lwl,
    DuiSig_vn,
};

class UIControl;
typedef struct tagTNotifyUI
{
    UIString sType;
    UIString sVirtualWnd;
    UIControl  *pSender;
    uint64_t   dwTimestamp;
    POINT      ptMouse;
    WPARAM     wParam;
    LPARAM     lParam;
}TNotifyUI;

class UINotifyPump;

typedef void (UINotifyPump::*DUI_PMSG)(TNotifyUI &msg);

union UIMessageMapFunctions
{
    DUI_PMSG pfn;
    long (UINotifyPump::*pfn_Notify_lwl)(WPARAM, LPARAM);
    void    (UINotifyPump::*pfn_Notify_vn)(TNotifyUI &);
};

struct DUI_MSGMAP_ENTRY
{
    UIString    sMsgType;       //消息类型
    UIString    sCtrlName;      //控件名称
    uint32_t    nSig;           //标记函数指针类型
    DUI_PMSG    pfn;            //指向函数指针
};

struct DUI_MSGMAP
{
    const DUI_MSGMAP*  (*pfnGetBaseMap)();
    const DUI_MSGMAP_ENTRY  *lpEntries;
};

#define DUI_DECLARE_MESSAGE_MAP()                           \
private:                                                    \
    static const DUI_MSGMAP_ENTRY _messageEntries[];        \
protected:                                                  \
    static const DUI_MSGMAP     messageMap;                 \
    static const DUI_MSGMAP*  _GetBaseMessageMap();         \
    virtual const DUI_MSGMAP *GetMessageMap()const;


#define DUI_BASE_BEGIN_MESSAGE_MAP(theClass) \
    const DUI_MSGMAP*   theClass::_GetBaseMessageMap() \
        {return nullptr;}                        \
    const DUI_MSGMAP* theClass::GetMessageMap()const{   \
        return &theClass::messageMap;                   \
    }                                        \
    const DUI_MSGMAP    theClass::messageMap =          \
        {&theClass::_GetBaseMessageMap, &theClass::_messageEntries[0]}; \
    const DUI_MSGMAP_ENTRY theClass::_messageEntries[] =\
    {                                        \

#define DUI_BEGIN_MESSAGE_MAP(theClass, baseClass) \
    const DUI_MSGMAP*   theClass::_GetBaseMessageMap() \
        {return &baseClass::messageMap;}           \
    const DUI_MSGMAP*   theClass::GetMessageMap()const\
        {return &theClass::messageMap;}             \
    const DUI_MSGMAP   theClass::messageMap =      \
        {&theClass::_GetBaseMessageMap, &theClass::_messageEntries[0]}; \
    const DUI_MSGMAP_ENTRY theClass::_messageEntries[] =                \
    {

#define DUI_END_MESSAGE_MAP()           \
    {UIString{""},UIString{""},DuiSig_end, (DUI_PMSG)0}     \
};

typedef struct tagTEventUI
{
    int Type;
    UIControl   *pSender;
    uint64_t    dwTimestamp;
    POINT       ptMouse;
    uint16_t        chKey;
    uint16_t    wKeyState;
    WPARAM      wParam;
    LPARAM      lParam;
}TEventUI;

#define DUI_ON_MSGTYPE(msgtype, memberFxn)                                                  \
    {UIString{msgtype}, UIString{""}, DuiSig_vn, (DUI_PMSG)&memberFxn},


#define DUI_ON_MSGTYPE_CTRNAME(msgtype,ctrname,memberFxn)                                   \
    {UIString{msgtype}, UIString{ctrname}, DiSign_vn, (DUI_PMSG)&memberFxn},


#define DUI_ON_CLICK_CTRNAME(ctrname, memberFxn)                                            \
    {UIString{DUI_MSGTYPE_CLICK},UIString{ctrname}, DuiSig_vn, (DUI_PMSG)&memberFxn},


#define DUI_ON_SELECTCHANGED_CTRNAME(ctrname, memberFxn)                                    \
    {UIString{DUI_MSGTYPE_SELECTCHANGED},UIString{ctrname}, DuiSig_vn, (DUI_PMSG)&memberFxn},


#define DUI_ON_KILLFOCUS_CTRNAME(ctrname, memberFxn)                                        \
    {UIString{DUI_MSGTYPE_KILLFOCUS}, UIString{ctrname}, DuiSig_vn, (DUI_PMSG)&memberFxn},


#define DUI_ON_MENU_CTRNAME(ctrname,memberFxn)                                              \
    {UIString{DUI_MSGTYPE_MENU}, UIString{ctrname}, DuiSig_vn, (DUI_PMSG)&memberFxn},


#define DUI_ON_TIMER()                                                                      \
    {UIString{DUI_MSGTYPE_TIMER},UIString{""}, DuiSig_vn, (DUI_PMSG)&OnTTimer},


#define  DUI_CTR_EDIT                            "Edit"
#define  DUI_CTR_LIST                            "List"
#define  DUI_CTR_TEXT                            "Text"
#define  DUI_CTR_TREE                            "Tree"
#define  DUI_CTR_HBOX                            "HBox"
#define  DUI_CTR_VBOX                            "VBox"

#define  DUI_CTR_ILIST                           "IList"
#define  DUI_CTR_COMBO                           "Combo"
#define  DUI_CTR_LABEL                           "Label"
#define  DUI_CTR_FLASH							 "Flash"

#define  DUI_CTR_BUTTON                          "Button"
#define  DUI_CTR_OPTION                          "Option"
#define  DUI_CTR_SLIDER                          "Slider"

#define  DUI_CTR_CONTROL                         "Control"

#define  DUI_CTR_GIFANIM                         "GifAnim"

#define  DUI_CTR_PROGRESS                        "Progress"
#define  DUI_CTR_RICHEDIT                        "RichEdit"
#define  DUI_CTR_CHECKBOX                        "CheckBox"
#define  DUI_CTR_COMBOBOX                        "ComboBox"
#define  DUI_CTR_DATETIME                        "DateTime"
#define  DUI_CTR_TREEVIEW                        "TreeView"
#define  DUI_CTR_TREENODE                        "TreeNode"

#define  DUI_CTR_ILISTITEM                       "IListItem"
#define  DUI_CTR_CONTAINER                       "Container"
#define  DUI_CTR_TABLAYOUT                       "TabLayout"
#define  DUI_CTR_SCROLLBAR                       "ScrollBar"

#define  DUI_CTR_ICONTAINER                      "IContainer"
#define  DUI_CTR_ILISTOWNER                      "IListOwner"
#define  DUI_CTR_LISTHEADER                      "ListHeader"
#define  DUI_CTR_TILELAYOUT                      "TileLayout"
#define  DUI_CTR_WEBBROWSER                      "WebBrowser"

#define  DUI_CTR_CHILDLAYOUT                     "ChildLayout"
#define  DUI_CTR_LISTELEMENT                     "ListElement"
#define  DUI_CTR_VIRTUALLIST                     "VirtualList"

#define  DUI_CTR_VERTICALLAYOUT                  "VerticalLayout"
#define  DUI_CTR_LISTHEADERITEM                  "ListHeaderItem"

#define  DUI_CTR_LISTHBOXELEMENT                 "ListHBoxElement"
#define  DUI_CTR_LISTTEXTELEMENT                 "ListTextElement"

#define  DUI_CTR_HORIZONTALLAYOUT                "HorizontalLayout"
#define  DUI_CTR_LISTLABELELEMENT                "ListLabelElement"

#define  DUI_CTR_LISTCONTAINERELEMENT            "ListContainerElement"


//定义所有消息类型
//////////////////////////////////////////////////////////////////////////

#define DUI_MSGTYPE_MENU                   "menu"
#define DUI_MSGTYPE_LINK                   "link"

#define DUI_MSGTYPE_TIMER                  "timer"
#define DUI_MSGTYPE_CLICK                  "click"

#define DUI_MSGTYPE_RETURN                 "return"
#define DUI_MSGTYPE_SCROLL                 "scroll"

#define DUI_MSGTYPE_DROPDOWN               "dropdown"
#define DUI_MSGTYPE_SETFOCUS               "setfocus"

#define DUI_MSGTYPE_KILLFOCUS              "killfocus"
#define DUI_MSGTYPE_ITEMCLICK 		   	   "itemclick"
#define DUI_MSGTYPE_TABSELECT              "tabselect"

#define DUI_MSGTYPE_ITEMSELECT 		   	   "itemselect"
#define DUI_MSGTYPE_ITEMEXPAND             "itemexpand"

#define DUI_MSGTYPE_WINDOWINIT             "windowinit"
#define DUI_MSGTYPE_BUTTONDOWN 		   	   "buttondown"
#define DUI_MSGTYPE_MOUSEENTER			   "mouseenter"
#define DUI_MSGTYPE_MOUSELEAVE			   "mouseleave"

#define DUI_MSGTYPE_TEXTCHANGED            "textchanged"
#define DUI_MSGTYPE_HEADERCLICK            "headerclick"
#define DUI_MSGTYPE_ITEMDBCLICK            "itemdbclick"
#define DUI_MSGTYPE_SHOWACTIVEX            "showactivex"

#define DUI_MSGTYPE_ITEMCOLLAPSE           "itemcollapse"
#define DUI_MSGTYPE_ITEMACTIVATE           "itemactivate"
#define DUI_MSGTYPE_VALUECHANGED           "valuechanged"

#define DUI_MSGTYPE_SELECTCHANGED 		   "selectchanged"

#define SCROLLBAR_LINESIZE      8

#ifndef CLAMP
#define MAX std::max
#define MIN std::min
#define CLAMP(x,a,b) (MIN(b,MAX(a,x)))
#endif

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x) ((void)(x))
#endif

#define MAX_FONT_ID				30000


#endif //DIRECTUI_UIDEFINE_H
