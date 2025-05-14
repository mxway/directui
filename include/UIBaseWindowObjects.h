#ifndef DIRECTUI_UIBASEWINDOWOBJECTS_H
#define DIRECTUI_UIBASEWINDOWOBJECTS_H
#include <mutex>
#include <map>
#include <UIBaseWindow.h>
#include <UIBackend.h>

class UIBaseWindowObjects {
public:
    UIBaseWindowObjects(const UIBaseWindowObjects &) = delete;
    UIBaseWindowObjects(UIBaseWindowObjects &&)=delete;
    UIBaseWindowObjects &operator=(const UIBaseWindowObjects&)=delete;
    UIBaseWindowObjects &operator=(UIBaseWindowObjects&&) = delete;

    static UIBaseWindowObjects &GetInstance();
    bool                AddObject(WindowEventType window, UIBaseWindow *baseWindow);
    UIBaseWindow        *RemoveObject(WindowEventType window);
    UIBaseWindow        *RemoveObject(UIBaseWindow *baseWindow);
    UIBaseWindow        *GetObject(WindowEventType window);
    uint32_t            GetWindowCount();
private:
    UIBaseWindowObjects();
private:
    std::map<WindowEventType,UIBaseWindow*> m_windowObjects;
    std::mutex  m_mutex;
};


#endif //DIRECTUI_UIBASEWINDOWOBJECTS_H
