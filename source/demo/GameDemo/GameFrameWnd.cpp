#include "GameFrameWnd.h"
#include "LoginFrameWnd.h"
#include "UIOption.h"
#include "UITabLayout.h"
#include <UIResourceMgr.h>

void GameFrameWnd::Init() {
    m_closeBtn = dynamic_cast<UIButton*>(m_pm.FindControl("closebtn"));
    m_maxBtn = dynamic_cast<UIButton*>(m_pm.FindControl("maxbtn"));
    m_restoreBtn = dynamic_cast<UIButton*>(m_pm.FindControl("restorebtn"));
    m_minBtn = dynamic_cast<UIButton*>(m_pm.FindControl("minbtn"));
}

void GameFrameWnd::OnPrepare() {
    UIGameList *gameList = dynamic_cast<UIGameList*>(m_pm.FindControl("gamelist"));
    UIGameList::Node *categoryNode = nullptr;
    UIGameList::Node *gameNode = nullptr;
    UIGameList::Node *serverNode = nullptr;
    UIGameList::Node *roomNode = nullptr;
    categoryNode = gameList->AddNode(UIString{u8"{x 4}{i gameicons.png 18 3}{x 4}推荐游戏"});
    for( int i = 0; i < 4; ++i )
    {
        gameNode = gameList->AddNode(UIString{u8"{x 4}{i gameicons.png 18 10}{x 4}四人斗地主"}, categoryNode);
        for( int i = 0; i < 3; ++i )
        {
            serverNode = gameList->AddNode(UIString{u8"{x 4}{i gameicons.png 18 10}{x 4}测试服务器"}, gameNode);
            for( int i = 0; i < 3; ++i )
            {
                roomNode = gameList->AddNode(UIString{u8"{x 4}{i gameicons.png 18 10}{x 4}测试房间"}, serverNode);
            }
        }
    }
    categoryNode = gameList->AddNode(UIString{u8"{x 4}{i gameicons.png 18 3}{x 4}最近玩过的游戏"});
    for( int i = 0; i < 2; ++i )
    {
        gameList->AddNode(UIString{u8"三缺一"}, categoryNode);
    }
    categoryNode = gameList->AddNode(UIString{u8"{x 4}{i gameicons.png 18 3}{x 4}棋牌游戏"});
    for( int i = 0; i < 8; ++i )
    {
        gameList->AddNode(UIString{u8"双扣"}, categoryNode);
    }
    categoryNode = gameList->AddNode(UIString{u8"{x 4}{i gameicons.png 18 3}{x 4}休闲游戏"});
    for( int i = 0; i < 8; ++i )
    {
        gameList->AddNode(UIString{u8"飞行棋"}, categoryNode);
    }

    auto* pUserList = dynamic_cast<UIList*>(m_pm.FindControl("userlist"));
    pUserList->SetTextCallback(this);
    for( int i = 0; i < 400; i++ ) {
        auto* pListElement = new UIListTextElement;
        pUserList->Add(pListElement);
    }
    //gameList->NeedUpdate();
    //pUserList->NeedUpdate();
}

