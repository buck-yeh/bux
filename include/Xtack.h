#ifndef __Xtack_H
#define __Xtack_H

#include <iterator> // std::iterator_traits<>, std::distance()
#include <ostream>  // std::basic_ostream<>
#include <string>   // std::char_traits<>

namespace bux {

//
//      Types
//
/*! \brief
    Any given type is safely instantiated even though C_DtorFreeStack<T>
    does better for bytewize types.
*/
template<class T>
class C_StackBase
{
public:

    // Types
    typedef size_t          size_type;
    typedef T               value_type;
    typedef T               *iterator;
    typedef const T         *const_iterator;

    // Nonvirtuals
    explicit C_StackBase(size_t minAlloc =4) noexcept;
    ~C_StackBase();
    C_StackBase(const C_StackBase &) = delete;

    // Nonvirtuals
    auto &operator[](size_type i) noexcept
        { return m_RawSpace[i]; }
    auto &operator[](size_type i) const noexcept
        { return m_RawSpace[i]; }
    auto &operator=(const C_StackBase &other)
        { assign(other.begin(), other.end()); return *this; }
#if !defined(_MSC_VER) || _MSC_VER >= 1300
    template<class T_Iter> void assign(T_Iter beg, T_Iter end)
#else
    void assign(const T *beg, const T *end)
#endif
        { clear(); push(beg, end); }
    auto begin() const noexcept
        { return m_RawSpace; }
    auto begin() noexcept
        { return m_RawSpace; }
    auto capacity() const
        { return m_ArrLimit; }
    void clear();
        //
    bool empty() const noexcept
        { return !m_ArrSize; }
    auto end() const noexcept
        { return m_RawSpace + m_ArrSize; }
    auto end() noexcept
        { return m_RawSpace + m_ArrSize; }
    void pop(size_t n =1);
        //
    auto &push()
        { return *new(pushRaw()) T; }
#if !defined(_MSC_VER) || _MSC_VER >= 1300
    template<class T_Iter> void push(T_Iter beg, T_Iter end);
#else
    void push(const T *beg, const T *end);
#endif
    auto size() const noexcept
        { return m_ArrSize; }
    auto &top()
        { return m_RawSpace[m_ArrSize-1]; }
    auto &top() const
        { return m_RawSpace[m_ArrSize-1]; }

protected:

    // Nonvirtuals
    auto copyMinAlloc() const noexcept
        { return m_ArrSize? m_ArrSize: m_MinAlloc; }
    T *pushRaw();
        //

private:

    // Data
    T                       *m_RawSpace;    // owned
    size_t                  m_ArrSize;      // counted in type T
    size_t                  m_ArrLimit;     // counted in type T
    const size_t            m_MinAlloc;
};

/*! \brief A much more succinct substitute of std::stack<std::vector<T> >
*/
template<class T>
class C_Stack: public C_StackBase<T>
{
    typedef C_StackBase<T> C_Super;

public:

    // Nonvirtuals
    explicit C_Stack(size_t minAlloc =4) noexcept: C_Super(minAlloc)
        {}
    C_Stack(const C_Stack &other): C_Super(other.copyMinAlloc())
        { push(other.begin(), other.end()); }
#if !defined(_MSC_VER) || _MSC_VER >= 1300
    template<class T_Iter> C_Stack(T_Iter beg, T_Iter end): C_Super(1)
#else
    C_Stack(const T *beg, const T *end): C_Super(1)
#endif
        { push(beg, end); }
    C_Stack &operator=(const C_Stack &other)
        { this->assign(other.begin(), other.end()); return *this; }
    auto &push()
        { return C_Super::push(); }
#if !defined(_MSC_VER) || _MSC_VER >= 1300
    template<class T_Elem> auto &push(const T_Elem &t)
#else
    auto &push(const T &t)
#endif
        { return *new(C_Super::pushRaw()) T(t); }
#if !defined(_MSC_VER) || _MSC_VER >= 1300
    template<class T_Iter> void push(T_Iter beg, T_Iter end)
#else
    void push(const T *beg, const T *end)
#endif
        { C_Super::push(beg, end); }
#if !defined(_MSC_VER) || _MSC_VER >= 1300
#else
    void push(const T *beg, size_t n)
        { C_Super::push(beg, beg+n); }
#endif
};

/*! \brief
    If ctor of type T properly handles ownership transfer,
    so does C_ResourceStack<T>
*/
template<class T>
class C_ResourceStack: public C_StackBase<T>
{
    typedef C_StackBase<T> C_Super;

public:

