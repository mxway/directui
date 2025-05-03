#include <NotifyPump.h>
#include <UIControl.h>

DUI_BASE_BEGIN_MESSAGE_MAP(UINotifyPump)
DUI_END_MESSAGE_MAP()

static const DUI_MSGMAP_ENTRY* DuiFindMessageEntry(const DUI_MSGMAP_ENTRY* lpEntry,TNotifyUI& msg )
{
    UIString sMsgType = msg.sType;
    UIString sCtrlName = msg.pSender->GetName();
    const DUI_MSGMAP_ENTRY* pMsgTypeEntry = nullptr;
    while (lpEntry->nSig != DuiSig_end)
    {
        if(lpEntry->sMsgType==sMsgType)
        {
            if(!lpEntry->sCtrlName.IsEmpty())
            {
                if(lpEntry->sCtrlName==sCtrlName)
                {
                    return lpEntry;
                }
            }
            else
            {
                pMsgTypeEntry = lpEntry;
            }
        }
        lpEntry++;
    }
    return pMsgTypeEntry;
}

bool UINotifyPump::AddVirtualWnd(UIString strName, UINotifyPump *pObject) {
    if( m_VirtualWndMap.Find(strName) == nullptr )
    {
        m_VirtualWndMap.Insert(strName,(LPVOID)pObject);
        return true;
    }
    return false;
}

bool UINotifyPump::RemoveVirtualWnd(UIString strName) {
    if( m_VirtualWndMap.Find(strName) != nullptr )
    {
        m_VirtualWndMap.Remove(strName);
        return true;
    }
    return false;
}

void UINotifyPump::NotifyPump(TNotifyUI &msg) {
    /*if(msg.sVirtualWnd.IsEmpty()){
        return;
    }
    for(int i=0;i<m_VirtualWndMap.GetSize();i++){
        UIString key = m_VirtualWndMap.GetAt(i);
        if(key.IsEmpty()){
            continue;
        }
        if(key==msg.sVirtualWnd){
            auto *pObject = static_cast<UINotifyPump*>(m_VirtualWndMap.Find(key,false));
            if(pObject && pObject->LoopDispatch(msg)){}
            return;
        }
    }*/
    LoopDispatch(msg);
}

bool UINotifyPump::LoopDispatch(TNotifyUI &msg) {
    const DUI_MSGMAP_ENTRY* lpEntry = nullptr;
    const DUI_MSGMAP*       pMessageMap = nullptr;
    for(pMessageMap = GetMessageMap(); pMessageMap != nullptr; pMessageMap=(*pMessageMap->pfnGetBaseMap)()){
        if((lpEntry= DuiFindMessageEntry(pMessageMap->lpEntries,msg))!=nullptr)
        {
            goto LDispatch;
        }
    }
    return false;
LDispatch:
    union UIMessageMapFunctions mmf{};
    mmf.pfn = lpEntry->pfn;
    bool bRet = false;
    uint32_t nSig = lpEntry->nSig;
    switch(nSig){
        case DuiSig_lwl:
            (this->*mmf.pfn_Notify_lwl)(msg.wParam,msg.lParam);
            bRet = true;
            break;
        case DuiSig_vn:
            (this->*mmf.pfn_Notify_vn)(msg);
            bRet = true;
            break;
        default:
            break;
    }
    return bRet;
}