void GameFrameWnd::Notify(TNotifyUI &msg) {
    if( msg.sType == "windowinit" ) OnPrepare();
    else if( msg.sType == "click" ) {
        if( msg.pSender == m_closeBtn ) {
            auto* pControl = dynamic_cast<UIOption*>(m_pm.FindControl("hallswitch"));
            if( pControl && (!pControl->IsSelected()) ) {
                UIControl* pFadeControl = m_pm.FindControl("fadeEffect");
                if( pFadeControl ) pFadeControl->SetVisible(true);
            }
            else {
                UI_APP_QUIT();
                ///*Close()*/PostQuitMessage(0); // 因为activex的原因，使用close可能会出现错误
            }
            return;
        }
        else if( msg.pSender == m_minBtn ) {this->Minimize(); return; }
        else if( msg.pSender == m_maxBtn ) { this->Maximize(); return; }
        else if( msg.pSender == m_restoreBtn ) { this->Restore(); return; }
        UIString name = msg.pSender->GetName();
        if( name == "quitbtn" ) {
            UI_APP_QUIT();
            // /*Close()*/PostQuitMessage(0); // 因为activex的原因，使用close可能会出现错误
        }
        else if( name == "returnhallbtn" ) {
            UIControl* pFadeControl = m_pm.FindControl("fadeEffect");
            if( pFadeControl ) pFadeControl->SetVisible(false);

            auto* pControl = dynamic_cast<UIOption*>(m_pm.FindControl("hallswitch"));
            pControl->Activate();
            pControl = dynamic_cast<UIOption*>(m_pm.FindControl("roomswitch"));
            if( pControl ) pControl->SetVisible(false);
        }
        else if( name == "fontswitch" ) {
            LoginFrameWnd* pLoginFrame = new LoginFrameWnd();
            if( pLoginFrame == NULL ) { Close(); return; }
            //pLoginFrame->Create(this->GetWND(), u8"LoginFrame", UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0);
            pLoginFrame->Create(this->GetWND(),UIString{u8"LoginFrame"},UI_WNDSTYLE_DIALOG,0,0,0,600,400);
            //pLoginFrame->SetIcon(IDI_ICON_DUILIB);
            pLoginFrame->CenterWindow();
            //pLoginFrame->ShowWindow();
            pLoginFrame->ShowModal();

            delete pLoginFrame;
        }
        else if( name == "leaveBtn"  || name == "roomclosebtn" ) {
            auto* pControl = dynamic_cast<UIOption*>(m_pm.FindControl("hallswitch"));
            if( pControl ) {
                pControl->Activate();
                pControl = static_cast<UIOption*>(m_pm.FindControl("roomswitch"));
                if( pControl ) pControl->SetVisible(false);
            }
        }
    }
    else if( msg.sType == "selectchanged" ) {
        UIString name = msg.pSender->GetName();
        if( name == "hallswitch" ) {
            auto* pControl = dynamic_cast<UITabLayout*>(m_pm.FindControl("switch"));
            if( pControl && pControl->GetCurSel() != 0 ) pControl->SelectItem(0);
        }
        else if( name == "roomswitch" ) {
            auto* pControl = dynamic_cast<UITabLayout*>(m_pm.FindControl("switch"));
            if( pControl && pControl->GetCurSel() != 1 ) {
                pControl->SelectItem(1);
                UIDeskList* pDeskList = static_cast<UIDeskList*>(m_pm.FindControl("destlist"));
                pDeskList->SetFocus();
#if 0
                CRichEditUI* pRichEdit = static_cast<CRichEditUI*>(m_pm.FindControl("chatmsglist"));
                if( pRichEdit ) {
                    pRichEdit->SetText(_T("欢迎进入XXX游戏，祝游戏愉快！\n\n"));
                    long lSelBegin = 0, lSelEnd = 0;
                    CHARFORMAT2 cf;
                    ZeroMemory(&cf, sizeof(CHARFORMAT2));
                    cf.cbSize = sizeof(cf);
                    cf.dwReserved = 0;
                    cf.dwMask = CFM_COLOR;
                    cf.crTextColor = RGB(255, 0, 0);
                    lSelEnd = pRichEdit->GetTextLength();
                    pRichEdit->SetSel(lSelBegin, lSelEnd);
                    pRichEdit->SetSelectionCharFormat(cf);
                }
#endif
            }
        }
    }
    else if( msg.sType == "itemclick" ) {
        auto* pGameList = dynamic_cast<UIGameList*>(m_pm.FindControl("gamelist"));
        if( pGameList->GetItemIndex(msg.pSender) != -1 )
        {
            if(msg.pSender->GetClass() == DUI_CTR_LISTLABELELEMENT ) {
                auto* node = (UIGameList::Node*)msg.pSender->GetTag();

                POINT pt = { 0 };
//                ::GetCursorPos(&pt);
//                ::ScreenToClient(m_pm.GetPaintWindow(), &pt);
                pt = m_pm.GetMousePos();
                pt.x -= msg.pSender->GetX();
                SIZE sz = pGameList->GetExpanderSizeX(node);
                if( pt.x >= sz.cx && pt.x < sz.cy )
                    pGameList->ExpandNode(node, !node->data()._expand);
            }
        }
    }
    else if( msg.sType == "itemactivate" ) {
        auto* pGameList = dynamic_cast<UIGameList*>(m_pm.FindControl("gamelist"));
        if( pGameList->GetItemIndex(msg.pSender) != -1 )
        {
            if( msg.pSender->GetClass() == DUI_CTR_LISTLABELELEMENT ) {
                auto* node = (UIGameList::Node*)msg.pSender->GetTag();
                pGameList->ExpandNode(node, !node->data()._expand);
                if( node->data()._level == 3 ) {
                    UIOption* pControl = static_cast<UIOption*>(m_pm.FindControl("roomswitch"));
                    if( pControl ) {
                        pControl->SetVisible(true);
                        pControl->SetText(node->parent()->parent()->data()._text);
                        pControl->Activate();

                    }
                }
            }
        }
    }
    else if( msg.sType == "itemselect" ) {
        if( msg.pSender->GetName() == "chatCombo" ) {
            auto* pChatEdit = dynamic_cast<UIEdit*>(m_pm.FindControl("chatEdit"));
            if( pChatEdit ) pChatEdit->SetText(msg.pSender->GetText());
            dynamic_cast<UICombo*>(msg.pSender)->SelectItem(-1);
        }
    }
}

UIString GameFrameWnd::GetItemText(UIControl *pList, int iItem, int iSubItem) {
    if( pList->GetParent()->GetParent()->GetName() == "userlist" ) {
        if( iSubItem == 0 ) return UIString{u8"<i vip.png>"};
        if( iSubItem == 1 ) return UIString{u8"<i vip.png>"};
        if( iSubItem == 2 ) return UIString{u8"此人昵称"};
        if( iSubItem == 3 ) return UIString{u8"5"};
        if( iSubItem == 4 ) return UIString{u8"50%"};
        if( iSubItem == 5 ) return UIString{u8"0%"};
        if( iSubItem == 6 ) return UIString{u8"100"};
    }

    return UIString{""};
}

long GameFrameWnd::OnCreate(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool &bHandled) {
    UIWindowImpBase::OnCreate(uMsg, wParam, lParam, bHandled);
    m_pm.Init(this->GetWND());
    UIDlgBuilder    dlgBuilder;
    CDialogBuilderCallbackEx    cb;
    UIControl *pRoot = dlgBuilder.Create(UIString{u8"hall.xml"},UIDlgBuilder::SkinType::SkinType_XmlFile,&cb,&m_pm);
    m_pm.AttachDialog(pRoot);
    m_pm.AddNotifier(this);
    Init();
    return 0;
}