    // Nonvirtuals
    explicit C_ResourceStack(size_t minAlloc =4) noexcept: C_Super(minAlloc)
        {}
    explicit C_ResourceStack(C_ResourceStack &other): C_Super(other.copyMinAlloc())
        { push(other.begin(), other.end()); }
#if !defined(_MSC_VER) || _MSC_VER >= 1300
    template<class T_Iter> C_ResourceStack(T_Iter beg, T_Iter end): C_Super(1)
#else
    C_ResourceStack(const T *beg, const T *end): C_Super(1)
#endif
        { push(beg, end); }
    auto &operator=(C_ResourceStack &other)
        { assign(other.begin(), other.end()); return *this; }
    auto &push()
        { T t; return push(t); }
#if !defined(_MSC_VER) || _MSC_VER >= 1300
    template<class T_Elem> auto &push(T_Elem &t)
#else
    auto &push(T &t)
#endif
        { return *new(C_Super::pushRaw()) T(t); }
#if !defined(_MSC_VER) || _MSC_VER >= 1300
    template<class T_Iter> void push(T_Iter beg, T_Iter end)
#else
    void push(const T *beg, const T *end)
#endif
        { C_Super::push(beg, end); }
};

/*! \brief
    Safe instantiation for types which's destructor does exactly nothing.
*/
template<class T>
class C_DtorFreeStack
{
public:

    // Types
    typedef size_t          size_type;
    typedef T               value_type;
    typedef T               *iterator;
    typedef const T         *const_iterator;

    // Nonvirtuals
    C_DtorFreeStack() noexcept: m_RawSpace(0), m_ArrSize(0), m_ArrLimit(0)
        {}
    ~C_DtorFreeStack() noexcept
        { delete[] m_RawSpace; }
    auto &operator=(const C_DtorFreeStack &other)
        { assign(other.begin(), other.size()); return *this; }
    auto &operator[](size_type i)
        { return m_RawSpace[i]; }
    auto &operator[](size_type i) const
        { return m_RawSpace[i]; }
#if !defined(_MSC_VER) || _MSC_VER >= 1300
    template<class T_Elem> void assign(const T_Elem *p, size_t n)
#else
    void assign(const T *p, size_t n)
#endif
        { clear(); push(p, n); }
#if !defined(_MSC_VER) || _MSC_VER >= 1300
    template<class T_Iter> void assign(T_Iter beg, T_Iter end)
#else
    void assign(const T *beg, const T *end)
#endif
        { clear(); push(beg, end); }
    auto begin() const noexcept
        { return m_RawSpace; }
    auto begin() noexcept
        { return m_RawSpace; }
    void clear() noexcept
        { m_ArrSize =0; }
    bool empty() const noexcept
        { return !m_ArrSize; }
    auto end() const noexcept
        { return m_RawSpace + m_ArrSize; }
    auto end() noexcept
        { return m_RawSpace + m_ArrSize; }
    void pop(size_t n =1);
        //
    auto &push()
        { return *new(pushRaw(1)) T; }
#if !defined(_MSC_VER) || _MSC_VER >= 1300
    template<class T_Elem> auto &push(const T_Elem &t)
#else
    auto &push(const T &t)
#endif
        { return *new(pushRaw(1)) T(t); }
#if !defined(_MSC_VER) || _MSC_VER >= 1300
    template<class T_Iter> void push(T_Iter p, size_t n);
#else
    void push(const T *p, size_t n);
#endif
        //
#if !defined(_MSC_VER) || _MSC_VER >= 1300
    template<class T_Iter> void push(T_Iter beg, T_Iter end)
        { pushByTraits(beg, end, typename std::iterator_traits<T_Iter>::iterator_category()); }
#else
    void push(const T *beg, const T *end)
        { push(beg, end-beg); }
#endif
    auto size() const noexcept
        { return m_ArrSize; }
    auto &top() const noexcept
        { return m_RawSpace[m_ArrSize-1]; }
    auto &top() noexcept
        { return m_RawSpace[m_ArrSize-1]; }

private:

