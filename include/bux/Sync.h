#ifndef bux_Sync_H_
#define bux_Sync_H_

#include <iosfwd>   // Forwarded std::ostream

#if defined(_MSC_VER) && _MSC_VER < 1300
#   pragma warning(disable:4786)
#   pragma warning(disable:4250)
#endif

namespace bux {

//
//      Types
//
template<class T> class C_UseResource;

template<class T>
class I_SyncResource
/*! \brief Interface of all synchronous resource classes.

    Applying bux::C_UseResource<T> "is supposed to" block any other thread
    from using it. Derived classes are responsible for making this happen.
*/
{
public:

    // Virtuals
    virtual ~I_SyncResource() =default;
    virtual T getResource() =0;
    virtual void releaseResource(T res) =0;
};
typedef I_SyncResource<std::ostream&> I_SyncOstream;

template<class T>
class C_UseResource
/*! Helper class to use a resource in the current thread and block
    any other thread from using it in the mean time.
*/
{
public:

    // Nonvirtuals
    C_UseResource(const C_UseResource&) = delete;
    C_UseResource &operator=(const C_UseResource&) = delete;
    C_UseResource(I_SyncResource<T> &obj): m_Obj(obj), m_Resource(obj.getResource())
        {}
    ~C_UseResource()
        { m_Obj.releaseResource(m_Resource); }
    operator T() const
        { return m_Resource; }

protected:

    // Data
    I_SyncResource<T>       &m_Obj;
    T                       m_Resource;
};

class C_LockTillEnd;

class I_Lockable
{
protected:

    // Virtuals
    virtual ~I_Lockable() =default;
    virtual bool lock() =0;
    virtual void unlock() =0;

    // Friends
    friend class C_LockTillEnd;
};

class C_LockTillEnd
{
public:

    // Data
    I_Lockable  &m_Obj;

    // Ctor/Dtor
    C_LockTillEnd(const C_LockTillEnd&) = delete;
    C_LockTillEnd &operator=(const C_LockTillEnd&) = delete;
    C_LockTillEnd(I_Lockable &obj);
    ~C_LockTillEnd();

    // Nonvirtuals
    operator bool() const
        { return m_Locked; }
    bool locked() const
        { return m_Locked; }
    void unlock();
        //

private:

    // Data
    bool        m_Locked;
};

class C_RefObject: virtual public I_Lockable
/*  Add extra power to I_StrSender
*/
{
public:

    // Nonvirtuals
    C_RefObject(const C_RefObject&) = delete;
    C_RefObject &operator=(const C_RefObject&) = delete;
    C_RefObject(size_t initRefCount =0);
    void decRef();
    bool incRef();
    void setZeroRefHandler(void (*pOnZeroRef)(void*), void *pData);

    // Pure virtuals
    virtual void destroyASAP() =0;

protected:

    // Nonvirtuals (No lock protection)
    ~C_RefObject() {}
        ///< \brief To ban pointer deletion
    void disableIncRef() { m_IncRefEnabled =false; }
        ///<
    auto refCount() const { return m_RefCount; }
        ///< Current refcount

    // Virtuals
    virtual void onNonzeroRef() =0;
    virtual void onZeroRef() =0;

private:

    // Data
    void                    (*m_pOnZeroRef)(void*);
    void                    *m_pZeroRefData;
    size_t                  m_RefCount;
    bool                    m_IncRefEnabled;    ///< oneway( true->false )
};

template<class T =C_RefObject>
class C_RefTillEnd
{
public:

    // Data
    T   &m_Obj;

    // Ctor/dtor
    C_RefTillEnd(const C_RefTillEnd&) = delete;
    C_RefTillEnd &operator=(const C_RefTillEnd&) = delete;
    C_RefTillEnd(T &obj);
    ~C_RefTillEnd();
    operator T*() const;

private:

    // Data
    C_RefObject *m_Referenced;
};

//
//      Implement Class Templates
//
template<class T>
C_RefTillEnd<T>::C_RefTillEnd(T &obj):
    m_Obj(obj),
    m_Referenced(dynamic_cast<C_RefObject*>(&obj))
{
    if (m_Referenced && !m_Referenced->incRef())
        m_Referenced =0;
}

template<class T>
C_RefTillEnd<T>::~C_RefTillEnd()
{
    if (m_Referenced)
        m_Referenced->decRef();
}

template<class T>
C_RefTillEnd<T>::operator T*() const
{
    return m_Referenced? &m_Obj: static_cast<T*>(0);
}

} //namespace bux

#endif  // bux_Sync_H_
