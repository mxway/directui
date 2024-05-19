#ifndef DIRECTUI_UISTRINGPTRMAP_H
#define DIRECTUI_UISTRINGPTRMAP_H
#include <UIString.h>
#include "UIDefine.h"

struct TITEM;

class UIStringPtrMap
{
public:
    explicit UIStringPtrMap(int nSize=83);
    ~UIStringPtrMap();

    void Resize(int nSize=83);
    LPVOID Find(const UIString &key, bool optimize=true)const;
    bool   Insert(const UIString &key, LPVOID pData);
    LPVOID  Set(const UIString &key, LPVOID pData);
    bool    Remove(const UIString &key);
    void    RemoveAll();
    int     GetSize()const;
    UIString GetAt(int iIndex)const;
    UIString operator[](int nIndex)const;
private:
    void    Destroy();
protected:
    TITEM       **m_aT;
    int         m_nBuckets;
    int         m_nCount;
};

#endif //DIRECTUI_UISTRINGPTRMAP_H