    // Data
    T                       *m_RawSpace;    ///< owned
    size_t                  m_ArrSize;      ///< counted in type T
    size_t                  m_ArrLimit;     ///< counted in type T

    // Nonvirtuals
#if !defined(_MSC_VER) || _MSC_VER >= 1300
    template<class T_Iter> inline void pushByTraits(T_Iter beg, T_Iter end,
        const std::random_access_iterator_tag&)
        { push(beg, end-beg); }
    template<class T_Iter> inline void pushByTraits(T_Iter beg, T_Iter end,
        const std::input_iterator_tag&)
        { push(beg, std::distance(beg,end)); }
#endif
    T *pushRaw(size_t n);
        ///< Return start point of raw memory
};

//
//      Function Templates
//
/*! \brief Lexical comparison
*/
template<class T>
bool operator <(const C_Stack<T> &a, const C_Stack<T> &b) noexcept
{
    size_t                  n = a.size();
    const bool              shorterA = n < b.size();
    if (!shorterA)
        n = b.size();

    if (const int cmp =std::char_traits<T>::compare(a.begin(), b.begin(), n))
        return cmp < 0;

    return shorterA;
}

template<class T>
bool operator ==(const C_Stack<T> &a, const C_Stack<T> &b) noexcept
{
    return a.size() == b.size() &&
           !std::char_traits<T>::compare(a.begin(), b.begin(), a.size());
}

template<class T>
bool operator !=(const C_Stack<T> &a, const C_Stack<T> &b) noexcept
{
    return !(a == b);
}

template<class T>
void operator +=(C_Stack<T> &a, const C_Stack<T> &b)
{
    a.push(b.begin(), b.end());
}

template<class T>
void operator +=(C_Stack<T> &out, const T *s)
{
    out.push(s, std::char_traits<T>::length(s));
}

template<class T>
void operator +=(C_ResourceStack<T> &a, C_StackBase<T> &b)
{
    a.push(b.begin(), b.end());
}

template<class T>
auto &operator <<(C_Stack<T> &a, const C_Stack<T> &b)
{
    a.push(b.begin(), b.size());
    return a;
}

template<class T>
auto &operator <<(C_Stack<T> &out, const T *s)
{
    out.push(s, std::char_traits<T>::length(s));
    return out;
}

template<class T>
auto &operator <<(C_ResourceStack<T> &a, C_StackBase<T> &b)
{
    a.push(b.begin(), b.end());
    return a;
}

template<class T>
auto &operator <<(C_Stack<T> &out, const T &c)
{
    out.push(c);
    return out;
}

template<class T>
auto &operator <<(C_ResourceStack<T> &out, T &t)
{
    out.push(t);
    return out;
}

template<class T>
auto &operator <<(std::basic_ostream<T> &out, const C_Stack<T> &s)
{
    if (!s.empty())
        out.write(s.begin(), s.size());
    return out;
}

//
//      Implementation of Class Templates
//
template<class T>
C_StackBase<T>::C_StackBase(size_t minAlloc) noexcept:
    m_RawSpace(0), m_ArrSize(0), m_ArrLimit(0), m_MinAlloc(minAlloc)
{
}

template<class T>
C_StackBase<T>::~C_StackBase()
{
    clear();
    delete[] reinterpret_cast<char*>(m_RawSpace);
}

template<class T>
void C_StackBase<T>::clear()
{
    while (!empty())
        pop();
}

template<class T>
void C_StackBase<T>::pop(size_t n)
{
    while (m_ArrSize > 0 && n-- > 0)
#if __BORLANDC__ >= 0x530
        m_RawSpace[--m_ArrSize].~T();
#else
        m_RawSpace[--m_ArrSize].T::~T();
#endif
}

template<class T>
#if !defined(_MSC_VER) || _MSC_VER >= 1300
template<class T_Iter> void C_StackBase<T>::push(T_Iter beg, T_Iter end)
#else
void C_StackBase<T>::push(const T *beg, const T *end)
#endif
{
    while (beg != end)
    {
        new(pushRaw()) T(*beg);
        ++beg;
    }
}

template<class T>
T *C_StackBase<T>::pushRaw()
{
    if (m_ArrSize >= m_ArrLimit)
    {
        const size_t limit =m_ArrLimit? m_ArrLimit *2: m_MinAlloc;
        T *const rebuf =reinterpret_cast<T*>(new char[limit*sizeof(T[1])]);
        size_t i;
        for (i =0; i < m_ArrSize; ++i)
            new(rebuf+i) T(m_RawSpace[i]);

        clear();
        delete[] reinterpret_cast<char*>(m_RawSpace);
        m_RawSpace =rebuf;
        m_ArrSize  =i;        // m_ArrSize is reset by clear()
        m_ArrLimit =limit;
    }
    return m_RawSpace +m_ArrSize++;
}

template<class T>
void C_DtorFreeStack<T>::pop(size_t n)
{
    if (m_ArrSize > n)
        m_ArrSize -=n;
    else
        m_ArrSize =0;
}

template<class T>
#if !defined(_MSC_VER) || _MSC_VER >= 1300
template<class T_Iter> void C_DtorFreeStack<T>::push(T_Iter src, size_t n)
#else
void C_DtorFreeStack<T>::push(const T *src, size_t n)
#endif
{
    for (T *dst =pushRaw(n); n--; ++src)
        new(dst++) T(*src);
}

template<class T>
T *C_DtorFreeStack<T>::pushRaw(size_t n)
{
    if (m_ArrSize +n > m_ArrLimit)
    {
        size_t limit =m_ArrLimit? m_ArrLimit: (15 +sizeof(T)) /sizeof(T);
        while (m_ArrSize +n > limit)
            limit <<=1;

        T *const rebuf =new T[limit];
        for (size_t i =0; i < m_ArrSize; ++i)
            new(rebuf+i) T(m_RawSpace[i]);
        delete[] m_RawSpace;
        m_RawSpace =rebuf;
        m_ArrLimit =limit;
    }
    T *const ret =m_RawSpace +m_ArrSize;
    m_ArrSize +=n;
    return ret;
}

} //namespace bux

