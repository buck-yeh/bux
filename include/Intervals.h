#ifndef IntervalsH
#define IntervalsH

#include "XException.h" // RUNTIME_ERROR()
#include <limits>       // std::numeric_limits<>
#include <vector>       // std::vector<>
#include <type_traits>  // std::is_integral<>, std::remove_reference<>, std::is_pointer<>, std::is_class<>
#include <ostream>      // std::basic_ostream<>
#include <iterator>     // std::iterator_traits<>

namespace bux {

//
//      Types
//
template<class T, bool IS_INTEGER>
class C_IntervalsByTraits;

template<class T>
using C_Intervals = C_IntervalsByTraits<T, std::is_integral<T>::value>;

template<class T>
class C_IntervalsByTraits<T,true>
{
public:

    // Types
    typedef std::pair<T,T> value_type;
    typedef typename std::vector<value_type>::const_iterator const_iterator;

    // Ctors
    C_IntervalsByTraits() {}
    C_IntervalsByTraits(const C_IntervalsByTraits &other): m_Intervals(other.m_Intervals) {}
    C_IntervalsByTraits(C_IntervalsByTraits &&other) { swap(other); }
    C_IntervalsByTraits(T id);
    C_IntervalsByTraits(value_type i);
    template<class I> C_IntervalsByTraits(I start, I end);

    // Nonvirtuals - Assignments
    void operator=(const C_IntervalsByTraits &other)   { m_Intervals =other.m_Intervals; }
    void operator=(C_IntervalsByTraits &&other)        { swap(other); }
    void operator|=(const C_IntervalsByTraits &other);
    void operator&=(const C_IntervalsByTraits &other);
    void operator-=(const C_IntervalsByTraits &other);
    bool operator==(const C_IntervalsByTraits &other) const { return m_Intervals == other.m_Intervals; }

    // Nonvirtuals - Immutables
    const_iterator begin() const    { return m_Intervals.begin(); }
    const_iterator end() const      { return m_Intervals.end(); }
    bool empty() const              { return m_Intervals.empty(); }
    size_t size() const             { return m_Intervals.size(); }

    // Nonvirtuals - Mutables
    void complement();
    void swap(C_IntervalsByTraits &other) { m_Intervals.swap(other.m_Intervals); }

private:

    // Data
    std::vector<value_type> m_Intervals;
};

//
//      Function Templates
//
template<class T, class charT, class traits=std::char_traits<charT>>
std::basic_ostream<charT,traits> &
operator<<(std::basic_ostream<charT,traits> &out, const C_IntervalsByTraits<T,true> &x)
{
    bool first =true;
    for (auto i: x)
    {
        out <<(first?"{[":", [") <<i.first <<',' <<i.second <<']';
        first =false;
    }
    if (first)
        out <<'{';

    return out <<'}';
}

//
//      Implement Class Templates
//
template<class T>
C_IntervalsByTraits<T,true>::C_IntervalsByTraits(T id)
{
    m_Intervals.emplace_back(id, id);
}

template<class T>
C_IntervalsByTraits<T,true>::C_IntervalsByTraits(value_type i)
{
    if (i.first > i.second)
        RUNTIME_ERROR('(' <<i.first <<',' <<i.second <<')')

    m_Intervals.emplace_back(i);
}

template<class I, bool CAN_BE_ITERATOR>
struct IsIteratorImpl
{
    static const bool value = false;
};

template<class I>
using IsIterator =IsIteratorImpl<typename std::remove_reference<I>::type,
    std::is_pointer<typename std::remove_reference<I>::type>::value||
    std::is_class<typename std::remove_reference<I>::type>::value>;

template<class I>
struct IsIteratorImpl<I,true>
{
    static const bool value = std::is_base_of<std::input_iterator_tag,
        typename std::iterator_traits<I>::iterator_category>::value;
};

template<class I, bool IS_ITERATOR>
struct C_SmartAddIntervalImpl;

template<class I>
using C_SmartAddInterval =C_SmartAddIntervalImpl<I, IsIterator<I>::value>;

template<class I>
struct C_SmartAddIntervalImpl<I,true>
{
    template<class C_DstType>
    static void add(C_DstType &dst, I start, I end)
    {
        while (start != end)
            dst |= C_DstType(*start++);
    }
};

template<class I>
struct C_SmartAddIntervalImpl<I,false>
{
    template<class C_DstType>
    static void add(C_DstType &dst, I start, I end)
    {
        dst |= C_DstType(typename C_DstType::value_type(start, end));
    }
};

template<class T>
template<class I>
C_IntervalsByTraits<T,true>::C_IntervalsByTraits(I start, I end)
{
    C_SmartAddInterval<I>::add(*this, start, end);
}

template<class T>
void C_IntervalsByTraits<T,true>::operator|=(const C_IntervalsByTraits<T,true> &other)
{
    decltype(m_Intervals) dst;
    const_iterator
         ia =other.m_Intervals.begin(), ea =other.m_Intervals.end(),
         ib =m_Intervals.begin(), eb =m_Intervals.end();

    value_type temp;
    bool tempInUse =false;
    while (ia != ea && ib != eb)
    {
        if (ia->first > ib->first)
        {
            std::swap(ia, ib);
            std::swap(ea, eb);
        }
        // ia->first <= ib->first for sure
        if (ia->second >= ib->second)
            // (*ib) devoured
            ++ib;
        else if (ia->second < ib->first && ia->second +1 < ib->first)
            // (*ia) and (*ib) disjointed
        {
            if (!tempInUse)
                dst.emplace_back(*ia);
            else
                // Pending is over
            {
                if (temp.second +1 >= ia->first)
                {
                    temp.second = ia->second;
                    dst.emplace_back(temp);
                }
                else
                {
                    dst.emplace_back(temp);
                    dst.emplace_back(*ia);
                }
                tempInUse =false;
            }
            ++ia;
        }
        else
            // Pending interval
        {
            if (!tempInUse)
            {
                temp.first = ia->first;
                tempInUse =true;
            }
            temp.second = ib->second;
            ++ia;
            ++ib;
        }
    }

    if (ib != eb)
    {
        ia =ib;
        ea =eb;
    }
    if (tempInUse)
    {
        while (ia != ea)
        {
            if (temp.second < ia->first && temp.second +1 < ia->first)
                break;

            temp.second = ia->second;
            ++ia;
        }
        dst.emplace_back(temp);
    }
    dst.insert(dst.end(), ia, ea);
    dst.swap(m_Intervals);
}

template<class T>
void C_IntervalsByTraits<T,true>::operator&=(const C_IntervalsByTraits<T,true> &other)
{
    decltype(m_Intervals) dst;
    const_iterator ia = other.m_Intervals.begin();
    const_iterator ea =other.m_Intervals.end();
    const_iterator ib =m_Intervals.begin();
    const_iterator eb =m_Intervals.end();

    while (ia != ea && ib != eb)
    {
        if (ia->first > ib->first)
        {
            std::swap(ia, ib);
            std::swap(ea, eb);
        }
        // ia->first <= ib->first for sure
        if (ia->second >= ib->second)
            // (*ib) devoured
            dst.emplace_back(*ib++);
        else if (ia->second < ib->first)
            // (*ia) and (*ib) disjointed
            ++ia;
        else
            // ia->first <= ib->first <= ia->second < ib->second
        {
            dst.emplace_back(ib->first, ia->second);
            ++ia;
        }
    }
    dst.swap(m_Intervals);
}

template<class T>
void C_IntervalsByTraits<T,true>::operator-=(const C_IntervalsByTraits<T,true> &other)
{
    auto ib =other.m_Intervals.begin(),
         eb =other.m_Intervals.end();

    for (auto ia =m_Intervals.begin(); ia != m_Intervals.end() && ib != eb;)
    {
        if (ib->second < ia->first)
        {
            ++ib;
            continue;
        }

        // ib->first <= ib->second
        // ia->first <= ib->second
        // ia->first <= ia->second
        if (ib->first <= ia->first)
        {
            if (ia->second <= ib->second)
                // (*ia) devoured by (*ib)
                ia = m_Intervals.erase(ia);
            else
                // LB of (*ia) is increased
            {
                ia->first = ib->second +1;
                ++ib;
            }
        }
        else // ib->first > ia->first
        {
            if (ia->second <= ib->second)
            {
                if (ia->second >= ib->first)
                    ia->second = ib->first -1;

                ++ia;
            }
            else // ia->second > ib->second
            {
                ia = m_Intervals.insert(ia, value_type(ia->first, ib->first-1));
                (++ia)->first = ib->second +1;
                ++ib;
            }
        }
    } // for (auto ia =m_Intervals.begin(); ia != m_Intervals.end() && ib != eb;)
}

template<class T>
void C_IntervalsByTraits<T,true>::complement()
{
    if (m_Intervals.empty())
    {
        m_Intervals.emplace_back(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
        return;
    }

    decltype(m_Intervals) dst;
    value_type t;
    t.first = std::numeric_limits<T>::min();
    for (const auto &i: m_Intervals)
    {
        if (i.first > std::numeric_limits<T>::min())
        {
            t.second = i.first -1;
            dst.emplace_back(t);
        }
        t.first = i.second +1;
    }
    if (t.first < std::numeric_limits<T>::max())
    {
        t.second = std::numeric_limits<T>::max();
        dst.emplace_back(t);
    }
    dst.swap(m_Intervals);
}

} // namespace bux

#endif // IntervalsH
