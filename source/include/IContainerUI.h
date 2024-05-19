#ifndef DIRECTUI_ICONTAINERUI_H
#define DIRECTUI_ICONTAINERUI_H
#include <UIControl.h>

class IContainerUI
{
public:
    virtual ~IContainerUI()=default;
    virtual UIControl* GetItemAt(int iIndex) const = 0;
    virtual int GetItemIndex(UIControl* pControl) const  = 0;
    virtual bool SetItemIndex(UIControl* pControl, int iNewIndex) = 0;
    virtual bool SetMultiItemIndex(UIControl* pStartControl, int iCount, int iNewStartIndex) = 0;
    virtual int GetCount() const = 0;
    virtual bool Add(UIControl* pControl) = 0;
    virtual bool AddAt(UIControl* pControl, int iIndex)  = 0;
    virtual bool Remove(UIControl* pControl, bool bDoNotDestroy=false) = 0;
    virtual bool RemoveAt(int iIndex, bool bDoNotDestroy=false)  = 0;
    virtual void RemoveAll() = 0;
};

#endif //DIRECTUI_ICONTAINERUI_H