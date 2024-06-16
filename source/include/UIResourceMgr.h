#ifndef DIRECTUI_UIRESOURCEMGR_H
#define DIRECTUI_UIRESOURCEMGR_H
#include <UIStringPtrMap.h>
#include <UIStringPtrMap.h>
#include <UIFont.h>
#include "UIPtrArray.h"

class UIResourceMgr
{
public:
    enum ResourceSkinType{
        ResourceSkinType_File,
        ResourceSkinType_ZipFile,
        ResourceSkinType_ZipBuffer,
        ResourceSkinType_Unknown
    };
public:
    ~UIResourceMgr();
    UIResourceMgr(const UIResourceMgr &)=delete;
    UIResourceMgr &operator=(const UIResourceMgr &)=delete;
    UIResourceMgr(UIResourceMgr &&)=delete;
    UIResourceMgr &operator=(UIResourceMgr &&)=delete;
public:
    static UIResourceMgr& GetInstance();
    void        Init(int argc, char **argv);
    void        SetResourcePath(const UIString &path){
        m_skinType = ResourceSkinType_File;
        m_strResDir = path;
    }
    UIString    GetResourcePath()const{
        return m_strResDir;
    }

    UIString    GetCurrentPath()const{
        return m_currentDir;
    }

    bool            AddImage(const UIString &image);
    TImageInfo      *GetImage(const UIString &image, bool bAdd=false);
    void            RemoveImage(const UIString &image);
	void			ReleaseAllImages();

    void            SetResourceZip(const UIString &zipFile){
        m_zipFile = zipFile;
        m_skinType = ResourceSkinType_ZipFile;
    }

    UIString        GetResourceZip()const{
        return m_zipFile;
    }

    ResourceSkinType GetResourceSkinType(){
        return m_skinType;
    }

private:
    UIResourceMgr();
    static void            FreeImageInfo(TImageInfo *imageInfo);
    static void     ReleaseDefaultFont();
    void            ReleaseArgs();
private:
    UIStringPtrMap      m_strImageMap;
    UIString            m_strResDir;
    UIString            m_currentDir;
    ResourceSkinType    m_skinType;
    UIString            m_zipFile;

public:
    UIFont  *GetFont(const UIString &fontName,int size, bool bold, bool underLine, bool italic);
    UIFont  *GetFont(int fontId);
    bool    AddFont(int fontId, const UIString &faceName,bool defaultFont=false,
                     int size=0,bool bold=false, bool underLine =false, bool italic=false);
    void    ReleaseAllFonts();
    UIFont  *GetDefaultFont();
    uint32_t GetFontHeight(int fontId, HANDLE_DC hdc);
    uint32_t GetDefaultFontHeight(HANDLE_DC hdc);
private:
    int             m_argc{0};
    UIPtrArray      m_args;
    UIStringPtrMap  m_fontMapping;
    UIFont          *m_defaultFont;
};

#endif //DIRECTUI_UIRESOURCEMGR_H
