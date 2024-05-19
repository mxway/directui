#ifndef DIRECTUI_MAINFRAME_H
#define DIRECTUI_MAINFRAME_H
#include <UIWindowImpBase.h>
#include <IDialogBuilderCallback.h>

class MainFrame : public UIWindowImpBase, public IDialogBuilderCallback
{
    DUI_DECLARE_MESSAGE_MAP()
public:
    long OnDestroy(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) override;

    long OnCreate(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) override;

    UIControl *CreateControl(const char *pstrClass) override;

private:
    void    OnClick(TNotifyUI &msg);
    void    OnItemSelected(TNotifyUI &msg);
};

#endif //DIRECTUI_MAINFRAME_H
