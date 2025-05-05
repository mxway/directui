#ifndef DIRECTUI_UIBASEWINDOWOBJECTS_H
#define DIRECTUI_UIBASEWINDOWOBJECTS_H
#include <mutex>
#include <map>
#include <UIBaseWindow.h>
#include "UIBackend.h"

class UIBaseWindowObjects {
public:
    UIBaseWindowObjects(const UIBaseWindowObjects &) = delete;
    UIBaseWindowObjects(UIBaseWindowObjects &&)=delete;
    UIBaseWindowObjects &operator=(const UIBaseWindowObjects&)=delete;
    UIBaseWindowObjects &operator=(UIBaseWindowObjects&&) = delete;

    static UIBaseWindowObjects &GetInstance();
    bool                AddObject(Window window, UIBaseWindow *baseWindow);
    UIBaseWindow        *RemoveObject(Window window);
    UIBaseWindow        *RemoveObject(UIBaseWindow *baseWindow);
    UIBaseWindow        *GetObject(Window window);
    uint32_t            GetWindowCount();
private:
    UIBaseWindowObjects();
private:
    std::map<Window,UIBaseWindow*> m_windowObjects;
    std::mutex  m_mutex;
};


#endif //DIRECTUI_UIBASEWINDOWOBJECTS_H
