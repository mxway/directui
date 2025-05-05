#include "UIBaseWindowObjects.h"

UIBaseWindowObjects::UIBaseWindowObjects() {

}

UIBaseWindowObjects &UIBaseWindowObjects::GetInstance() {
    static UIBaseWindowObjects baseWindowObjects;
    return baseWindowObjects;
}

bool UIBaseWindowObjects::AddObject(Window window, UIBaseWindow *baseWindow) {
    std::lock_guard<std::mutex> lockGuard{m_mutex};
    auto itr = m_windowObjects.find(window);
    if(itr != m_windowObjects.end()){
        return false;
    }
    m_windowObjects.insert(make_pair(window,baseWindow));
    return true;
}

UIBaseWindow *UIBaseWindowObjects::RemoveObject(Window window) {
    std::lock_guard<std::mutex> lockGuard{m_mutex};
    auto itr = m_windowObjects.find(window);
    if(itr == m_windowObjects.end()){
        return nullptr;
    }
    UIBaseWindow *result = itr->second;
    m_windowObjects.erase(itr);
    return result;
}

UIBaseWindow *UIBaseWindowObjects::RemoveObject(UIBaseWindow *baseWindow) {
    std::lock_guard<std::mutex> lockGuard{m_mutex};
    auto itr = m_windowObjects.begin();
    while(itr != m_windowObjects.end()){
        if(itr->second == baseWindow){
            break;
        }
        ++itr;
    }
    if(itr == m_windowObjects.end()){
        return nullptr;
    }
    m_windowObjects.erase(itr);
    return baseWindow;
}

UIBaseWindow *UIBaseWindowObjects::GetObject(Window window) {
    std::lock_guard<std::mutex> lockGuard{m_mutex};
    auto itr = m_windowObjects.find(window);
    if(itr == m_windowObjects.end()){
        return nullptr;
    }
    return itr->second;
}

uint32_t UIBaseWindowObjects::GetWindowCount() {
    std::lock_guard<std::mutex> lockGuard{m_mutex};
    return m_windowObjects.size();
}
