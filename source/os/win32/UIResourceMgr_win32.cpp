#include <UIResourceMgr.h>
#include "stb_image.h"
#include <shlwapi.h>
#include "EncodingTransform.h"
#include "../src/SkinFileReaderService.h"

static UIFont   *glbSystemDefaultFont;

static UIString GetCurrentModulePath()
{
    wchar_t   moduleFileName[MAX_PATH+2] = {0};
    ::GetModuleFileNameW(GetModuleHandleW(nullptr), moduleFileName, MAX_PATH);
    ::PathRemoveFileSpecW(moduleFileName);
    char *currentPath = Ucs2ToUtf8(moduleFileName);
    UIString result{currentPath};
    delete []currentPath;
    return result;
}

UIResourceMgr::UIResourceMgr()
    :m_defaultFont {nullptr},
    m_skinType{ResourceSkinType_Unknown}
{
    LOGFONTW lf = {0};
    ::GetObjectW(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONTW),&lf);
    glbSystemDefaultFont = new UIFont {UIString{""},-lf.lfHeight,(lf.lfWeight&FW_BOLD)==FW_BOLD,
                                       lf.lfUnderline!=0,lf.lfItalic!=0};
    glbSystemDefaultFont->Create();
    m_strResDir = GetCurrentModulePath();
    m_currentDir = m_strResDir;
}

UIResourceMgr &UIResourceMgr::GetInstance() {
    static UIResourceMgr    uiResourceMgr;
    return uiResourceMgr;
}

void UIResourceMgr::Init(int argc, char **argv) {
    UNUSED_PARAMETER(argv);
    m_argc = argc;
    wchar_t **localArgs = CommandLineToArgvW(GetCommandLineW(),&argc);
    for(int i=0;i<m_argc;i++){
        char *utf8String = Ucs2ToUtf8(localArgs[i]);
        m_args.Add(new UIString{utf8String});
        delete []utf8String;
    }
    ::LocalFree(localArgs);
}

static TImageInfo*      LoadImageToMemory(const UIString &image)
{
    ByteArray resultData = SkinFileReaderFactory::GetSkinFileReader()->ReadFile(image);
    int x = 1, y = 1, n;
    LPBYTE pImage = stbi_load_from_memory(resultData.m_buffer, (int)resultData.m_bufferSize, &x, &y, &n, 4);
    delete[] resultData.m_buffer;
    if(pImage == nullptr){
        return nullptr;
    }
    BITMAPINFO bmi;
    ::ZeroMemory(&bmi, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = x;
    bmi.bmiHeader.biHeight = -y;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = x * y * 4;

    bool bAlphaChannel = false;
    LPBYTE pDest = nullptr;
    HBITMAP hBitmap = ::CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, (void**)&pDest, nullptr, 0);
    if( !hBitmap ) {
        stbi_image_free(pImage);
        //::MessageBox(0, _T("CreateDIBSection失败"), _T("抓BUG"), MB_OK);
        return nullptr;
    }

    for( int i = 0; i < x * y; i++ )
    {
        pDest[i*4 + 3] = pImage[i*4 + 3];
        if( pDest[i*4 + 3] < 255 )
        {
            pDest[i*4] = (BYTE)(DWORD(pImage[i*4 + 2])*pImage[i*4 + 3]/255);
            pDest[i*4 + 1] = (BYTE)(DWORD(pImage[i*4 + 1])*pImage[i*4 + 3]/255);
            pDest[i*4 + 2] = (BYTE)(DWORD(pImage[i*4])*pImage[i*4 + 3]/255);
            bAlphaChannel = true;
        }
        else
        {
            pDest[i*4] = pImage[i*4 + 2];
            pDest[i*4 + 1] = pImage[i*4 + 1];
            pDest[i*4 + 2] = pImage[i*4];
        }

        if( *(DWORD*)(&pDest[i*4]) == 0 ) {
            pDest[i*4] = (BYTE)0;
            pDest[i*4 + 1] = (BYTE)0;
            pDest[i*4 + 2] = (BYTE)0;
            pDest[i*4 + 3] = (BYTE)0;
            bAlphaChannel = true;
        }
    }
    stbi_image_free(pImage);

    auto* data = new TImageInfo;
    data->hBitmap = hBitmap;
    data->pBits = pDest;
    data->nX = x;
    data->nY = y;
    data->bAlpha = bAlphaChannel;
    data->bUseHSL = false;
    data->pSrcBits = nullptr;
    return data;
}

bool UIResourceMgr::AddImage(const UIString &image) {
    TImageInfo  *data = LoadImageToMemory(image);
    if(data == nullptr){
        return false;
    }
    m_strImageMap.Insert(image, data);
    return true;
}

TImageInfo   *UIResourceMgr::GetImage(const UIString &image, bool bAdd) {
    auto  *data = static_cast<TImageInfo*>(m_strImageMap.Find(image));
    if(data == nullptr){
        if(bAdd && AddImage(image)){
            data = static_cast<TImageInfo*>(m_strImageMap.Find(image));
        }
    }
    return data;
}

void UIResourceMgr::FreeImageInfo(TImageInfo *imageInfo) {
    if(imageInfo == nullptr){
        return;
    }
    if(imageInfo->hBitmap){
        ::DeleteObject(imageInfo->hBitmap);
        imageInfo->hBitmap = nullptr;
    }
    if(imageInfo->pSrcBits){
        delete []imageInfo->pSrcBits;
        imageInfo->pSrcBits = nullptr;
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
    delete glbSystemDefaultFont;
}

UIFont *UIResourceMgr::GetDefaultFont() {
    if(m_defaultFont != nullptr){
        return m_defaultFont;
    }
    return glbSystemDefaultFont;
}
