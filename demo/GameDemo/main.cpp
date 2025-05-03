#include <UIResourceMgr.h>
#include <UIFileHelper.h>
#include "GameFrameWnd.h"

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
                            u8"GameRes";
    UIResourceMgr::GetInstance().SetResourcePath(resourcePath);
    auto *frame = new GameFrameWnd;
    frame->Create(nullptr,UIString{u8"游戏中心"},UI_WNDSTYLE_FRAME,0,0,0, 500,400);
    frame->CenterWindow();
    frame->ShowWindow();
    frame->Maximize();
    UIPaintManager::MessageLoop();
    return 0;
}
