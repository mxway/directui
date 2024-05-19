#ifndef DIRECTUI_UIRESOURCEMGR_H
#define DIRECTUI_UIRESOURCEMGR_H
#include <UIStringPtrMap.h>
#include <UIStringPtrMap.h>
#include <UIFont.h>
#include "UIPtrArray.h"

class UIResourceMgr
{
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

private:
    UIResourceMgr();
private:
    UIStringPtrMap      m_strImageMap;
    UIString            m_strResDir;
    UIString            m_currentDir;

public:
    UIFont  *GetFont(const UIString &fontName,int size, bool bold, bool underLine, bool italic);
    UIFont  *GetFont(int fontId);
    bool    AddFont(int fontId, const UIString &faceName,bool defaultFont=false,
                     int size=0,bool bold=false, bool underLine =false, bool italic=false);
    void    ReleaseAllFont();
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