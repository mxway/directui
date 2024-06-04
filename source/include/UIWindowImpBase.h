#ifndef DIRECTUI_UIWINDOWIMPBASE_H
#define DIRECTUI_UIWINDOWIMPBASE_H
#include <UIBaseWindow.h>
#include <NotifyPump.h>

class UIWindowImpBase : public UIBaseWindow, public INotifyUI, public UINotifyPump
{
    DUI_DECLARE_MESSAGE_MAP()
public:
    long HandleMessage(uint32_t uMsg, WPARAM wParam, LPARAM lParam) override;

    //long OnCreate(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) override;

    void Notify(TNotifyUI &msg) override;

    virtual UIString  GetSkinFile()const{
        return UIString{"skin.xml"};
    }

    virtual long OnKillFocus(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled);
    virtual long OnCreate(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled);
    virtual long OnClose(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled);
    virtual long OnDestroy(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled);
    virtual long OnSize(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled);

};

#endif //DIRECTUI_UIWINDOWIMPBASE_H