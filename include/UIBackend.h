#ifndef DIRECTUI_UIBACKEND_H
#define DIRECTUI_UIBACKEND_H

#ifdef WIN32
    // win32 use native api
    #include <windows.h>
    typedef HWND HANDLE_WND;
    typedef HDC HANDLE_DC;
    typedef HBITMAP     HANDLE_BITMAP;
    typedef HFONT       HANDLE_FONT;
    #define UILoadCursor(manager,CURSOR) ::SetCursor(::LoadCursor(nullptr,CURSOR));

    #define UI_IDC_ARROW        IDC_ARROW
    #define UI_IDC_HAND         IDC_HAND
    #define UI_IDC_TEXT         IDC_IBEAM
    #define UI_IDC_RESIZEWE     IDC_SIZEWE
    #define UI_IDC_RESIZENS     IDC_SIZENS

    #define UI_APP_QUIT()           ::PostQuitMessage(0)
    #define UI_DESTROY_WINDOW(wnd)  ::DestroyWindow(wnd)
    #define UI_CLOSE_WINDOW(wnd, ret) ::PostMessageW(wnd, WM_CLOSE, WPARAM(ret),0)

    #ifdef _MSC_VER
    #define strcasecmp _stricmp
    #endif
    // win32 native api end

#elif defined(GTK_BACKEND)
    // use gtk as backend
    #include <gtk/gtk.h>
    typedef GtkWidget *HANDLE_WND;
    typedef cairo_t   *HANDLE_DC;
    typedef GdkPixbuf   *HANDLE_BITMAP;
    typedef PangoFontDescription *HANDLE_FONT;

    #define DT_TOP              0x00000001
    #define DT_LEFT             0x00000002
    #define DT_CENTER           0x00000004
    #define DT_RIGHT            0x00000008
    #define DT_VCENTER          0x00000010
    #define DT_BOTTOM           0x00000020
    #define DT_WORDBREAK        0x00000040
    #define DT_SINGLELINE       0x00000080
    #define DT_CALCRECT         0x00000400
    #define DT_END_ELLIPSIS     0x00008000

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

    /*
     * Key State Masks for Mouse Messages
     */
    #define MK_LBUTTON          0x0001
    #define MK_RBUTTON          0x0002
    #define MK_SHIFT            0x0004
    #define MK_CONTROL          0x0008
    #define MK_MBUTTON          0x0010

    #define UI_APP_QUIT()       gtk_main_quit()
    #define UI_DESTROY_WINDOW(wnd)                          \
       do{                                                  \
            GdkEvent  *event = gdk_event_new(GDK_DELETE);   \
            event->any.window = GDK_WINDOW(g_object_ref(G_OBJECT(gtk_widget_get_window(wnd)))); \
            event->any.send_event = true;                   \
            gtk_main_do_event(event);                       \
            gdk_event_free(event);                          \
       }while(0);

    #define UI_CLOSE_WINDOW(wnd, ret)                          \
       do{                                                  \
            GdkEvent  *event = gdk_event_new(GDK_DELETE);   \
            event->any.window = GDK_WINDOW(g_object_ref(G_OBJECT(gtk_widget_get_window(wnd)))); \
            event->any.send_event = true;                   \
            event->configure.send_event = ret;               \
            gtk_main_do_event(event);                       \
            gdk_event_free(event);                          \
       }while(0);

    // use gtk as backend
#else
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
    #include <X11/Xft/Xft.h>
    #include <pango/pango-font.h>
    //x11 as the default backend
    typedef struct _X11Window_s{
        Display     *display;
        int         screen;
        Window      window;
        Visual      *visual;
        Colormap    colormap;
        Pixmap      offscreenPixmap;
        int         depth;
        int         x;
        int         y;
        int         width;
        int         height;
        GC          gc;
    }X11Window;

    typedef X11Window   *HANDLE_WND;
    typedef Drawable    HANDLE_DC;
    typedef Pixmap      HANDLE_BITMAP;
    typedef PangoFontDescription *HANDLE_FONT;

    #define UI_CLOSE_WINDOW(wnd, ret)                          \
       do{                                                  \
            exit(0);                          \
       }while(0);
#endif

typedef struct tagTImageInfo
{
    HANDLE_BITMAP  hBitmap;
    LPBYTE  pBits;
    LPBYTE  pSrcBits;
    int nX;
    int nY;
    bool bAlpha;
    bool bUseHSL;
    UIString sResType;
    uint32_t mask;
}TImageInfo;

typedef struct tagTDrawInfo
{
    tagTDrawInfo();
    explicit tagTDrawInfo(const char *lpsz);
    void Clear();
    UIString sDrawString;
    UIString sImageName;
    bool     bLoaded;
    const TImageInfo  *pImageInfo;
    RECT   rcDestOffset;
    RECT   rcBmpPart;
    RECT   rcScale9;
    uint8_t  uFade;
    bool   bHole;
    bool   bTiledX;
    bool   bTiledY;
}TDrawInfo;

#endif //DIRECTUI_UIBACKEND_H
