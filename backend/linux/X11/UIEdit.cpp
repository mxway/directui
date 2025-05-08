#include "UIEdit.h"
#include "UIPaintManager.h"
#include "UIRect.h"
#include "UIResourceMgr.h"
#include <string>
#include <pango/pango-layout.h>
#include <pango/pangoxft.h>
#include "X11HDC.h"

using namespace std;

static uint32_t GetNumberOfCharacters(const UIString &str)
{
    const char *start = str.GetData();
    const char *end = str.GetData() + str.GetLength();
    uint32_t result = 0;
    while(start < end){
        start = CharNext(start);
        result++;
    }
    return result;
}

struct ImmContext {
    XIC m_xic;
    XIM m_xim;
};

void set_preedit_position(XIC ic, int x, int y) {
    XPoint point = {static_cast<short>(x),static_cast<short>(y)};
    XVaNestedList preedit_attr = XVaCreateNestedList(0,
                                                     XNSpotLocation, &point,
                                                     NULL);
    if (preedit_attr) {
        XSetICValues(ic, XNPreeditAttributes, preedit_attr, NULL);
        XFree(preedit_attr);
    }
}

struct UIEditInternal
{
    explicit UIEditInternal(UIEdit *uiEdit);
    ~UIEditInternal();
    void    DoEvent(TEventUI &event);
    void    CreateImmContext(int initX, int initY);
    void    ReleaseImmContext();
    void    OnImmCommit(const char *str);
    void    CalculateCharactersWidth();
    void    CalculateCurrentEditPositionFromMousePoint(POINT ptMouse);
    void    DrawCaret(HANDLE_DC hDC);
    void    OnKeyDown(TEventUI &event);
    //
    int     GetCharactersBytes(int numberOfChars);

    void   InsertNewCharactersWidth(const char *insertString);

private:
    struct ImmContext   *m_imContext;
    UIEdit          *m_uiEdit;
    string          m_text;
    UIPtrArray      m_textWidthList;
    int             m_currentEditPos;
    Pixmap          m_cursor;
    uint32_t        m_cursorHeight;

private:
    void CalculatePasswordCharactersWidth();

    void CalculateNormalCharactersWidth();

    void UpdateInsertedPasswordCharacterWidth(const char *insertString, PangoLayout *layout);

    void UpdateInsertedNormalTextWidth(const char *insertString, PangoLayout *layout);
};

UIEditInternal::UIEditInternal(UIEdit *uiEdit)
        :m_imContext{nullptr},
         m_uiEdit{uiEdit},
         m_text{uiEdit->GetText().GetData()},
         m_currentEditPos{-1},
         m_cursor{0},
         m_cursorHeight{0}
{

}

UIEditInternal::~UIEditInternal() {
    this->ReleaseImmContext();
    if (m_cursor != 0) {
        XFreePixmap(m_uiEdit->GetManager()->GetPaintWindow()->display,m_cursor);
    }
}

void UIEditInternal::DoEvent(TEventUI &event) {
    if(event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK || event.Type == UIEVENT_RBUTTONDOWN)
    {
        m_text = m_uiEdit->GetText().GetData();
        this->CreateImmContext(event.ptMouse.x, event.ptMouse.y);
        this->CalculateCharactersWidth();
        this->CalculateCurrentEditPositionFromMousePoint(event.ptMouse);
    }
}

void UIEditInternal::CreateImmContext(int initX, int initY) {
    if (m_imContext == nullptr) {
        m_imContext = new struct ImmContext;
        m_imContext->m_xim = nullptr;
        m_imContext->m_xic = nullptr;
        XSetLocaleModifiers("");
        m_imContext->m_xim = XOpenIM(m_uiEdit->GetManager()->GetPaintWindow()->display,0,0,0);
        m_imContext->m_xic = XCreateIC(m_imContext->m_xim,
            XNInputStyle,XIMPreeditNothing|XIMStatusNothing,
            XNClientWindow,m_uiEdit->GetManager()->GetPaintWindow()->window,
            XNFocusWindow,m_uiEdit->GetManager()->GetPaintWindow()->window,nullptr);
    }
    if (m_imContext->m_xic!=nullptr) {
        XSetICFocus(m_imContext->m_xic);
        set_preedit_position(m_imContext->m_xic,initX,initY);
    }
}

