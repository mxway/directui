#ifndef DIRECTUI_RICHEDITFRAME_H
#define DIRECTUI_RICHEDITFRAME_H
#include "UIWindowImpBase.h"


class RichEditFrame : public UIWindowImpBase{
DUI_DECLARE_MESSAGE_MAP()
public:
    RichEditFrame();
    virtual ~RichEditFrame();

    long OnCreate(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;

    UIString GetSkinFile() const override {
        return UIString{u8"main.xml"};
    }

    void Notify(TNotifyUI& msg) override;

    long OnKeyPress(uint32_t uMsg, uint32_t keyCode, bool& bHandled) override;

private:
    void Init();
};



#endif //DIRECTUI_RICHEDITFRAME_H
