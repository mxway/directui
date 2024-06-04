#ifndef DIRECTUI_LOGINFRAMEWND_H
#define DIRECTUI_LOGINFRAMEWND_H
#include <UIWindowImpBase.h>
#include <UICombo.h>
#include <UIEdit.h>

class LoginFrameWnd : public UIWindowImpBase{
public:
    void    Init();
    void    Notify(TNotifyUI &msg);

    long    OnCreate(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) override;
};

#endif //DIRECTUI_LOGINFRAMEWND_H