void UIEditInternal::ReleaseImmContext() {
    if (m_imContext != nullptr) {
        if (m_imContext->m_xic != nullptr) {
            //XUnsetICFocus(m_imContext->m_xic);
            XDestroyIC(m_imContext->m_xic);
        }
        if (m_imContext->m_xim != nullptr) {
            XCloseIM(m_imContext->m_xim);
        }
        delete m_imContext;
        m_imContext = nullptr;
    }
    m_currentEditPos = -1;
    m_uiEdit->Invalidate();
}

void UIEditInternal::OnImmCommit(const char *str) {
    uint32_t totalBytes = this->GetCharactersBytes(m_currentEditPos);
    m_text.insert(totalBytes, str);
    this->InsertNewCharactersWidth(str);
    m_uiEdit->SetText(UIString{m_text.c_str()});
}

void UIEditInternal::CalculateCharactersWidth() {
    if(m_uiEdit->IsPasswordMode()){
        CalculatePasswordCharactersWidth();
        return;
    }
    CalculateNormalCharactersWidth();
}

void UIEditInternal::CalculateNormalCharactersWidth() {
    HANDLE_DC hDC = m_uiEdit->GetManager()->GetPaintDC();
    PangoFontMap *font_map = pango_xft_get_font_map(hDC->x11Window->display,hDC->x11Window->screen);
    PangoContext *context = pango_font_map_create_context(font_map);

    // 创建 Pango 布局对象
    PangoLayout *layout = pango_layout_new(context);

    UIFont *font = UIResourceMgr::GetInstance().GetFont(m_uiEdit->GetFont());
    pango_layout_set_font_description(layout, font->GetHandle());
    m_textWidthList.Empty();
    const char *start = m_text.c_str();
    const char *end = start + m_text.length();
    const char *nextChar = CharNext(start);
    m_textWidthList.Add((LPVOID)0);
    while(start != end){
        pango_layout_set_text(layout, start, nextChar-start);
        int width = 0;
        int height = 0;
        pango_layout_get_pixel_size(layout, &width, &height);
        m_textWidthList.Add((LPVOID)(long)width);
        start = nextChar;
        nextChar = CharNext(nextChar);
    }
    g_object_unref(layout);
    g_object_unref(context);
}

void UIEditInternal::CalculatePasswordCharactersWidth() {
    HANDLE_DC hDC = m_uiEdit->GetManager()->GetPaintDC();
    PangoFontMap *font_map = pango_xft_get_font_map(hDC->x11Window->display,hDC->x11Window->screen);
    PangoContext *context = pango_font_map_create_context(font_map);

    // 创建 Pango 布局对象
    PangoLayout *layout = pango_layout_new(context);

    UIFont *font = UIResourceMgr::GetInstance().GetFont(m_uiEdit->GetFont());
    pango_layout_set_font_description(layout, font->GetHandle());
    m_textWidthList.Empty();
    m_textWidthList.Add((LPVOID)0);
    char passwordCharacter = m_uiEdit->GetPasswordChar();
    pango_layout_set_text(layout, &passwordCharacter,1);
    int width = 0;
    int height = 0;
    pango_layout_get_pixel_size(layout, &width, &height);
    uint32_t numberOfCharacters = m_uiEdit->GetCharNumber();
    while(numberOfCharacters--){
        m_textWidthList.Add((LPVOID)(long)width);
    }
    g_object_unref(layout);
    g_object_unref(context);
}

void UIEditInternal::CalculateCurrentEditPositionFromMousePoint(POINT pt) {
    int    width = 9999;
    m_currentEditPos = -1;
    int totalLength = 0;
    for(int i=0; i < m_textWidthList.GetSize(); i++){
        totalLength += (int)(long)m_textWidthList.GetAt(i);
        int textPos = m_uiEdit->GetPos().left + m_uiEdit->GetTextPadding().left + totalLength;
        if(abs(pt.x - textPos) < width){
            m_currentEditPos = i;
            width = abs(pt.x - textPos);
        }
    }
}

const int CURSOR_WIDTH = 6;

