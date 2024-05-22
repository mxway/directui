#include <UIResourceMgr.h>
#include <UIFileHelper.h>
#include "ListDemo.h"

#ifdef _MSC_VER
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR cmdLine, int show) {
    int argc = __argc;
    char** argv = __argv;
#else
int main(int argc, char** argv) {
#endif
    UIResourceMgr::GetInstance().Init(argc, argv);
    UIString resourcePath = UIResourceMgr::GetInstance().GetCurrentPath() +
                            UIFileHelper::UI_PATH_SEPARATOR +
                            u8"ListRes";
    UIResourceMgr::GetInstance().SetResourcePath(resourcePath);
    ListDemo   mainFrame;
    mainFrame.Create(nullptr,UIString{u8"计算机测试1"},0,0, 500,400);
    mainFrame.CenterWindow();
    mainFrame.ShowWindow();
    UIPaintManager::MessageLoop();
    return 0;
}
