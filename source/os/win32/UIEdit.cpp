#include <UIEdit.h>
#include <UIPaintManager.h>
#include "EncodingTransform.h"
#include "UIResourceMgr.h"
#include "UIRect.h"
#include <vector>
#include <iostream>

using namespace std;

struct UIEditInternal
{
    explicit        UIEditInternal(UIEdit *uiEdit);
    ~UIEditInternal();
    void            DoEvent(TEventUI &event);
    void            CreateImmContext();
    void            ReleaseImmContext();
    uint32_t        GetCharNumber()const{
        return m_text.length();
    }
private:
    void            ShowCaretAndSetImmPosition();
    uint32_t        CalculateTextOffset();
    void            CalculateForCharactersWidth();
    //从鼠标点时的坐标计算出当前光标应该插入到哪个字符后进行显示。
    void            CalculateCurrentEditPositionFromMousePoint(POINT pt);
    //初始化光标显示位置以及输入法跟随的位置。
    void            InitCaretAndImmPosition();
    //从当前编辑点处删除一个字符
    void            DeleteCharAtEditPosition();
    void            InsertCharAtEditPosition(wchar_t character);
private:
    std::wstring    m_text;
    UIPtrArray      m_textWidthList;
    UIEdit          *m_uiEdit;
    int             m_currentEditPos;
    HIMC            m_hIMC;

    void CalculateForPasswordCharactersWidth();

    void CalculateForNormalChartersWidth();
};

UIEditInternal::UIEditInternal(UIEdit *uiEdit)
        : m_uiEdit {uiEdit},
          m_currentEditPos {0},
          m_hIMC{nullptr}
{

}

UIEditInternal::~UIEditInternal() {
    if(m_hIMC!=nullptr){
        ::ImmReleaseContext(m_uiEdit->GetManager()->GetPaintWindow(),
                            m_hIMC);
    }
}

void UIEditInternal::DoEvent(TEventUI &event) {
    if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK || event.Type == UIEVENT_RBUTTONDOWN)
    {
        this->CalculateForCharactersWidth();
        this->CalculateCurrentEditPositionFromMousePoint(event.ptMouse);
        this->InitCaretAndImmPosition();
    }
    if(event.Type == UIEVENT_KEYDOWN){
        if(event.wParam == VK_LEFT){
            if(m_currentEditPos != 0){
                m_currentEditPos--;
                this->ShowCaretAndSetImmPosition();
            }
        }
        if(event.wParam == VK_RIGHT){
            if(m_currentEditPos >= m_text.length()){
                m_currentEditPos = m_text.length();
            }else{
                m_currentEditPos += 1;
            }
            this->ShowCaretAndSetImmPosition();
        }
        if(event.wParam == VK_HOME){
            //响应Home键,光标定位到编辑框的开始处
            m_currentEditPos = 0;
            this->ShowCaretAndSetImmPosition();
        }
        if(event.wParam == VK_END){
            //响应End键，光标定位到编辑框文本结尾处
            m_currentEditPos = (int)m_text.length();
            this->ShowCaretAndSetImmPosition();
        }
        if(event.wParam == VK_DELETE){
            if(m_currentEditPos>=m_textWidthList.GetSize()){
                m_currentEditPos = m_textWidthList.GetSize();
            }else{
                m_currentEditPos++;
            }
            this->DeleteCharAtEditPosition();
        }
    }
    if(event.Type == UIEVENT_CHAR)
    {
        if(event.wParam == VK_BACK){
            this->DeleteCharAtEditPosition();
            return;
        }
        this->InsertCharAtEditPosition((wchar_t)event.wParam);
    }
}

uint32_t UIEditInternal::CalculateTextOffset() {
    uint32_t result = 0;
    for(int i=0;i<=m_currentEditPos && i<m_textWidthList.GetSize();i++){
        result += (int)m_textWidthList.GetAt(i);
    }
    return result;
}

void UIEditInternal::CalculateForCharactersWidth() {
    if(m_uiEdit->IsPasswordMode()){
        CalculateForPasswordCharactersWidth();
        return;
    }
    CalculateForNormalChartersWidth();
}