void UIEditInternal::DrawCaret(HANDLE_DC hDC) {
    if(m_currentEditPos < 0){
        return;
    }
    RECT rcItem{m_uiEdit->GetPos()};
    XRectangle rect = {0};
    int totalWidth = 0;
    for(int i=0;i<=m_currentEditPos && i<m_textWidthList.GetSize();i++){
        totalWidth += (int)(long)m_textWidthList.GetAt(i);
    }
    RECT rcTextPadding = m_uiEdit->GetTextPadding();
    rect.x = totalWidth + rcItem.left + rcTextPadding.left;
    rect.y = rcItem.top + rcTextPadding.top + 2;
    rect.width = 1;
    rect.height = rcItem.bottom - rcItem.top;
    if (rect.height <8) {
        return;
    }

    X11Window *window = m_uiEdit->GetManager()->GetPaintWindow();
    if (m_cursor == 0) {
        m_cursorHeight = rect.height - 8;
        unsigned char *byteData = new unsigned char[m_cursorHeight];
        byteData[0] = 0x3f;
        byteData[1] = 0x3f;
        byteData[m_cursorHeight-1] = 0x3f;
        byteData[m_cursorHeight-2] = 0x3f;
        memset(byteData+2,0x0c,m_cursorHeight-4);
        m_cursor = XCreatePixmapFromBitmapData( window->display , window->window ,
                reinterpret_cast<char *>(byteData), CURSOR_WIDTH , m_cursorHeight ,
                0xff000000 , WhitePixel ( window->display , window->screen ) ,
                window->depth ) ;
        delete []byteData;
    }
    XCopyArea(window->display,m_cursor,hDC->drawablePixmap,hDC->gc,0,0,CURSOR_WIDTH,m_cursorHeight,rect.x,rect.y+2);
}

static bool IsPrintableChar(KeySym keysym)
{
    return (keysym>=0x20 && keysym<127) || (keysym>=XK_KP_Multiply && keysym<=XK_KP_9);
}

void UIEditInternal::OnKeyDown(TEventUI &event) {
    Status status;
    KeySym keysym = NoSymbol;
    char    text[128] = {0};
    XKeyEvent *keyEvent = static_cast<XKeyEvent*>(event.wParam);
    Xutf8LookupString(m_imContext->m_xic,keyEvent,text,sizeof(text)-1,&keysym,&status);
    if (status == XBufferOverflow) {
        return;
    }
    if (status == XLookupChars) {
        this->OnImmCommit(text);
        return;
    }
    if (status == XLookupBoth) {
        if( (!(keyEvent->state & ControlMask)) && IsPrintableChar(keysym))
        {
            this->OnImmCommit(text);
            return;
        }
        if (keysym == VK_BACKSPACE) {
            if(m_currentEditPos == 0){
                return;
            }
            uint32_t totalBytes = GetCharactersBytes(m_currentEditPos);
            const char *p = m_text.c_str() + totalBytes;
            const char *charStart = CharPrev(m_text.c_str(),p);
            m_text.erase(charStart - m_text.c_str(), p-charStart);
            m_textWidthList.Remove(m_currentEditPos);
            m_currentEditPos--;
            m_uiEdit->SetText(UIString{m_text.c_str()});
        }
        if(keysym == VK_DELETE){
            if(m_currentEditPos == m_textWidthList.GetSize()){
                return;
            }
            int totalBytes = GetCharactersBytes(m_currentEditPos);
            const char *p = m_text.c_str() + totalBytes;
            const char *nextChar = CharNext(p);
            m_text.erase(p - m_text.c_str(), nextChar-p);
            m_textWidthList.Remove(m_currentEditPos+1);
            m_uiEdit->SetText(UIString{m_text.c_str()});
        }
    }
    if (status == XLookupKeySym) {
        if(keysym == VK_HOME || keysym == XK_KP_Home){
            m_currentEditPos = 0;
            m_uiEdit->Invalidate();
            return;
        }
        if(keysym == VK_END || keysym == XK_KP_End){
            m_currentEditPos = m_textWidthList.GetSize()-1;
            m_uiEdit->Invalidate();
            return;
        }
        if(keysym == VK_LEFT || keysym == XK_KP_End){
            if(m_currentEditPos != 0){
                m_currentEditPos--;
                m_uiEdit->Invalidate();
            }
            return;
        }
        if(keysym == VK_RIGHT || keysym == XK_KP_Right){
            if(m_currentEditPos >= m_textWidthList.GetSize()-1){
                m_currentEditPos = m_textWidthList.GetSize()-1;
                return;
            }else{
                m_currentEditPos += 1;
            }
            m_uiEdit->Invalidate();
            return;
        }
        if (keysym == XK_KP_Delete) {
            if(m_currentEditPos == m_textWidthList.GetSize()){
                return;
            }
            int totalBytes = GetCharactersBytes(m_currentEditPos);
            const char *p = m_text.c_str() + totalBytes;
            const char *nextChar = CharNext(p);
            m_text.erase(p - m_text.c_str(), nextChar-p);
            m_textWidthList.Remove(m_currentEditPos+1);
            m_uiEdit->SetText(UIString{m_text.c_str()});
        }
    }
}

