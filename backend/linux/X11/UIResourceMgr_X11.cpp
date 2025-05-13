#include "UIResourceMgr.h"
#include <unistd.h>
#include <libgen.h>
#include <pango/pango-font.h>
#include <pango/pango-context.h>
#include <pango/pangoxft.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "DisplayInstance.h"
#include "../../../src/SkinFileReaderService.h"
#include "X11HDC.h"
#include "X11Bitmap.h"

UIFont *glbSystemDefaultGUIFont=nullptr;

static void CreateSystemDefaultGUIFont()
{
    PangoFontMap *font_map = pango_xft_get_font_map(DisplayInstance::GetInstance().GetDisplay(),
                                                    DisplayInstance::GetInstance().GetScreenNumber());
    PangoContext *pangoContext = pango_font_map_create_context(font_map);
    //PangoContext *pangoContext = pango_font_map_create_context(pango_cairo_font_map_get_default());
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
        :m_defaultFont {nullptr},
         m_skinType{ResourceSkinType_Unknown}
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
    m_argc = argc;
    for(int i=0;i<argc;i++){
        m_args.Add(new UIString{argv[i]});
    }
}

bool UIResourceMgr::AddImage(const UIString &image) {
    ByteArray resultData = SkinFileReaderFactory::GetSkinFileReader()->ReadFile(image);
    if(resultData.m_bufferSize == 0){
        return false;
    }
    int width = 0;
    int height = 0;
    int channels = 0;
    stbi_uc *data = stbi_load_from_memory(resultData.m_buffer,resultData.m_bufferSize,&width,&height,&channels,4);
    delete []resultData.m_buffer;
    bool bAlphaChannel = false;
    auto *imageBuffer = (unsigned char*)malloc(width*height*4);
    for( int i = 0; i < width * height; i++ )
    {
        imageBuffer[i*4 + 3] = data[i*4 + 3];
        if( imageBuffer[i*4 + 3] < 255 )
        {
            imageBuffer[i*4] = (uint8_t)(uint32_t(data[i*4 + 2])*data[i*4 + 3]/255);
            imageBuffer[i*4 + 1] = (uint8_t)(uint32_t(data[i*4 + 1])*data[i*4 + 3]/255);
            imageBuffer[i*4 + 2] = (uint8_t)(uint32_t(data[i*4])*data[i*4 + 3]/255);
            bAlphaChannel = true;
        }
        else
        {
            imageBuffer[i*4] = data[i*4 + 2];
            imageBuffer[i*4 + 1] = data[i*4 + 1];
            imageBuffer[i*4 + 2] = data[i*4];
        }

        if( *(uint32_t *)(&imageBuffer[i*4]) == 0 ) {
            imageBuffer[i*4] = 0;
            imageBuffer[i*4 + 1] = 0;
            imageBuffer[i*4 + 2] = 0;
            imageBuffer[i*4 + 3] = 0;
            bAlphaChannel = true;
        }
    }
    stbi_image_free(data);

    auto  *imageInfo = new TImageInfo ;
    imageInfo->hBitmap = new X11Bitmap;
    imageInfo->hBitmap->buffer = imageBuffer;
    imageInfo->hBitmap->bufferSize = width*height*4;
    imageInfo->hBitmap->width = width;
    imageInfo->hBitmap->height = height;
    imageInfo->bAlpha = bAlphaChannel;
    imageInfo->nX = width;
    imageInfo->nY = height;
    if(!m_strImageMap.Insert(image, imageInfo)){
        delete imageInfo->hBitmap;
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
    delete imageInfo->hBitmap;
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
