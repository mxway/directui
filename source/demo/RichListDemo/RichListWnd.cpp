#include "RichListWnd.h"
#include "UIDlgBuilder.h"
#include "UIList.h"
#include "UIProgress.h"
#include "UITabLayout.h"

DUI_BEGIN_MESSAGE_MAP(RichListWnd, UIWindowImpBase)
DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, RichListWnd::OnClick)
DUI_ON_MSGTYPE(DUI_MSGTYPE_SELECTCHANGED, RichListWnd::OnSelectChanged)
DUI_ON_MSGTYPE(DUI_MSGTYPE_ITEMSELECT, RichListWnd::OnItemClick)
DUI_END_MESSAGE_MAP()

RichListWnd::RichListWnd() {

}

long RichListWnd::OnDestroy(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) {
    UI_APP_QUIT();
    return UIWindowImpBase::OnDestroy(uMsg, wParam, lParam, bHandled);
}

long RichListWnd::OnCreate(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) {
    UIWindowImpBase::OnCreate(uMsg, wParam, lParam, bHandled);
    m_pm.Init(this->GetWND());
    UIDlgBuilder    dlgBuilder;
    UIControl *root = dlgBuilder.Create(this->GetSkinFile(),
                                        UIDlgBuilder::SkinType_XmlFile,nullptr,&m_pm);
    m_pm.AttachDialog(root);
    m_pm.AddNotifier(this);
    return 1;
}

void RichListWnd::Init() {
    m_pCloseBtn = dynamic_cast<UIButton*>(m_pm.FindControl("closebtn"));
    m_pMaxBtn = dynamic_cast<UIButton*>(m_pm.FindControl("maxbtn"));
    m_pRestoreBtn = dynamic_cast<UIButton*>(m_pm.FindControl("restorebtn"));
    m_pMinBtn = dynamic_cast<UIButton*>(m_pm.FindControl("minbtn"));
}

UIString RichListWnd::GetSkinFile() const {
    return UIString{"duilib.xml"};
}

void RichListWnd::OnClick(TNotifyUI &msg) {
    if(msg.pSender == m_pCloseBtn){
        this->Close();
    }else if(msg.pSender == m_pMinBtn){
        this->Minimize();
    }else if(msg.pSender == m_pMaxBtn){
        this->Maximize();
    }else if(msg.pSender == m_pRestoreBtn){
        this->Restore();
    }else if(msg.pSender->GetName() == "quitbtn"){
        this->Close();
    }else if(msg.pSender->GetName() == "down_ico"){
        UIControl *find_ctrl = m_pm.FindSubControlByName(msg.pSender->GetParent(),"down_name");
        if(find_ctrl){
            ((UILabel*)find_ctrl)->SetText(UIString{u8"由程序动态设置的名称..."});
        }
    }else if(msg.pSender->GetName() == "down_del"){
        auto *down_list = dynamic_cast<UIList*>(m_pm.FindControl("down_list_tab"));
        if(!down_list){
            return;
        }
        down_list->RemoveAt(down_list->GetCurSel());
    }else if(msg.pSender->GetName() == "down_new"){
        auto *down_list = dynamic_cast<UIList*>(m_pm.FindControl("down_list_tab"));
        if(!down_list)
            return;

        auto *new_node = new UIListContainerElement;
        new_node->SetAttributeList("height=\"45\"");

        auto *new_h_lay = new UIHorizontalLayout;
        new_h_lay->SetAttributeList("float=\"false\" "\
			"childpadding=\"10\" inset=\"3,5,3,5\"");

        auto *new_btn_1 = new UIButton;
        new_btn_1->SetAttributeList(
                "name=\"down_ico\" float=\"false\" "\
			"bordersize=\"0\" width=\"32\" maxheight=\"26\" "\
			"bkimage=\"downlist_app.png\" "\
			"normalimage=\"file='downlist_run.png' dest='20,14,32,26'\"");

        auto *new_v_lay = new UIVerticalLayout;
        new_h_lay->Add(new_btn_1);
        new_h_lay->Add(new_v_lay);

        auto *new_label = new UILabel;
        new_label->SetAttributeList("textcolor=\"#FFAAAAAA\" showhtml=\"true\"");
        new_label->SetText(UIString{"new added item.exe"});
        new_label->SetName(UIString{"down_name"});
        auto *new_progress = new UIProgress;
        new_progress->SetMinValue(0);
        new_progress->SetMaxValue(100);
        new_progress->SetValue(1);
        new_progress->SetMaxWidth(200);
        new_progress->SetMaxHeight(7);
        new_progress->SetForeImage(UIString{"progress_fore.png"});
        new_progress->SetName(UIString{"down_progress"});
        new_v_lay->Add(new_label);
        new_v_lay->Add(new_progress);

        auto *new_label2 = new UILabel;
        auto *new_label3 = new UILabel;
        auto *new_v_lay2 = new UIVerticalLayout;
        new_h_lay->Add(new_v_lay2);
        new_v_lay2->Add(new_label2);
        new_v_lay2->Add(new_label3);
        new_label2->SetAttributeList(
                "align=\"right\" text=\"\" textcolor=\"#FFAAAAAA\" showhtml=\"true\"");
        new_label3->SetAttributeList(
                "align=\"right\" text=\"0.00K/34.33M \" textcolor=\"#FFAAAAAA\" showhtml=\"true\"");

        new_node->Add(new_h_lay);
        down_list->Add(new_node);
    }
}

void RichListWnd::OnSelectChanged(TNotifyUI &msg) {
    if(msg.pSender->GetName() == "down_list")
    {
        dynamic_cast<UITabLayout*>(m_pm.FindControl("tab_main"))->SelectItem(0);
    }
    else if(msg.pSender->GetName() == "down_his")
    {
        dynamic_cast<UITabLayout*>(m_pm.FindControl("tab_main"))->SelectItem(1);
    }
}

void RichListWnd::OnItemClick(TNotifyUI &msg) {

}

void RichListWnd::Notify(TNotifyUI &msg) {
    if(msg.sType == DUI_MSGTYPE_WINDOWINIT)
    {
        Init();
    }
    UIWindowImpBase::Notify(msg);
}