int UIEditInternal::GetCharactersBytes(int numberOfChars) {
    if(numberOfChars > GetNumberOfCharacters(UIString{m_text.c_str()})){
        numberOfChars = GetNumberOfCharacters(UIString{m_text.c_str()});
    }
    int totalBytes = 0;
    const char *start = m_text.c_str();
    const char *end = start + m_text.length();
    const char *next = CharNext(start);
    while(numberOfChars-- && start<end){
        totalBytes += (next - start);
        start = next;
        next = CharNext(start);
    }
    return totalBytes;
}

void UIEditInternal::InsertNewCharactersWidth(const char *insertString) {
    HANDLE_DC hDC = m_uiEdit->GetManager()->GetPaintDC();
    PangoFontMap *font_map = pango_xft_get_font_map(hDC->x11Window->display,hDC->x11Window->screen);
    PangoContext *context = pango_font_map_create_context(font_map);

    // 创建 Pango 布局对象
    PangoLayout *layout = pango_layout_new(context);
    UIFont *font = UIResourceMgr::GetInstance().GetFont(m_uiEdit->GetFont());
    pango_layout_set_font_description(layout, font->GetHandle());
    if(m_uiEdit->IsPasswordMode()){
        UpdateInsertedPasswordCharacterWidth(insertString, layout);
    }else{
        UpdateInsertedNormalTextWidth(insertString, layout);
    }
    g_object_unref(layout);
    g_object_unref(context);
}

void
UIEditInternal::UpdateInsertedNormalTextWidth(const char *insertString, PangoLayout *layout) {
    uint32_t numberOfCharacters = GetNumberOfCharacters(UIString{insertString});
    const char *start = insertString;
    const char *next = CharNext(insertString);
    while(numberOfCharacters--){
        int width = 0;
        int height = 0;
        pango_layout_set_text(layout, start, next-start);
        pango_layout_get_pixel_size(layout, &width, &height);
        m_textWidthList.InsertAt(++m_currentEditPos, (LPVOID)(long)width);
        start = next;
        next = CharNext(next);
    }
}

void UIEditInternal::UpdateInsertedPasswordCharacterWidth(const char *insertString,PangoLayout *layout) {
    int width = 0;
    int height = 0;
    uint32_t numberOfCharacters = GetNumberOfCharacters(UIString{insertString});
    char passwordCharacter = m_uiEdit->GetPasswordChar();
    pango_layout_set_text(layout, &passwordCharacter,1);
    pango_layout_get_pixel_size(layout, &width, &height);
    while(numberOfCharacters--){
        m_textWidthList.InsertAt(++m_currentEditPos, (LPVOID)(long)width);
    }
}

uint32_t UIEdit::GetCharNumber() const {
    return GetNumberOfCharacters(m_text);
}

void UIEdit::InitInternal() {
    m_internalImpl = make_shared<UIEditInternal>(this);
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

        Invalidate();
    }
    if( event.Type == UIEVENT_KILLFOCUS && IsEnabled() )
    {
        m_internalImpl->ReleaseImmContext();
        //::HideCaret(m_manager->GetPaintWindow());
        Invalidate();
    }
    if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK || event.Type == UIEVENT_RBUTTONDOWN)
    {
        m_internalImpl->DoEvent(event);
        if( IsEnabled() ) {
            //GetManager()->ReleaseCapture();
        }
        Invalidate();
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
    if(event.Type == UIEVENT_KEYDOWN){

        m_internalImpl->OnKeyDown(event);
        return;
    }
    UILabel::DoEvent(event);
}

void UIEdit::DrawCaret(HANDLE_DC hDC) {
    m_internalImpl->DrawCaret(hDC);
}
