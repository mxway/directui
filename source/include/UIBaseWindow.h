#ifndef DIRECTUI_UIBASEWINDOW_H
#define DIRECTUI_UIBASEWINDOW_H
#include <UIDefine.h>
#include <UIString.h>
#include <memory>
#include <UIPaintManager.h>

using namespace std;

#ifdef WIN32
#define DUI_WM_PAINT                WM_PAINT
#define DUI_WM_SIZE                 WM_SIZE
#define DUI_WM_MOUSEMOVE            WM_MOUSEMOVE
#define DUI_WM_MOUSEPRESS           0x10000001
#define DUI_WM_MOUSERELEASE         0x10000002
#define DUI_WM_MOUSEENTER           WM_MOUSEHOVER
#define DUI_WM_MOUSELEAVE           WM_MOUSELEAVE
#define DUI_WM_KEYPRESS             WM_KEYDOWN
#define DUI_WM_KEYRELEASE           WM_KEYUP
#define DUI_WM_CLOSE                WM_CLOSE
#define DUI_WM_TIMER                WM_TIMER
#define DUI_WM_MOUSEWHEEL           WM_MOUSEWHEEL
#define DUI_WM_DESTROY              WM_DESTROY
#define DUI_WM_CREATE               WM_CREATE
#define DUI_WM_KILLFOCUS            WM_KILLFOCUS

#define UI_WNDSTYLE_FRAME      (WS_VISIBLE | WS_OVERLAPPEDWINDOW)
#define UI_WNDSTYLE_CHILD      (WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN)
#define UI_WNDSTYLE_DIALOG     (WS_VISIBLE | WS_POPUPWINDOW | WS_CAPTION | WS_DLGFRAME | WS_CLIPSIBLINGS | WS_CLIPCHILDREN)

#define UI_WNDSTYLE_EX_FRAME   (WS_EX_WINDOWEDGE)
#define UI_WNDSTYLE_EX_DIALOG  (WS_EX_TOOLWINDOW | WS_EX_DLGMODALFRAME)

#else
#define DUI_WM_PAINT                0x10000000
#define DUI_WM_SIZE                 0x10000001
#define DUI_WM_MOUSEMOVE            0x10000002
#define DUI_WM_MOUSEPRESS           0x10000003
#define DUI_WM_MOUSERELEASE         0x10000004
#define DUI_WM_MOUSEENTER           0x10000005
#define DUI_WM_MOUSELEAVE           0x10000006
#define DUI_WM_KEYPRESS             0x10000007
#define DUI_WM_KEYRELEASE           0x10000008
#define DUI_WM_CLOSE                0x10000009
#define DUI_WM_TIMER                0x1000000A
#define DUI_WM_MOUSEWHEEL           0x1000000B
#define DUI_WM_DESTROY              0x1000000C
#define DUI_WM_CREATE               0x1000000D
#define DUI_WM_KILLFOCUS            0x1000000E

#define UI_WNDSTYLE_FRAME      (GTK_WINDOW_TOPLEVEL)
#define UI_WNDSTYLE_CHILD      (GTK_WINDOW_POPUP)
#define UI_WNDSTYLE_DIALOG     (GTK_WINDOW_TOPLEVEL)

#endif

class UIBaseWindowPrivate;

enum DuiResponseVal
{
    DUI_RESPONSE_CLOSE,
    DUI_RESPONSE_OK,
    DUI_RESPONSE_CANCEL,
    DUI_RESPONSE_YES,
    DUI_RESPONSE_NO,
    DUI_RESPONSE_DENY,
    DUI_RESPONSE_RETRY,
};

class UIBaseWindow {
public:
    UIBaseWindow();

    HANDLE_WND  Create(HANDLE_WND  parent, const UIString &className, uint32_t style, uint32_t exStyle, RECT rc);
    HANDLE_WND  Create(HANDLE_WND  parent, const UIString &className, uint32_t style, uint32_t exStyle, int x, int y, int cx,int cy);
    void ShowWindow(bool bShow = true);
    DuiResponseVal ShowModal();
    HANDLE_WND GetWND();
    void       SetWND(HANDLE_WND wndHandle);
    void       Close(DuiResponseVal val=DUI_RESPONSE_OK){
        UI_CLOSE_WINDOW(GetWND(),val);
    }
    void       CenterWindow();

    void        Maximize();

    void        Restore();

    void        Minimize();

    virtual long HandleMessage(uint32_t uMsg, WPARAM wParam, LPARAM lParam);
    virtual void OnFinalMessage(HANDLE_WND hWnd);

protected:
    UIPaintManager  m_pm;
private:
    shared_ptr<UIBaseWindowPrivate> m_data;
};

#endif //DIRECTUI_UIBASEWINDOW_H