void UIEditInternal::CalculateForNormalChartersWidth() {
    wchar_t *wideText = Utf8ToUcs2(m_uiEdit->GetText().GetData());
    m_text = wstring{wideText};
    delete wideText;
    UIFont *fontHandle = UIResourceMgr::GetInstance().GetFont(m_uiEdit->GetFont());
    auto hOldFont = (HFONT) SelectObject(m_uiEdit->GetManager()->GetPaintDC(),
                                         fontHandle->GetHandle());
    SIZE szSpace = {0};
    m_textWidthList.Empty();
    m_textWidthList.Add((LPVOID)0);
    for(int i=0; i < m_text.length(); i++){
        GetTextExtentPoint32W(m_uiEdit->GetManager()->GetPaintDC(),
                              m_text.c_str() + i, 1, &szSpace);
        m_textWidthList.Add((LPVOID)szSpace.cx);
    }
    SelectObject(m_uiEdit->GetManager()->GetPaintDC(),
                 hOldFont);
}

void UIEditInternal::CalculateForPasswordCharactersWidth() {
    SIZE szSpace = {0};
    UIFont *fontHandle = UIResourceMgr::GetInstance().GetFont(m_uiEdit->GetFont());
    auto hOldFont = (HFONT) SelectObject(m_uiEdit->GetManager()->GetPaintDC(),
                                         fontHandle->GetHandle());
    char passwordChar = m_uiEdit->GetPasswordChar();
    GetTextExtentPoint32A(m_uiEdit->GetManager()->GetPaintDC(),
                          &passwordChar, 1, &szSpace);
    SelectObject(m_uiEdit->GetManager()->GetPaintDC(),
                 hOldFont);
    m_textWidthList.Empty();
    m_textWidthList.Add((LPVOID)0);
    for(int i=0; i < m_text.length(); i++){
        m_textWidthList.Add((LPVOID)szSpace.cx);
    }
}

void UIEditInternal::CalculateCurrentEditPositionFromMousePoint(POINT pt) {
    int    width = 9999;
    m_currentEditPos = -1;
    int totalLength = 0;
    for(int i=0; i < m_textWidthList.GetSize(); i++){
        totalLength += (int)m_textWidthList.GetAt(i);
        int textPos = m_uiEdit->GetPos().left + m_uiEdit->GetTextPadding().left + totalLength;
        if(abs(pt.x - textPos) < width){
            m_currentEditPos = i;
            width = abs(pt.x - textPos);
        }
    }
}

void UIEditInternal::DeleteCharAtEditPosition() {
    if(m_currentEditPos == 0){
        return;
    }
    m_text.erase(m_currentEditPos-1,1);
    m_textWidthList.Remove(m_currentEditPos);
    m_currentEditPos--;
    const char *utf8Text = Ucs2ToUtf8(m_text.c_str());
    m_uiEdit->SetText(UIString{utf8Text});
    this->ShowCaretAndSetImmPosition();
    delete []utf8Text;
}

void UIEditInternal::InsertCharAtEditPosition(wchar_t character) {
    wchar_t str[2] = {character, 0};
    m_text.insert(m_currentEditPos, str);
    m_currentEditPos++;
    char *utf8Text = Ucs2ToUtf8(m_text.c_str());
    m_uiEdit->SetText(UIString{utf8Text});
    delete []utf8Text;
    UIFont *fontHandle = UIResourceMgr::GetInstance().GetFont(m_uiEdit->GetFont());
    auto hOldFont = (HFONT)::SelectObject(m_uiEdit->GetManager()->GetPaintDC(),
                                          fontHandle->GetHandle());
    SIZE szSpace = {0};
    if(m_uiEdit->IsPasswordMode()){
        //文本框显示密码字符，这时需要计算密码字符的宽度
        char passwordChar = m_uiEdit->GetPasswordChar();
        ::GetTextExtentPoint32A(m_uiEdit->GetManager()->GetPaintDC(),
                                &passwordChar,1, &szSpace);
    }else{
        ::GetTextExtentPoint32W(m_uiEdit->GetManager()->GetPaintDC(),
                                str,1, &szSpace);
    }
    ::SelectObject(m_uiEdit->GetManager()->GetPaintDC(),hOldFont);
    m_textWidthList.InsertAt(m_currentEditPos, (LPVOID)szSpace.cx);
    this->ShowCaretAndSetImmPosition();
}

