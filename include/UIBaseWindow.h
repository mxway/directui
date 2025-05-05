#ifndef DIRECTUI_UIBASEWINDOW_H
#define DIRECTUI_UIBASEWINDOW_H
#include "UIDefine.h"
#include "UIString.h"
#include <memory>
#include "UIPaintManager.h"
#include "UIBackend.h"

using namespace std;

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
