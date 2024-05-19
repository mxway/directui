#ifndef DIRECTUI_IDIALOGBUILDERCALLBACK_H
#define DIRECTUI_IDIALOGBUILDERCALLBACK_H

class UIControl;

class IDialogBuilderCallback
{
public:
    virtual ~IDialogBuilderCallback()=default;
    virtual UIControl *CreateControl(const char *pstrClass) = 0;
};

#endif //DIRECTUI_IDIALOGBUILDERCALLBACK_H