#ifndef __XAutoPtr_H
#define __XAutoPtr_H

#include <new>          // placement new
#include <utility>      // std::forward<>

namespace bux {

//
//      Types
//
template<class T>
class C_AutoNode
{
public:

    // Nonvirtuals
    constexpr C_AutoNode() noexcept: m_Ptr(nullptr), m_Owned(false) {}
    constexpr C_AutoNode(T *p, bool owned) noexcept: m_Ptr(p), m_Owned(owned) {}
    explicit C_AutoNode(C_AutoNode &another) noexcept:
        m_Ptr(another.m_Ptr), m_Owned(another.m_Owned) { another.m_Owned = false; }
    template<class T2>
    explicit C_AutoNode(C_AutoNode<T2> &another) noexcept:
        m_Ptr(static_cast<T*>(another.m_Ptr)), m_Owned(another.m_Owned) { another.m_Owned = false; }
    ~C_AutoNode();
    void operator=(C_AutoNode &another);
    template<class T2>
    void operator=(C_AutoNode<T2> &another);
    operator bool() const   noexcept { return m_Ptr != nullptr; }
    template<class T2>
    operator T2*() const    noexcept { return dynamic_cast<T2*>(m_Ptr); }
    T *operator->() const   noexcept { return m_Ptr; }
    T **operator&()         noexcept { assign(nullptr,true); return &m_Ptr; }
    T &operator*() const    noexcept { return *m_Ptr; }
    void assign(T *ptr, bool owned);
    void reset(T *ptr)      noexcept { assign(ptr, true); }
    void clear()            noexcept { assign(nullptr,false); }
    T *disown() noexcept;
    T *get() const          noexcept { return m_Ptr; }
    bool owned() const      noexcept { return m_Owned; }
    void swap(C_AutoNode &another) noexcept ;
    template<class T2>
    bool takeOver(C_AutoNode<T2> &another);

private:

    // Data
    T *m_Ptr;
    bool m_Owned;

    // Friend of my kind
    template<class T2> friend class C_AutoNode;
};

struct C_Void {};

template<class T>
class C_NewNode: public C_AutoNode<T>
{
protected:
    constexpr C_NewNode(C_Void) noexcept: C_AutoNode<T>(nullptr,false) {}

public:
    C_NewNode(): C_AutoNode<T>(new T, true) {}
    template<class...T_Args>
    C_NewNode(T_Args&&...args): C_AutoNode<T>(new T{std::forward<T_Args>(args)...}, true) {}
};

//
//      Implementation of Class Templates
//
template<class T>
C_AutoNode<T>::~C_AutoNode()
{
    if (m_Owned)
        delete m_Ptr;
}

template<class T>
void C_AutoNode<T>::operator=(C_AutoNode &another)
{
    this->~C_AutoNode<T>();
    new(this)C_AutoNode(another);
}

template<class T>
template<class T2>
void C_AutoNode<T>::operator=(C_AutoNode<T2> &another)
{
    this->~C_AutoNode<T>();
    new(this)C_AutoNode(another);
}

template<class T>
void C_AutoNode<T>::assign(T *ptr, bool owned)
{
    this->~C_AutoNode<T>();
    new(this)C_AutoNode(ptr, owned);
}

template<class T>
T *C_AutoNode<T>::disown() noexcept
{
    if (m_Owned)
    {
        m_Owned = false;
        return m_Ptr;
    }
    return nullptr;
}

template<class T>
void C_AutoNode<T>::swap(C_AutoNode &another) noexcept
{
    const auto p = m_Ptr;
    m_Ptr = another.m_Ptr;
    another.m_Ptr = p;
    //------------------------
    const auto y = m_Owned;
    m_Owned = another.m_Owned;
    another.m_Owned = y;
}

template<class T>
template<class T2>
bool C_AutoNode<T>::takeOver(C_AutoNode<T2> &another)
{
    if (auto ptr = dynamic_cast<T*>(another.get()))
    {
        assign(ptr, another.m_Owned);
        another.m_Owned = false;
        return true;
    }
    return false;
}

} // namespace bux

#endif // __XAutoPtr_H
