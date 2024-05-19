#include "MainFrame.h"
#include "UITabLayout.h"
#include <UIDlgBuilder.h>
#include "UIComputerExamine.h"
#include <iostream>

using namespace std;

DUI_BEGIN_MESSAGE_MAP(MainFrame, UIWindowImpBase)
DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK,MainFrame::OnClick)
DUI_ON_MSGTYPE(DUI_MSGTYPE_SELECTCHANGED,MainFrame::OnItemSelected)
DUI_END_MESSAGE_MAP()

long MainFrame::OnDestroy(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) {
    UI_APP_QUIT();
    return UIBaseWindow::OnDestroy(uMsg, wParam, lParam, bHandled);
}

void MainFrame::OnClick(TNotifyUI &msg) {
    if(msg.pSender->GetName() == "closebtn"){
        this->Close();
    }else if(msg.pSender->GetName()=="minbtn"){
        //this->Minimize();
    }else if(msg.pSender->GetName()=="maxbtn"){
        this->Maximize();
    }
    //this->Close();
}

long MainFrame::OnCreate(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) {
    UIBaseWindow::OnCreate(uMsg, wParam, lParam, bHandled);
    m_pm.Init(this->GetWND());
    UIDlgBuilder    dlgBuilder;
    UIControl *root = dlgBuilder.Create(this->GetSkinFile(),
                                        UIDlgBuilder::SkinType_XmlFile,this,&m_pm);
    m_pm.AttachDialog(root);
    m_pm.AddNotifier(this);
    return 1;
}

void MainFrame::OnItemSelected(TNotifyUI &msg) {
    UIString name = msg.pSender->GetName();
    auto *tab = dynamic_cast<UITabLayout*>(m_pm.FindControl("switch"));
    if(name=="examine")
        tab->SelectItem(0);
    else if(name=="trojan")
        tab->SelectItem(1);
    else if(name=="plugins")
        tab->SelectItem(2);
    else if(name=="vulnerability")
        tab->SelectItem(3);
    else if(name=="rubbish")
        tab->SelectItem(4);
    else if(name=="cleanup")
        tab->SelectItem(5);
    else if(name=="fix")
        tab->SelectItem(6);
    else if(name=="tool")
        tab->SelectItem(7);
}

UIControl *MainFrame::CreateControl(const char *pstrClass) {
    if(strcasecmp("ComputerExamine", pstrClass)==0){
        return new UIComputerExamine;
    }
    return nullptr;
}

//UIString MainFrame::GetSkinFile() const {
//    return UIString{u8"duilib.xml"};
//}
