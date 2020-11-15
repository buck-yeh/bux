#ifndef XQueH
#define XQueH

#include <cstddef>  // size_t
#include <utility>  // std::swap()

namespace bux {

//
//      Types
//
template<size_t N>
class C_RawQueue
{
public:

    // Types
    typedef size_t size_type;

    // Nonvirtuals
    C_RawQueue() = default;
    C_RawQueue(const C_RawQueue &another) = delete;
    C_RawQueue &operator=(const C_RawQueue &another) = delete;
    ~C_RawQueue()
    {
        clearGarbage(m_pFront); // Expected to clear first from its derived type
    }
    size_t compact();
    bool empty() const { return !m_pBack; }
    void splice(C_RawQueue &other);
    void swap(C_RawQueue &other);

protected:

    // Nonvirtuals
    char *backRaw() const { return m_pBack->m_Datum; }
    char *frontRaw() const { return m_pFront->m_Datum; }
    void popRaw();
    char *pushRaw();

private:

    // Types
    struct C_Link
    {
        C_Link              *m_Next{};
        char                m_Datum[N];
    };

    // Data
    C_Link                  *m_pFront{};    ///< The first node
    C_Link                  *m_pBack{};     ///< The last node or null to mean empty queue

    // Nonvirtuals
    static size_t clearGarbage(C_Link *p);
};

template<class T>
class C_Queue: public C_RawQueue<sizeof(T)>
/*! A much more succinct substitute for std::queue\<std::deque\<T\> \> or
    std::queue\<std::list\<T\> \>

    Types and methods exposed as public members are compilant to container
    requirements as much as possible although a queue in the standard C++ way is
    merely a container adaptor.

    This class is not thread-safe.
*/
{
public:

    // Types
    typedef T value_type;

    // Nonvirtuals
    C_Queue() = default;
    ~C_Queue() { clear(); }
    T &back() const { return *reinterpret_cast<T*>(this->backRaw()); }
    void clear();
    T &front() const { return *reinterpret_cast<T*>(this->frontRaw()); }
    void pop();
    T &push() { return *new(this->pushRaw()) T; }
    template<class...T_Args>
    T &push(T_Args&&...args) { return *new(this->pushRaw()) T{std::forward<T_Args>(args)...}; }
};

//
//      Implement Class Templates
//
template<size_t N>
size_t C_RawQueue<N>::clearGarbage(C_Link *p)
{
    size_t ret =0;
    while (p)
    {
        C_Link *const t =p;
        p =p->m_Next;
        delete t;
        ++ret;
    }
    return ret;
}

template<size_t N>
size_t C_RawQueue<N>::compact()
/*! \return Count of deletions of recycled nodes

    Unlink all of recycled nodes.
*/
{
    size_t ret;
    if (m_pBack)
        // Not empty
    {
        ret =clearGarbage(m_pBack->m_Next);
        m_pBack->m_Next =0;
    }
    else
        // Empty queue
    {
        ret =clearGarbage(m_pFront);
        m_pFront =0;
    }
    return ret;
}

template<size_t N>
void C_RawQueue<N>::popRaw()
{
    if (m_pFront == m_pBack)
        // Become empty
        m_pBack =0;
    else
        // Remain non-empty
    {
        C_Link *const t =m_pFront;
        m_pFront =t->m_Next;
        t->m_Next =m_pBack->m_Next;
        m_pBack->m_Next =t;
    }
}

template<size_t N>
char *C_RawQueue<N>::pushRaw()
{
    if (m_pBack)
        // Not empty
    {
        if (m_pBack->m_Next)
            m_pBack =m_pBack->m_Next;
        else
            m_pBack =m_pBack->m_Next =new C_Link;
    }
    else if (m_pFront)
        // Empty but garbage link is not empty
    {
        m_pBack =m_pFront;
    }
    else
        // Both empty
    {
        m_pFront =
        m_pBack =new C_Link;
    }
    return m_pBack->m_Datum;
}

template<size_t N>
void C_RawQueue<N>::splice(C_RawQueue &other)
/*! \param [in,out] other The queue the link of which are moves and appended this one.

    Concatenate the internal queue of \em other after this queue. The other
    queue become empty.

    In most cases the \em other queue will carry no recycled node, so constant
    complexity is expected.
*/
{
    if (this == &other)
        // Trivial splice
        return;

    C_Link *recycled;
    if (m_pBack)
    {
        recycled =m_pBack->m_Next;
        m_pBack->m_Next =other.m_pFront;
        if (other.m_pBack)
            m_pBack =other.m_pBack;
    }
    else
    {
        recycled =m_pFront;
        m_pFront =other.m_pFront;
        m_pBack =other.m_pBack;
    }
    other.m_pFront =
    other.m_pBack =0;

    if (recycled)
    {
        C_Link *last =m_pBack;
        if (!last)
            last =m_pFront;

        if (last)
            // Find the last node and append
        {
            while (last->m_Next)
                last =last->m_Next;

            last->m_Next =recycled;
        }
        else
            // null m_pFront
            m_pFront =recycled;
    }
}

template<size_t N>
void C_RawQueue<N>::swap(C_RawQueue &other)
/*! \param other The other queue to swap with
*/
{
    std::swap(m_pFront, other.m_pFront);
    std::swap(m_pBack, other.m_pBack);
}

template<class T>
void C_Queue<T>::clear()
/*! Erase all elements
*/
{
    while (!this->empty())
        pop();
}

template<class T>
void C_Queue<T>::pop()
/*! \pre empty() == false (i.e. m_pBack != 0)

    Erase one element from the front end
*/
{
    reinterpret_cast<T*>(this->frontRaw())->~T();
    this->popRaw();
}

} // namespace bux

#endif // XQueH
