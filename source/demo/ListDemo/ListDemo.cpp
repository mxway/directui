#include "ListDemo.h"
#include "UIDlgBuilder.h"
#include <sstream>
#include <vector>
#include <string>

/*
* 存放第二列数据
*/
std::vector<std::string> domain;
/*
* 存放第三列数据
*/
std::vector<std::string> desc;

DUI_BEGIN_MESSAGE_MAP(ListDemo, UIWindowImpBase)

DUI_END_MESSAGE_MAP()

long ListDemo::OnDestroy(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) {
    UI_APP_QUIT();
    return UIBaseWindow::OnDestroy(uMsg, wParam, lParam, bHandled);
}

long ListDemo::OnCreate(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) {
    UIBaseWindow::OnCreate(uMsg, wParam, lParam, bHandled);
    m_pm.Init(this->GetWND());
    UIDlgBuilder    dlgBuilder;
    UIControl *root = dlgBuilder.Create(this->GetSkinFile(),
                                        UIDlgBuilder::SkinType_XmlFile,nullptr,&m_pm);
    m_pm.AttachDialog(root);
    m_pm.AddNotifier(this);
    return 1;
}

ListDemo::ListDemo() {

}

void ListDemo::Init() {
    m_closeBtn = dynamic_cast<UIButton*>(m_pm.FindControl("closebtn"));
    m_maxBtn = dynamic_cast<UIButton*>(m_pm.FindControl("maxbtn"));
    m_restoreBtn = dynamic_cast<UIButton*>(m_pm.FindControl("restorebtn"));
    m_minBtn = dynamic_cast<UIButton*>(m_pm.FindControl("minbtn"));
    m_search = dynamic_cast<UIButton*>(m_pm.FindControl("btn"));
}

void ListDemo::OnSearch() {
    for(int i=0;i<100;i++){
        std::stringstream ss;
        ss<<"www."<<i<<".com";
        domain.push_back(ss.str());
        ss.clear();
        ss<<"it's "<<i;
        desc.push_back(ss.str());
        auto* pListElement = new UIListTextElement;
        pListElement->SetTag((LPVOID)(long)i);
        auto* pList = dynamic_cast<UIList*>(m_pm.FindControl("domainlist"));
        pList->SetTextCallback(this);
        if( pList ) pList->Add(pListElement);
    }
}

UIString ListDemo::GetItemText(UIControl *control, int index, int subItem) {
    char szBuf[512+2] = {0};
    switch(subItem){
        case 0:
            snprintf(szBuf, 512, "%d", index);
            break;
        case 1:
            return UIString{domain[index].c_str()};
        case 2:
            return UIString{desc[index].c_str()};
    }
    return UIString{szBuf};
}

void ListDemo::Notify(TNotifyUI &msg) {
    if(msg.sType == DUI_MSGTYPE_WINDOWINIT)
    {
        Init();
    }else if(msg.sType == DUI_MSGTYPE_CLICK){
        if(msg.pSender == m_closeBtn){
            this->Close();
        }else if(msg.pSender == m_minBtn){
            this->Minimize();
        }else if(msg.pSender == m_maxBtn){
            this->Maximize();
        }else if(msg.pSender == m_restoreBtn){
            this->Restore();
        }else if(msg.pSender == m_search){
            OnSearch();
        }
    }else if(msg.sType == DUI_MSGTYPE_SETFOCUS){

    }else if(msg.sType == DUI_MSGTYPE_ITEMCLICK){

    }else if(msg.sType == DUI_MSGTYPE_ITEMACTIVATE){
        //int index = (int)(msg.pSender->GetTag());
    }
}
