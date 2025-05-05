#include "DisplayInstance.h"

DisplayInstance::DisplayInstance() {
    m_display = XOpenDisplay(nullptr);
}

DisplayInstance::~DisplayInstance() {
    if(m_display != nullptr){
        XCloseDisplay(m_display);
    }
}

DisplayInstance &DisplayInstance::GetInstance() {
    static DisplayInstance displayInstance;
    return displayInstance;
}

Display *DisplayInstance::GetDisplay() const {
    return m_display;
}

int DisplayInstance::GetWidth() const {
    return DisplayWidth(m_display,this->GetScreenNumber());
}

int DisplayInstance::GetHeight() const {
    return DisplayHeight(m_display,this->GetScreenNumber());
}

int DisplayInstance::GetScreenNumber() const {
    return DefaultScreen(m_display);
}

int DisplayInstance::GetDisplayDepth() const {
    return DefaultDepth(m_display,this->GetScreenNumber());
}

Visual *DisplayInstance::GetVisual() const {
    return DefaultVisual(m_display,this->GetScreenNumber());
}

Colormap DisplayInstance::GetColormap() const {
    return DefaultColormap(m_display,this->GetScreenNumber());
}