#ifndef _MSC_VER
#   define SPECIALIZE_DTORFREESTACK(classX,X)                           \
    namespace bux {                                                     \
    template<classX> struct C_Stack<X>: C_DtorFreeStack<X>              \
    {                                                                   \
        C_Stack()                                                       \
            {}                                                          \
        C_Stack(const C_Stack &other)                                   \
            { push(other.begin(), other.size()); }                      \
        template<class T_Elem> C_Stack(T_Elem const *p)                 \
            { push(p, std::char_traits<T_Elem>::length(p)); }           \
        template<class T_Elem> C_Stack(T_Elem const *p, size_t n)       \
            { push(p, n); }                                             \
        template<class T_Iter> C_Stack(T_Iter beg, T_Iter end)          \
            { push(beg, end); }                                         \
        C_Stack &operator=(const C_Stack &other)                        \
            { assign(other.begin(), other.size()); return *this; }      \
        C_Stack &operator=(X const *s)                                  \
            { assign(s, std::char_traits<X>::length(s)); return *this; }\
    }; }

// C++ Standard Builins
SPECIALIZE_DTORFREESTACK(,bool)
SPECIALIZE_DTORFREESTACK(,char)
SPECIALIZE_DTORFREESTACK(,unsigned char)
SPECIALIZE_DTORFREESTACK(,signed char)
SPECIALIZE_DTORFREESTACK(,wchar_t)
SPECIALIZE_DTORFREESTACK(,short)
SPECIALIZE_DTORFREESTACK(,unsigned short)
SPECIALIZE_DTORFREESTACK(,int)
SPECIALIZE_DTORFREESTACK(,unsigned int)
SPECIALIZE_DTORFREESTACK(,long)
SPECIALIZE_DTORFREESTACK(,unsigned long)
SPECIALIZE_DTORFREESTACK(class T,T*)

#endif // #ifndef _MSC_VER

#endif // __Xtack_H
