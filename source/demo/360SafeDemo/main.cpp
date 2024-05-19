#include <UIResourceMgr.h>
#include <UIFileHelper.h>
#include "MainFrame.h"

int main(int argc, char **argv) {
    UIResourceMgr::GetInstance().Init(argc, argv);
    UIString resourcePath = UIResourceMgr::GetInstance().GetCurrentPath() +
                                UIFileHelper::UI_PATH_SEPARATOR +
                                u8"360SafeRes";
    UIResourceMgr::GetInstance().SetResourcePath(resourcePath);
    MainFrame   mainFrame;
    mainFrame.Create(nullptr,UIString{u8"计算机测试"},0,0, 500,400);
    mainFrame.CenterWindow();
    mainFrame.ShowWindow();
    UIPaintManager::MessageLoop();
    return 0;
}
