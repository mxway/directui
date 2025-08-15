#ifndef DIRECTUI_UIBACKEND_H
#define DIRECTUI_UIBACKEND_H

#ifdef WIN32
    // win32 use native api
    #include <windows.h>
    typedef HWND HANDLE_WND;
    typedef HDC HANDLE_DC;
    typedef HBITMAP     HANDLE_BITMAP;
    typedef HFONT       HANDLE_FONT;
    typedef HWND    WindowEventType;
    #define UILoadCursor(manager,CURSOR) ::SetCursor(::LoadCursor(nullptr,CURSOR));

    #define UI_IDC_ARROW        IDC_ARROW
    #define UI_IDC_HAND         IDC_HAND
    #define UI_IDC_TEXT         IDC_IBEAM
    #define UI_IDC_RESIZEWE     IDC_SIZEWE
    #define UI_IDC_RESIZENS     IDC_SIZENS

    #define UI_APP_QUIT()           ::PostQuitMessage(0)
    #define UI_DESTROY_WINDOW(wnd)  ::DestroyWindow(wnd)

    #ifdef _MSC_VER
    #define strcasecmp _stricmp
    #endif

    //定义窗口类型
    #define UI_WNDSTYLE_FRAME      (WS_VISIBLE | WS_OVERLAPPEDWINDOW)
    #define UI_WNDSTYLE_CHILD      (WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN)
    #define UI_WNDSTYLE_DIALOG     (WS_VISIBLE | WS_POPUPWINDOW | WS_CAPTION | WS_DLGFRAME | WS_CLIPSIBLINGS | WS_CLIPCHILDREN)

    #define UI_WNDSTYLE_EX_FRAME   (WS_EX_WINDOWEDGE)
    #define UI_WNDSTYLE_EX_DIALOG  (WS_EX_TOOLWINDOW | WS_EX_DLGMODALFRAME)

    // win32 native api end

#elif defined(GTK_BACKEND)
    // use gtk as backend
    #include <gtk/gtk.h>
    typedef GtkWidget *HANDLE_WND;
    typedef cairo_t   *HANDLE_DC;
    typedef GdkPixbuf   *HANDLE_BITMAP;
    typedef PangoFontDescription *HANDLE_FONT;
    typedef GtkWidget  *WindowEventType;

    #define VK_DOWN         GDK_KEY_Down
    #define VK_UP           GDK_KEY_Up
    #define VK_NEXT         GDK_KEY_Next
    #define VK_PRIOR        GDK_KEY_Prior
    #define VK_HOME         GDK_KEY_Home
    #define VK_END          GDK_KEY_End
    #define VK_SPACE        GDK_KEY_space
    #define VK_RETURN       GDK_KEY_Return
    #define VK_ESCAPE       GDK_KEY_Escape
    #define VK_LEFT         GDK_KEY_Left
    #define VK_RIGHT        GDK_KEY_Right
    #define VK_BACKSPACE    GDK_KEY_BackSpace
    #define VK_DELETE       GDK_KEY_Delete
    #define VK_F4           GDK_KEY_F4

    #define SB_LINEUP       GDK_SCROLL_UP
    #define SB_LINEDOWN     GDK_SCROLL_DOWN
    #define UILoadCursor(manager,CURSOR) \
       do{                 \
            GdkCursor  *cursor = gdk_cursor_new_from_name(gtk_widget_get_display(manager->GetPaintWindow()), CURSOR);   \
            gdk_window_set_cursor(gtk_widget_get_window(manager->GetPaintWindow()), cursor);  \
            if(cursor){         \
                g_object_unref(G_OBJECT(cursor));       \
            }                            \
       }while(0);

    //定义鼠标光标
    #define UI_IDC_ARROW        "default"
    #define UI_IDC_HAND         "pointer"
    #define UI_IDC_TEXT         "text"
    #define UI_IDC_RESIZEWE     "ew-resize"
    #define UI_IDC_RESIZENS     "ns-resize"

    #define UI_APP_QUIT()       gtk_main_quit()
    #define UI_DESTROY_WINDOW(wnd)                          \
       do{                                                  \
            GdkEvent  *event = gdk_event_new(GDK_DELETE);   \
            event->any.window = GDK_WINDOW(g_object_ref(G_OBJECT(gtk_widget_get_window(wnd)))); \
            event->any.send_event = true;                   \
            gtk_main_do_event(event);                       \
            gdk_event_free(event);                          \
       }while(0);

    //定义窗口类型
    #define UI_WNDSTYLE_FRAME      (GTK_WINDOW_TOPLEVEL)
    #define UI_WNDSTYLE_CHILD      (GTK_WINDOW_POPUP)
    #define UI_WNDSTYLE_DIALOG     (GTK_WINDOW_TOPLEVEL)

    // use gtk as backend
#else
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
    #include <X11/Xft/Xft.h>
    #include<X11/Xatom.h>
    #include <locale.h>
    #include <X11/cursorfont.h>
    #include <pango/pango-font.h>

    //x11 as the default backend
    struct X11WindowHDC;
    struct X11Bitmap;
    struct X11Window;

    typedef X11Window       *HANDLE_WND;
    typedef X11WindowHDC    *HANDLE_DC;
    typedef X11Bitmap       *HANDLE_BITMAP;
    typedef PangoFontDescription *HANDLE_FONT;

    typedef Window          WindowEventType;
    void    UIAppQuitX11();
    #define UI_APP_QUIT()       UIAppQuitX11()

    #define VK_DOWN         XK_Down
    #define VK_UP           XK_Up
    #define VK_NEXT         XK_Next
    #define VK_PRIOR        XK_Prior
    #define VK_HOME         XK_Home
    #define VK_END          XK_End
    #define VK_SPACE        XK_space
    #define VK_RETURN       XK_Return
    #define VK_ESCAPE       XK_Escape
    #define VK_LEFT         XK_Left
    #define VK_RIGHT        XK_Right
    #define VK_BACKSPACE    XK_BackSpace
    #define VK_DELETE       XK_Delete
    #define VK_F4           XK_F4
    #define SB_LINEUP       Button4Mask
    #define SB_LINEDOWN     Button5Mask

    class UIPaintManager;
    void UILoadCursorX11(UIPaintManager *,int);
    #define UILoadCursor(manager,CURSOR) \
       do{                 \
                UILoadCursorX11(manager,CURSOR);\
       }while(0);

    //定义鼠标光标
    #define UI_IDC_ARROW        XC_left_ptr
    #define UI_IDC_HAND         XC_hand1
    #define UI_IDC_TEXT         XC_xterm
    #define UI_IDC_RESIZEWE     XC_sb_h_double_arrow
    #define UI_IDC_RESIZENS     XC_sb_v_double_arrow

    //定义窗口类型
    #define UI_WNDSTYLE_FRAME       0x01
    #define UI_WNDSTYLE_CHILD       0x02
    #define UI_WNDSTYLE_DIALOG      0x04
#endif

#endif //DIRECTUI_UIBACKEND_H
