#include <UIScrollBar.h>
#include <windows.h>
#include <UIPaintManager.h>

POINT UIScrollBar::GetCursorPos() const{
    POINT pt = { 0 };
    ::GetCursorPos(&pt);
    ::ScreenToClient(m_manager->GetPaintWindow(), &pt);
    return pt;
}