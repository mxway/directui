#include <UIScrollBar.h>
#include <UIPaintManager.h>

POINT UIScrollBar::GetCursorPos()const {
    POINT pt = { 0 };
#if GTK_CHECK_VERSION(3, 20, 0)
    GtkWidget  *widget = m_manager->GetPaintWindow();
    gdk_window_get_device_position(gtk_widget_get_window(widget),
                                   gdk_seat_get_pointer(gdk_display_get_default_seat(gtk_widget_get_display(widget))),
                                   reinterpret_cast<gint *>(&pt.x), reinterpret_cast<gint *>(&pt.y), 0);
#else
    gdk_window_get_device_position(gtk_widget_get_window(m_manager->GetPaintWindow()),
                         gdk_device_manager_get_client_pointer(
                             gdk_display_get_device_manager(gdk_window_get_display(m_manager->GetPaintWindow()))),
                         reinterpret_cast<gint *>(&pt.x), reinterpret_cast<gint *>(&pt.y), NULL);
#endif
    return pt;
}