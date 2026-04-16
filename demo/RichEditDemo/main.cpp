#include "RichEditFrame.h"

#include <UIResourceMgr.h>
#include <UIFileHelper.h>
#include "RichEditFrame.h"

//对于windows系统下使用vs编译器，在debug版本下直接使用main函数运行带有控制台的界面程序。
//对于release版本则直接使用WinMain运行时不带控制台信息。
//在windows以及linux系统下使用gcc编译器，所有入口统一使用main
#if defined(_MSC_VER) && defined(NDEBUG)
#pragma comment(linker,"/subsystem:windows")
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
                                u8"RichEditRes";
    UIPaintManager::SetQuitOnLastWindowDestroy(true);
    UIResourceMgr::GetInstance().SetResourcePath(resourcePath);
    RichEditFrame   mainFrame;
    mainFrame.Create(nullptr,UIString{u8"RichEdit Demo App"},UI_WNDSTYLE_FRAME,0,0,0, 1200,800);
    //mainFrame.CenterWindow();
    mainFrame.ShowWindow();
    mainFrame.Maximize();
    UIPaintManager::MessageLoop();
    return 0;
}
