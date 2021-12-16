#pragma once

#include "XException.h" // RUNTIME_ERROR()
#include <concepts>     // std::integral<>
#include <iterator>     // std::input_iterator<>
#include <limits>       // std::numeric_limits<>
#include <ostream>      // std::basic_ostream<>
#include <vector>       // std::vector<>

namespace bux {

//
//      Types
//
template<class T>
concept IntervalPt = std::integral<T> && !std::same_as<char,T>;

template<IntervalPt T>
class C_Intervals
{
public:

    // Types
    typedef std::pair<T,T> value_type;
    typedef typename std::vector<value_type>::const_iterator const_iterator;

    // Ctors
    C_Intervals() {}
    C_Intervals(const C_Intervals &other): m_Intervals(other.m_Intervals) {}
    C_Intervals(C_Intervals &&other) { swap(other); }
    C_Intervals(T id);
    C_Intervals(value_type i);
    C_Intervals(T start, T end): C_Intervals(value_type(start, end)) {}
    template<std::input_iterator I>
    C_Intervals(I start, I end) requires std::same_as<char, std::remove_cvref_t<decltype(*start)>>
    {
        while (start != end)
            operator|=(T(*start++)); // The cast can be an undefined behavior but I don't care for now.
    }
    template<std::input_iterator I>
    C_Intervals(I start, I end) requires
        std::same_as<value_type, std::remove_cvref_t<decltype(*start)>> ||
        (
            !std::same_as<char,std::remove_cvref_t<decltype(*start)>> &&
            std::integral<std::remove_cvref_t<decltype(*start)>> &&
            std::cmp_less_equal(std::numeric_limits<T>::min(), std::numeric_limits<std::remove_cvref_t<decltype(*start)>>::min()) &&
            std::cmp_less_equal(std::numeric_limits<std::remove_cvref_t<decltype(*start)>>::max(), std::numeric_limits<T>::max())
        )
    {
        while (start != end)
            operator|=(*start++);
    }

    // Nonvirtuals - Assignments
    void operator=(const C_Intervals &other)   { m_Intervals =other.m_Intervals; }
    void operator=(C_Intervals &&other)        { swap(other); }
    void operator|=(const C_Intervals &other);
    void operator&=(const C_Intervals &other);
    void operator-=(const C_Intervals &other);
    bool operator==(const C_Intervals &other) const { return m_Intervals == other.m_Intervals; }

    // Nonvirtuals - Immutables
    const_iterator begin() const    { return m_Intervals.begin(); }
    const_iterator end() const      { return m_Intervals.end(); }
    bool empty() const              { return m_Intervals.empty(); }
    size_t size() const             { return m_Intervals.size(); }

    // Nonvirtuals - Mutables
    void complement();
    void swap(C_Intervals &other) { m_Intervals.swap(other.m_Intervals); }

private:

    // Data
    std::vector<value_type> m_Intervals;
};

//
//      Function Templates
//
template<IntervalPt T, class charT, class traits=std::char_traits<charT>>
std::basic_ostream<charT,traits> &
operator<<(std::basic_ostream<charT,traits> &out, const C_Intervals<T> &x)
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
template<IntervalPt T>
C_Intervals<T>::C_Intervals(T id)
{
    m_Intervals.emplace_back(id, id);
}

template<IntervalPt T>
C_Intervals<T>::C_Intervals(value_type i)
{
    if (i.first > i.second)
        RUNTIME_ERROR("({},{})", i.first, i.second);

    m_Intervals.emplace_back(i);
}

template<IntervalPt T>
void C_Intervals<T>::operator|=(const C_Intervals<T> &other)
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

template<IntervalPt T>
void C_Intervals<T>::operator&=(const C_Intervals<T> &other)
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

template<IntervalPt T>
void C_Intervals<T>::operator-=(const C_Intervals<T> &other)
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

template<IntervalPt T>
void C_Intervals<T>::complement()
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
