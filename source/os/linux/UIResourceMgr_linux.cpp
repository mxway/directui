#include <UIResourceMgr.h>
#include <unistd.h>
#include <libgen.h>

UIFont *glbSystemDefaultGUIFont=nullptr;

static void CreateSystemDefaultGUIFont()
{
    PangoContext *pangoContext = pango_font_map_create_context(pango_cairo_font_map_get_default());
    PangoFontDescription *fontDescription = pango_context_get_font_description(pangoContext);
    glbSystemDefaultGUIFont = new UIFont{
            UIString{pango_font_description_get_family(fontDescription)},
            pango_font_description_get_size(fontDescription)/PANGO_SCALE,
            pango_font_description_get_weight(fontDescription)==PANGO_WEIGHT_BOLD,
            false,
            pango_font_description_get_style(fontDescription) == PANGO_STYLE_ITALIC
    };
    g_object_unref(pangoContext);
    glbSystemDefaultGUIFont->Create();
}

UIResourceMgr::UIResourceMgr()
    :m_defaultFont {nullptr}
{
    CreateSystemDefaultGUIFont();
    char exeFileName[4096] = {0};
    ssize_t size = readlink("/proc/self/exe", exeFileName, 4094);
    if(size == -1){
        printf("Cannot get current execute file path.\n");
        return;
    }
    m_currentDir = UIString{dirname(exeFileName)};
    m_strResDir = m_currentDir;
}

UIResourceMgr &UIResourceMgr::GetInstance() {
    static UIResourceMgr uiResourceMgr;
    return uiResourceMgr;
}

void UIResourceMgr::Init(int argc, char **argv) {
    gtk_init(&argc, &argv);
    m_argc = argc;
    for(int i=0;i<argc;i++){
        m_args.Add(new UIString{argv[i]});
    }
}

bool UIResourceMgr::AddImage(const UIString &image) {

    UIString    strPath = m_strResDir + "/" + image;
    GdkPixbuf *pixBuf = gdk_pixbuf_new_from_file(strPath.GetData(),nullptr);
    if(pixBuf==nullptr)
    {
        return false;
    }
    auto  *imageInfo = new TImageInfo ;
    imageInfo->hBitmap = pixBuf;
    imageInfo->bAlpha = true;
    imageInfo->nX = gdk_pixbuf_get_width(pixBuf);
    imageInfo->nY = gdk_pixbuf_get_height(pixBuf);
    if(!m_strImageMap.Insert(image, imageInfo)){
        g_object_unref(imageInfo->hBitmap);
        delete imageInfo;
        return false;
    }
    return true;
}

TImageInfo *UIResourceMgr::GetImage(const UIString &image, bool bAdd) {
    auto  *data = static_cast<TImageInfo*>(m_strImageMap.Find(image));
    if(data == nullptr){
        if(bAdd && AddImage(image)){
            data = static_cast<TImageInfo*>(m_strImageMap.Find(image));
        }
    }
    return data;
}

void UIResourceMgr::FreeImageInfo(TImageInfo *imageInfo) {
    if(imageInfo->hBitmap){
        g_object_unref(imageInfo->hBitmap);
    }
    delete imageInfo;
}

void UIResourceMgr::RemoveImage(const UIString &image) {
    auto *data = static_cast<TImageInfo*>(m_strImageMap.Find(image));
    if (data)
    {
        FreeImageInfo(data) ;
        m_strImageMap.Remove(image);
    }
}

void UIResourceMgr::ReleaseDefaultFont() {
    delete glbSystemDefaultGUIFont;
}

UIFont *UIResourceMgr::GetDefaultFont() {
    if(m_defaultFont){
        return m_defaultFont;
    }
    return glbSystemDefaultGUIFont;
}
