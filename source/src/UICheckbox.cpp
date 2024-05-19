#include <UICheckbox.h>

UIString UICheckbox::GetClass() const {
    return UIString{DUI_CTR_CHECKBOX};
}

LPVOID UICheckbox::GetInterface(const UIString &name) {
    if(name == DUI_CTR_CHECKBOX){
        return static_cast<UICheckbox*>(this);
    }
    return UIOption::GetInterface(name);
}

void UICheckbox::SetCheck(bool check, bool triggerEvent) {
    Selected(check, triggerEvent);
}

bool UICheckbox::GetCheck() const {
    return IsSelected();
}
