#ifndef DIRECTUI_RICHLISTWND_H
#define DIRECTUI_RICHLISTWND_H
#include <UIWindowImpBase.h>
#include "UIButton.h"

class RichListWnd : public UIWindowImpBase{
DUI_DECLARE_MESSAGE_MAP()
public:
    RichListWnd();
    long OnDestroy(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) override;

    long OnCreate(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) override;

    void   Init();

    void Notify(TNotifyUI &msg) override;

    UIString GetSkinFile() const override;
    void OnClick(TNotifyUI& msg);
    void OnSelectChanged( TNotifyUI &msg );
    void OnItemClick( TNotifyUI &msg );
private:
    UIButton* m_pCloseBtn;
    UIButton* m_pMaxBtn;
    UIButton* m_pRestoreBtn;
    UIButton* m_pMinBtn;
};

#endif //DIRECTUI_RICHLISTWND_H
