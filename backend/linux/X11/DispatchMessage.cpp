#include "DispatchMessage.h"
#include "UIDefine.h"
#include <UIBackend.h>

#include "DisplayInstance.h"
#include "UIBaseWindowObjects.h"

static void DispatchMessage(Window window, uint32_t msgType, WPARAM wParam,LPARAM lParam){
    UIBaseWindow *baseWindow = UIBaseWindowObjects::GetInstance().GetObject(window);
    if(baseWindow == nullptr){
        return;
    }
    baseWindow->HandleMessage(msgType,wParam,lParam);
    if(msgType == DUI_WM_DESTROY){
        UIBaseWindowObjects::GetInstance().RemoveObject(window);
        baseWindow->OnFinalMessage(nullptr);
    }
}

void DispatchMessage(XEvent& event) {
    Atom wm_protocols = XInternAtom(DisplayInstance::GetInstance().GetDisplay(), "WM_PROTOCOLS", False);
    Atom wm_delete_window = XInternAtom(DisplayInstance::GetInstance().GetDisplay(), "WM_DELETE_WINDOW", False);
    switch(event.type){
        case DestroyNotify: {
            DispatchMessage(event.xdestroywindow.window,DUI_WM_DESTROY,&event.xdestroywindow,nullptr);
            break;
        }
        case Expose: {
            DispatchMessage(event.xexpose.window,DUI_WM_PAINT,&event.xexpose,nullptr);
            break;
        }
        case KeyPress: {
            DispatchMessage(event.xkey.window,DUI_WM_KEYPRESS, &event.xkey,nullptr);
            break;
        }
        case KeyRelease: {
            DispatchMessage(event.xkey.window,DUI_WM_KEYRELEASE,&event.xkey,nullptr);
            break;
        }
        case ButtonPress: {
            if(event.xbutton.button == 4 || event.xbutton.button==5){
                DispatchMessage(event.xbutton.window,DUI_WM_MOUSEWHEEL,&event.xbutton,nullptr);
            }else{
                DispatchMessage(event.xbutton.window,DUI_WM_MOUSEPRESS,&event.xbutton,nullptr);
            }
            break;
        }
        case ButtonRelease: {
            DispatchMessage(event.xbutton.window,DUI_WM_MOUSERELEASE,&event.xbutton,nullptr);
            break;
        }
        case MotionNotify: {
            DispatchMessage(event.xmotion.window,DUI_WM_MOUSEMOVE, &event.xmotion,nullptr);
            break;
        }
        case EnterNotify: {
            DispatchMessage(event.xcrossing.window,DUI_WM_MOUSEENTER,&event.xcrossing,nullptr);
            break;
        }
        case LeaveNotify: {
            DispatchMessage(event.xcrossing.window,DUI_WM_MOUSEENTER,&event.xcrossing,nullptr);
            break;
        }
        case FocusOut: {
            DispatchMessage(event.xfocus.window,DUI_WM_KILLFOCUS,&event.xfocus,nullptr);
            break;
        }
        case ConfigureNotify: {
            DispatchMessage(event.xconfigure.window,DUI_WM_SIZE, &event.xconfigure,nullptr);
            break;
        }
        case ClientMessage: {
            if (event.xclient.message_type == wm_protocols &&
                event.xclient.data.l[0] == wm_delete_window) {
                DispatchMessage(event.xclient.window,DUI_WM_CLOSE,&event.xclient,nullptr);
            }else if (event.xclient.message_type == XInternAtom(DisplayInstance::GetInstance().GetDisplay(), "UI_WINDOW_CLOSE_RESPONSE", False)) {
                    DispatchMessage(event.xclient.window,DUI_WM_CLOSE,&event.xclient,nullptr);
            }
        }
        default:
            break;
    }
}
