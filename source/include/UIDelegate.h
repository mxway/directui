#ifndef DIRECTUI_UIDELEGATE_H
#define DIRECTUI_UIDELEGATE_H
#include <UIPtrArray.h>

class CDelegateBase {
public:
    CDelegateBase(void *pObject, void *pFn);

    CDelegateBase(const CDelegateBase &rhs);

    virtual ~CDelegateBase();

    bool Equals(const CDelegateBase &rhs) const;

    bool operator()(void *param);

    virtual CDelegateBase *Copy() const = 0; // add const for gcc

protected:
    void *GetFn();

    void *GetObject();

    virtual bool Invoke(void *param) = 0;

private:
    void *m_pObject;
    void *m_pFn;
};

class CDelegateStatic: public CDelegateBase
{
    typedef bool (*Fn)(void*);
public:
    explicit CDelegateStatic(Fn pFn) : CDelegateBase((void*)nullptr, (void*)pFn) { }
    CDelegateStatic(const CDelegateStatic& rhs)  = default;
    CDelegateBase* Copy() const override { return new CDelegateStatic(*this); }

protected:
    bool Invoke(void* param) override
    {
        Fn pFn = (Fn)GetFn();
        return (*pFn)(param);
    }
};

template <class O, class T>
class CDelegate : public CDelegateBase
{
    typedef bool (T::* Fn)(void*);
public:
    CDelegate(O* pObj, Fn pFn) : CDelegateBase(pObj, *(void**)&pFn) { }
    CDelegate(const CDelegate& rhs) : CDelegateBase(rhs) { }
    CDelegateBase* Copy() const override { return new CDelegate(*this); }

protected:
    bool Invoke(void* param) override
    {
        O* pObject = (O*) GetObject();
        union
        {
            void* ptr;
            Fn fn;
        } func = { GetFn() };
        return (pObject->*func.fn)(param);
    }

private:
    Fn m_pFn;
};

template <class O, class T>
CDelegate<O, T> MakeDelegate(O* pObject, bool (T::* pFn)(void*))
{
    return CDelegate<O, T>(pObject, pFn);
}

inline CDelegateStatic MakeDelegate(bool (*pFn)(void*))
{
    return CDelegateStatic(pFn);
}

class CEventSource {
    typedef bool (*FnType)(void *);

public:
    ~CEventSource();

    explicit operator bool();

    void operator+=(const CDelegateBase &d); // add const for gcc
    void operator+=(FnType pFn);

    void operator-=(const CDelegateBase &d);

    void operator-=(FnType pFn);

    bool operator()(void *param);

protected:
    UIPtrArray m_aDelegates;
};

#endif //DIRECTUI_UIDELEGATE_H