void UIEditInternal::InitCaretAndImmPosition() {
    RECT rcItem{m_uiEdit->GetPos()};
    RECT textPadding{m_uiEdit->GetTextPadding()};
    int caretHeight = rcItem.bottom - rcItem.top - textPadding.top - textPadding.bottom - 4;
    ::CreateCaret(m_uiEdit->GetManager()->GetPaintWindow(),nullptr, 1, caretHeight);
    ::SetCaretPos(rcItem.left + textPadding.left + this->CalculateTextOffset(),
                  rcItem.top + textPadding.top + 2);
    COMPOSITIONFORM composition;
    composition.dwStyle = CFS_POINT;
    composition.ptCurrentPos.x = rcItem.left + textPadding.left + this->CalculateTextOffset();
    composition.ptCurrentPos.y = rcItem.bottom;
    ::ImmSetCompositionWindow(m_hIMC, &composition);
    ::ShowCaret(m_uiEdit->GetManager()->GetPaintWindow());
}

void UIEditInternal::CreateImmContext() {
    m_hIMC = ::ImmGetContext(m_uiEdit->GetManager()->GetPaintWindow());
}

void UIEditInternal::ReleaseImmContext() {
    ::ImmReleaseContext(m_uiEdit->GetManager()->GetPaintWindow(), m_hIMC);
    m_hIMC = nullptr;
}

void    UIEdit::InitInternal() {
    m_internalImpl = make_shared<UIEditInternal>(this);
}

uint32_t UIEdit::GetCharNumber() const {
    return m_internalImpl->GetCharNumber();
}

void UIEdit::DoEvent(TEventUI &event) {
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_parent != nullptr ) m_parent->DoEvent(event);
        else UILabel::DoEvent(event);
        return;
    }

    if( event.Type == UIEVENT_SETCURSOR && IsEnabled() )
    {
        UILoadCursor(m_manager, UI_IDC_TEXT);
        return;
    }
    if( event.Type == UIEVENT_SETFOCUS && IsEnabled() )
    {
        m_internalImpl->CreateImmContext();
        Invalidate();
    }
    if( event.Type == UIEVENT_KILLFOCUS && IsEnabled() )
    {
        m_internalImpl->ReleaseImmContext();
        ::HideCaret(m_manager->GetPaintWindow());
        Invalidate();
    }
    if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK || event.Type == UIEVENT_RBUTTONDOWN)
    {
        m_internalImpl->DoEvent(event);
        if( IsEnabled() ) {
            GetManager()->ReleaseCapture();
        }
        return;
    }
    if( event.Type == UIEVENT_MOUSEMOVE )
    {
        return;
    }
    if( event.Type == UIEVENT_BUTTONUP )
    {
        return;
    }
    if( event.Type == UIEVENT_CONTEXTMENU )
    {
        return;
    }
    if( event.Type == UIEVENT_MOUSEENTER )
    {
        UIRect rcItem {m_rcItem};
        if( rcItem.IsPtIn(event.ptMouse ) ) {
            if( IsEnabled() ) {
                if( (m_buttonState & UISTATE_HOT) == 0  ) {
                    m_buttonState |= UISTATE_HOT;
                    Invalidate();
                }
            }
        }
    }
    if( event.Type == UIEVENT_MOUSELEAVE )
    {
        UIRect rcItem{m_rcItem};
        if( rcItem.IsPtIn(event.ptMouse ) ) {
            if( IsEnabled() ) {
                if( (m_buttonState & UISTATE_HOT) != 0  ) {
                    m_buttonState &= ~UISTATE_HOT;
                    Invalidate();
                }
            }
            if (m_manager) m_manager->RemoveMouseLeaveNeeded(this);
        }
        else {
            if (m_manager) m_manager->AddMouseLeaveNeeded(this);
            return;
        }
    }
    if(event.Type == UIEVENT_CHAR || event.Type == UIEVENT_KEYDOWN){
        m_internalImpl->DoEvent(event);
        return;
    }
    UILabel::DoEvent(event);
}

void UIEditInternal::ShowCaretAndSetImmPosition() {
    RECT rcItem{m_uiEdit->GetPos()};
    RECT textPadding{m_uiEdit->GetTextPadding()};
    ::HideCaret(m_uiEdit->GetManager()->GetPaintWindow());
    ::SetCaretPos(rcItem.left + textPadding.left + this->CalculateTextOffset(),
                  rcItem.top + textPadding.top + 2);
    ::ShowCaret(m_uiEdit->GetManager()->GetPaintWindow());
    COMPOSITIONFORM composition;
    composition.dwStyle = CFS_POINT;
    composition.ptCurrentPos.x = rcItem.left + textPadding.left + this->CalculateTextOffset();
    composition.ptCurrentPos.y = rcItem.bottom;
    ::ImmSetCompositionWindow(m_hIMC, &composition);
}

void UIEdit::DrawCaret() {
    
}
