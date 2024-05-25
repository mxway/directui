#include <UIEdit.h>
#include <UIPaintManager.h>
#include "UIRect.h"
#include "UIResourceMgr.h"

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

struct UIEditInternal
{
    explicit UIEditInternal(UIEdit *uiEdit);
    ~UIEditInternal();
    void    DoEvent(TEventUI &event);
    void    CreateImmContext(int initX, int initY);
    void    ReleaseImmContext();
    void    OnImmCommit(const gchar *str);
    void    CalculateCharactersWidth();
    void    CalculateCurrentEditPositionFromMousePoint(POINT ptMouse);
    void    DrawCaret();
    void    OnKeyDown(TEventUI &event);
    //
    int     GetCharactersBytes(int numberOfChars){
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

    void   InsertNewCharactersWidth(const char *insertString){
        int numberOfCharacters = GetNumberOfCharacters(UIString{insertString});
        const char *start = insertString;
        const char *next = CharNext(insertString);
        PangoLayout *layout = pango_cairo_create_layout(m_uiEdit->GetManager()->GetPaintDC());
        UIFont *font = UIResourceMgr::GetInstance().GetFont(m_uiEdit->GetFont());
        pango_layout_set_font_description(layout, font->GetHandle());
        while(numberOfCharacters--){
            int width = 0;
            int height = 0;
            pango_layout_set_text(layout, start, next-start);
            pango_layout_get_pixel_size(layout, &width, &height);
            m_textWidthList.InsertAt(++m_currentEditPos,(LPVOID)(long)width);
            start = next;
            next = CharNext(next);
        }
        g_object_unref(layout);
    }
private:
    GtkIMContext    *m_imContext;
    gulong          m_handlerId;
    UIEdit          *m_uiEdit;
    string          m_text;
    UIPtrArray      m_textWidthList;
    int             m_currentEditPos;
};

static void imm_commit_callback(GtkIMContext *self, gchar *str, gpointer user_data)
{
    auto *uiEditInternal = static_cast<UIEditInternal*>(user_data);
    uiEditInternal->OnImmCommit(str);
}

UIEditInternal::UIEditInternal(UIEdit *uiEdit)
        :m_imContext{nullptr},
         m_handlerId{0},
         m_uiEdit{uiEdit},
         m_text{uiEdit->GetText().GetData()},
         m_currentEditPos{-1}
{

}

UIEditInternal::~UIEditInternal() {
    this->ReleaseImmContext();
}

void UIEditInternal::DoEvent(TEventUI &event) {
    if(event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK || event.Type == UIEVENT_RBUTTONDOWN)
    {
        //cout<<"DoEvent:"<<endl;
        m_text = m_uiEdit->GetText().GetData();
        this->CreateImmContext(event.ptMouse.x, event.ptMouse.y);
        this->CalculateCharactersWidth();
        this->CalculateCurrentEditPositionFromMousePoint(event.ptMouse);
        //gtk_draw_insertion_cursor
    }
    if(event.Type == UIEVENT_KEYDOWN){
        if(m_imContext){
            gtk_im_context_filter_keypress(m_imContext,(GdkEventKey*)event.wParam);
            return;
        }
    }
}

void UIEditInternal::CreateImmContext(int initX, int initY) {
    if(m_imContext == nullptr){
        m_imContext = gtk_im_multicontext_new();
        GdkWindow  *gdkWindow = gtk_widget_get_window(m_uiEdit->GetManager()->GetPaintWindow());
        gtk_im_context_set_client_window(m_imContext,gdkWindow);
        gtk_im_context_focus_in(m_imContext);
        GdkRectangle    rectangle={0};
        rectangle.x = initX;
        rectangle.y = initY;
        rectangle.height = 100;
        rectangle.width = 100;
        gtk_im_context_set_cursor_location(m_imContext,&rectangle);
        m_handlerId = g_signal_connect(m_imContext, "commit", G_CALLBACK(imm_commit_callback),this);
    }
}

void UIEditInternal::ReleaseImmContext() {
    if(m_imContext!=nullptr){
        g_signal_handler_disconnect(m_imContext,m_handlerId);
        g_clear_object(&m_imContext);
        m_imContext = nullptr;
    }
    m_currentEditPos = -1;
    m_uiEdit->Invalidate();
}

void UIEditInternal::OnImmCommit(const gchar *str) {
    uint32_t totalBytes = this->GetCharactersBytes(m_currentEditPos);
    m_text.insert(totalBytes, str);
    this->InsertNewCharactersWidth(str);
    m_uiEdit->SetText(UIString{m_text.c_str()});
}

void UIEditInternal::CalculateCharactersWidth() {
    PangoLayout *layout = pango_cairo_create_layout(m_uiEdit->GetManager()->GetPaintDC());
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

void UIEditInternal::DrawCaret() {
    if(m_currentEditPos < 0){
        return;
    }
    RECT rcItem{m_uiEdit->GetPos()};
    GdkRectangle rect = {0};
    int totalWidth = 0;
    for(int i=0;i<=m_currentEditPos && i<m_textWidthList.GetSize();i++){
        totalWidth += (int)(long)m_textWidthList.GetAt(i);
    }
    RECT rcTextPadding = m_uiEdit->GetTextPadding();
    rect.x = totalWidth + rcItem.left + rcTextPadding.left;
    rect.y = rcItem.top + rcTextPadding.top + 4;
    rect.width = 1;
    rect.height = rcItem.bottom - rcItem.top;
#if GTK_CHECK_VERSION(3,4,0)
    GtkStyleContext *styleContext = gtk_widget_get_style_context(m_uiEdit->GetManager()->GetPaintWindow());
    PangoLayout *layout = pango_cairo_create_layout(m_uiEdit->GetManager()->GetPaintDC());
    UIFont *font = UIResourceMgr::GetInstance().GetFont(m_uiEdit->GetFont());
    pango_layout_set_font_description(layout, font->GetHandle());
    gtk_render_insertion_cursor(styleContext,m_uiEdit->GetManager()->GetPaintDC(),rect.x,rect.y,layout,0,PANGO_DIRECTION_LTR);
    g_object_unref(layout);
#else
    gtk_draw_insertion_cursor(m_uiEdit->GetManager()->GetPaintWindow(),m_uiEdit->GetManager()->GetPaintDC(),
                                  &rect,true,GTK_TEXT_DIR_LTR,false );
#endif
}

void UIEditInternal::OnKeyDown(TEventUI &event) {
    auto  *eventKey = (GdkEventKey*)event.wParam;
    if(eventKey->keyval == VK_HOME){
        m_currentEditPos = 0;
        m_uiEdit->Invalidate();
        return;
    }
    if(eventKey->keyval == VK_END){
        m_currentEditPos = m_textWidthList.GetSize()-1;
        m_uiEdit->Invalidate();
        return;
    }
    if(eventKey->keyval == VK_LEFT){
        if(m_currentEditPos != 0){
            m_currentEditPos--;
            m_uiEdit->Invalidate();
        }
        return;
    }
    if(eventKey->keyval == VK_RIGHT){
        if(m_currentEditPos >= m_textWidthList.GetSize()-1){
            m_currentEditPos = m_textWidthList.GetSize()-1;
            return;
        }else{
            m_currentEditPos += 1;
        }
        m_uiEdit->Invalidate();
        return;
    }
    if(eventKey->keyval == VK_BACKSPACE){
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
        return;
    }
    if(eventKey->keyval == VK_DELETE){
        if(m_currentEditPos == m_textWidthList.GetSize()){
            return;
        }
        int totalBytes = GetCharactersBytes(m_currentEditPos);
        const char *p = m_text.c_str() + totalBytes;
        const char *nextChar = CharNext(p);
        m_text.erase(p - m_text.c_str(), nextChar-p);
        m_textWidthList.Remove(m_currentEditPos+1);
        m_uiEdit->SetText(UIString{m_text.c_str()});
        return;
    }
    if(m_imContext != nullptr){
        gtk_im_context_filter_keypress(m_imContext,(GdkEventKey*)event.wParam);
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

void UIEdit::DrawCaret() {
    m_internalImpl->DrawCaret();
}
