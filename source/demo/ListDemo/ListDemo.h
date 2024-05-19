#ifndef DIRECTUI_LISTDEMO_H
#define DIRECTUI_LISTDEMO_H
#include <UIWindowImpBase.h>
#include <UIButton.h>
#include <UIList.h>

class ListDemo : public UIWindowImpBase, public IListCallbackUI {
DUI_DECLARE_MESSAGE_MAP()
public:
    ListDemo();
    long OnDestroy(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) override;

    long OnCreate(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) override;

    void   Init();

    void        OnSearch();

    UIString    GetItemText(UIControl *control, int index, int subItem);

    void        Notify(TNotifyUI &msg);

private:
    UIButton        *m_closeBtn;
    UIButton        *m_maxBtn;
    UIButton        *m_restoreBtn;
    UIButton        *m_minBtn;
    UIButton        *m_search;
};

#endif //DIRECTUI_LISTDEMO_H
