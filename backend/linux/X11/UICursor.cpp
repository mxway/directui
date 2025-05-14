#include "UICursor.h"
#include <UIBackend.h>
#include "DisplayInstance.h"
#include "X11Window.h"

UICursor::UICursor() {
    Display *display = DisplayInstance::GetInstance().GetDisplay();
    m_cursorMapping.insert(make_pair(UI_IDC_ARROW,XCreateFontCursor(display,UI_IDC_ARROW)));
    m_cursorMapping.insert(make_pair(UI_IDC_HAND,XCreateFontCursor(display,UI_IDC_HAND)));
    m_cursorMapping.insert(make_pair(UI_IDC_TEXT,XCreateFontCursor(display,UI_IDC_TEXT)));
    m_cursorMapping.insert(make_pair(UI_IDC_RESIZEWE,XCreateFontCursor(display,UI_IDC_RESIZEWE)));
    m_cursorMapping.insert(make_pair(UI_IDC_RESIZENS,XCreateFontCursor(display,UI_IDC_RESIZENS)));
}

UICursor::~UICursor() {
    Display *display = DisplayInstance::GetInstance().GetDisplay();
    for (auto itr : m_cursorMapping) {
        XFreeCursor(display,itr.second);
    }
}

UICursor& UICursor::GetInstance() {
    static UICursor cursor;
    return cursor;
}

void UICursor::LoadCursor(UIPaintManager *manager,int cursor){
    X11Window *window = manager->GetPaintWindow();
    auto itr = m_cursorMapping.find(cursor);
    if (itr == m_cursorMapping.end()) {
        return;
    }
    XDefineCursor(window->display,window->window,itr->second);
}

void UILoadCursorX11(UIPaintManager *manager,int cursor) {
    UICursor::GetInstance().LoadCursor(manager,cursor);
}