#ifndef DIRECTUI_UIPTRARRAY_H
#define DIRECTUI_UIPTRARRAY_H
#include <UIDefine.h>

class UIPtrArray
{
public:
    explicit UIPtrArray(int preAllocSize=0);
    UIPtrArray(const UIPtrArray &src);
    ~UIPtrArray();

    void    Empty();
    void    Resize(int size);
    bool    IsEmpty()const;
    int     Find(LPVOID data)const;
    bool    Add(LPVOID data);
    bool    SetAt(int index, LPVOID data);
    bool    InsertAt(int index, LPVOID data);
    bool    Remove(int index, int count=1);
    int     GetSize()const;
    LPVOID* GetData();

    LPVOID  GetAt(int index)const;
    LPVOID  operator[](int index)const;
private:
    LPVOID  *m_ppVoid;
    int     m_nCount;
    int     m_nAllocated;
};

#endif //DIRECTUI_UIPTRARRAY_H