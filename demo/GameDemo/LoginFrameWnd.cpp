#include "LoginFrameWnd.h"
#include <UIDlgBuilder.h>

void LoginFrameWnd::Init() {
    auto* pAccountCombo = dynamic_cast<UICombo*>(m_pm.FindControl("accountcombo"));
    auto* pAccountEdit = dynamic_cast<UIEdit*>(m_pm.FindControl("accountedit"));
    if( pAccountCombo && pAccountEdit ) pAccountEdit->SetText(pAccountCombo->GetText());
    pAccountEdit->SetFocus();
}

void LoginFrameWnd::Notify(TNotifyUI &msg) {
    if( msg.sType == "click" ) {
        if( msg.pSender->GetName() == "closebtn" ) { UI_APP_QUIT(); return; }
        else if( msg.pSender->GetName() == "loginBtn" ) { Close(); return; }
    }
    else if( msg.sType == "itemselect" ) {
        if( msg.pSender->GetName() == "accountcombo" ) {
            auto* pAccountEdit = dynamic_cast<UIEdit*>(m_pm.FindControl("accountedit"));
            if( pAccountEdit ) pAccountEdit->SetText(msg.pSender->GetText());
        }
    }
}

long LoginFrameWnd::OnCreate(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) {
    UIWindowImpBase::OnCreate(uMsg, wParam, lParam, bHandled);
    m_pm.Init(this->GetWND());

    UIDlgBuilder dlgBuilder;
    UIControl *pRoot = dlgBuilder.Create(UIString{"login.xml"},
                                         UIDlgBuilder::SkinType::SkinType_XmlFile,
                                         nullptr,&m_pm);
    m_pm.AttachDialog(pRoot);
    m_pm.AddNotifier(this);
    Init();
    return 0;
}
