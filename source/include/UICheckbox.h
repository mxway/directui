#ifndef DIRECTUI_UICHECKBOX_H
#define DIRECTUI_UICHECKBOX_H
#include <UIOption.h>

class UICheckbox : public UIOption
{
public:
    UIString GetClass() const override;

    LPVOID GetInterface(const UIString &name) override;

    void    SetCheck(bool check, bool triggerEvent=true);
    bool    GetCheck()const;
};

#endif //DIRECTUI_UICHECKBOX_H