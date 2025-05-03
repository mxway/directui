#ifndef DIRECTUI_COLOR_SKIN_H
#define DIRECTUI_COLOR_SKIN_H
#include <UIWindowImpBase.h>

class MainFrame;

class ColorSkinWindow  : public UIWindowImpBase{
public:
    ColorSkinWindow(MainFrame *main_frame, RECT recParentWindow);

    void    OnFinalMessage(HANDLE_WND wnd)override;

    void    Notify(TNotifyUI &msg)override;
    virtual void InitWindow();
    UIString     GetSkinFile()const override;
    long        OnKillFocus(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled)override;
private:
    RECT        parent_window_rect_;
    MainFrame   *main_frame_;
};

#endif //DIRECTUI_COLOR_SKIN_H
