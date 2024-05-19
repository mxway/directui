#ifndef DIRECTUI_UIVALARRAY_H
#define DIRECTUI_UIVALARRAY_H
#include <UIDefine.h>

class UIValArray
{
public:
    explicit UIValArray(int elementSize, int preAllocSize=0);
    ~UIValArray();

    void     Empty();
    bool     IsEmpty();
    bool     Add(LPVOID data);
    bool     Remove(int index, int count=1);
    int     GetSize()const;
    LPVOID  GetData();

    LPVOID  GetAt(int index)const;
    LPVOID  operator[](int index)const;
private:
    LPBYTE m_pVoid;
    int    m_elementSize;
    int    m_nCount;
    int    m_nAllocated;
};

#endif //DIRECTUI_UIVALARRAY_H