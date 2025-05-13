#include <UIBaseWindow.h>

UIBaseWindow::~UIBaseWindow() {
    m_pm.Init(nullptr);
}
