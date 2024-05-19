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
#endif

class UIBaseWindowPrivate;

class UIBaseWindow {
public:
    UIBaseWindow();
    HANDLE_WND Create(HANDLE_WND parent, const UIString &className, int x, int y, int nWidth, int nHeight);
    void ShowWindow(bool bShow = true);
    HANDLE_WND GetWND();
    void       SetWND(HANDLE_WND wndHandle);
    void       Close(){
        UI_DESTROY_WINDOW(GetWND());
    }
    void       CenterWindow();

    void        Maximize();

    void        Restore();

    void        Minimize();

    virtual long HandleMessage(uint32_t uMsg, WPARAM wParam, LPARAM lParam);
    virtual void OnFinalMessage(HANDLE_WND hWnd);

    virtual long OnCreate(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled);
    virtual long OnClose(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled);
    virtual long OnDestroy(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled);
    virtual long OnSize(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled);
protected:
    UIPaintManager  m_pm;
private:
    shared_ptr<UIBaseWindowPrivate> m_data;
};

#endif //DIRECTUI_UIBASEWINDOW_H
