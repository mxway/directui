#include "RichEditFrame.h"

#include "UIDlgBuilder.h"
#include "UIRichEdit.h"

DUI_BEGIN_MESSAGE_MAP(RichEditFrame, UIWindowImpBase)
    DUI_END_MESSAGE_MAP()

RichEditFrame::RichEditFrame() {
}

RichEditFrame::~RichEditFrame() {
}

long RichEditFrame::OnCreate(uint32_t uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) {
    UIWindowImpBase::OnCreate(uMsg, wParam, lParam, bHandled);
    m_pm.Init(this->GetWND());
    UIDlgBuilder    dlgBuilder;
    UIControl *root = dlgBuilder.Create(this->GetSkinFile(),
                                        UIDlgBuilder::SkinType_XmlFile,nullptr,&m_pm);
    m_pm.AttachDialog(root);
    m_pm.AddNotifier(this);
    return 1;
}

void RichEditFrame::Notify(TNotifyUI& msg) {
    if (msg.sType == "windowinit") {
        Init();
    }else if (msg.sType == "click") {
        if (msg.pSender->GetName() == "closebtn") {
            UI_APP_QUIT();
        }
    }
    //UIWindowImpBase::Notify(msg);
}

void RichEditFrame::Init() {
    UIRichEdit *pRichEdit = dynamic_cast<UIRichEdit*>(m_pm.FindControl(u8"richedit_demo"));
    if (!pRichEdit) {
        return;
    }
    FILE *fp = fopen("red_chamber.txt","rb");
    if (!fp) {
        return;
    }
    fseek(fp,0,SEEK_END);
    uint32_t fileSize = ftell(fp);
    fseek(fp,0,SEEK_SET);
    char *buffer = new char[fileSize + 2];
    memset(buffer,0,fileSize + 2);
    fread(buffer,1,fileSize,fp);
    fclose(fp);
    //auto textRun = make_shared<TextRun>();
    //textRun->SetFontFamily(UIString{u8"宋体"});
    //textRun->SetText(UIString{buffer});
    //Paragraph paragraph;
    //paragraph.AppendRun(textRun);
    //pRichEdit->AppendParagraph(paragraph);
    pRichEdit->SetText(UIString{buffer});

    delete [] buffer;
}

long RichEditFrame::OnKeyPress(uint32_t uMsg, uint32_t keyCode, bool& bHandled) {
    if (keyCode == VK_ESCAPE) {
        UI_APP_QUIT();
        bHandled = true;
        return 0;
    }
    return UIWindowImpBase::OnKeyPress(uMsg, keyCode, bHandled);
}
