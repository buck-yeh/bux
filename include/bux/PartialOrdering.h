#pragma once

#include <concepts>     // std::constructible_from<>, std::equality_comparable<>, std::invocable<>
#include <list>         // std::list<>

namespace bux {

//
//      Types
//
enum E_MakeLinearPolicy
{
    MLP_BREADTH_FIRST,
    MLP_DEPTH_FIRST
};

template<std::equality_comparable T>
class C_PartialOrdering
/*! \e DEFINITIONs:
    Let R be a partial ordering and aRb. Then a is said to be "left-related"
    to b, and similarly b is "right-related" to a. If no x in fld R satisfies
    xRa, a is said to be a "leftmost" in R. "Rightmost" is defined similarly.
*/
{
public:

    // Types
    typedef std::list<T> C_ValList;

    // Nonvirtuals
    template<class T1, class T2>
    [[nodiscard]]bool addOrder(T1 a, T2 b);
        // Return true if given order (a,b) is successfully added
    template<class T2>
    bool anyLeft(const T2 &R) const;
    template<class T2>
    bool anyRight(const T2 &L) const;
    void clear()                        { m_Relation.clear(); }
    size_t depthToLeft(T t) const;
        // Return 0 for the leftmosts.
    bool empty() const { return m_Relation.empty(); }
    template<class T2, class F>
    void getRelated(T2 L, F R, size_t maxDepth = 0) const requires std::constructible_from<T,T2> && std::invocable<F,T>;
    template<class F>
    void getRelated(C_ValList L, F R, size_t maxDepth = 0) const requires std::invocable<F,T>;
    template<class F, class T2>
    void getRelated(F L, T2 R, size_t maxDepth = 0) const requires std::constructible_from<T,T2> && std::invocable<F,T>;
    template<class F>
    void getRelated(F L, C_ValList R, size_t maxDepth = 0) const requires std::invocable<F,T>;
    template<class F>
    void makeLinear(F apply, E_MakeLinearPolicy policy = MLP_BREADTH_FIRST) const requires std::invocable<F,T>;
    bool related(const T &a, const T &b) const;

private:

    // Data
    std::list<std::pair<T,T>>   m_Relation;

    // Nonvirtuals
    bool find(const C_ValList &src, T t) const;
    void pruneLeftmost(C_ValList &l, const C_ValList &r) const;
};

//
//      Function Templates
//
template<class C, class T>
void addUnique(C &c, T &&value)
{
    for (auto &i: c)
        if (i == value)
            return;

    c.emplace_back(std::forward<T>(value));
}

//
//      Implement Class Templates
//
template<std::equality_comparable T>
template<class T1, class T2>
bool C_PartialOrdering<T>::addOrder(T1 a, T2 b)
{
    if (a == b)
        return false;

    if (related(b, a))
        // Looping
        return false;

    for (auto &i: m_Relation)
        if (a == i.first && b == i.second)
            // Already added
            return true;

    m_Relation.emplace_back(a, b);
    return true;
}

template<std::equality_comparable T>
size_t C_PartialOrdering<T>::depthToLeft(T t) const
{
    size_t ret =0;
    for (auto &i: m_Relation)
        if (t == i.second)
        {
            const size_t cur = depthToLeft(i.first) +1;
            if (cur > ret)
                ret = cur;
        }
    return ret;
}

template<std::equality_comparable T>
bool C_PartialOrdering<T>::find(const C_ValList &src, T t) const
{
    for (auto &i: src)
        if (t == i)
            return true;

    return false;
}

template<std::equality_comparable T>
template<class T2>
bool C_PartialOrdering<T>::anyLeft(const T2 &R) const
{
    for (auto &i: m_Relation)
        if (i.second == R)
            return true;

    return false;
}

template<std::equality_comparable T>
template<class T2>
bool C_PartialOrdering<T>::anyRight(const T2 &L) const
{
    for (auto &i: m_Relation)
        if (L == i.first)
            return true;

    return false;
}

template<std::equality_comparable T>
template<class T2, class F>
void C_PartialOrdering<T>::getRelated(T2 L, F R, size_t maxDepth) const requires std::constructible_from<T,T2> && std::invocable<F,T>
{
    getRelated({1, std::forward<T2>(L)}, R, maxDepth);
}

template<std::equality_comparable T>
template<class F, class T2>
void C_PartialOrdering<T>::getRelated(F L, T2 R, size_t maxDepth) const requires std::constructible_from<T,T2> && std::invocable<F,T>
{
    getRelated(L, {1, std::forward<T2>(R)}, maxDepth);
}

template<std::equality_comparable T>
template<class F>
void C_PartialOrdering<T>::getRelated(C_ValList L, F R, size_t maxDepth) const requires std::invocable<F,T>
{
    size_t oldSize = 0;
    C_ValList dst;
    for (size_t depth = 0; !maxDepth || depth < maxDepth; ++depth)
    {
        for (auto &i: m_Relation)
            for (auto &j: L)
                if (j == i.first)
                    addUnique(dst, i.second);

        if (oldSize == dst.size())
            break;

        auto iDst = dst.begin();
        std::advance(iDst, oldSize);
        L.assign(iDst, dst.end());
        oldSize = dst.size();
    }
    for (auto &i: dst)
        R(i);
}

template<std::equality_comparable T>
template<class F>
void C_PartialOrdering<T>::getRelated(F L, C_ValList R, size_t maxDepth) const requires std::invocable<F,T>
{
    size_t oldSize = 0;
    C_ValList dst;
    for (size_t depth = 0; !maxDepth || depth < maxDepth; ++depth)
    {
        for (auto &i: m_Relation)
            for (auto &j: R)
                if (j == i.second)
                    addUnique(dst, i.first);

        if (oldSize == dst.size())
            break;

        auto iDst = dst.begin();
        std::advance(iDst, oldSize);
        R.assign(iDst, dst.end());
        oldSize = dst.size();
    }
    for (auto &i: dst)
        L(i);
}

template<std::equality_comparable T>
template<class F>
void C_PartialOrdering<T>::makeLinear(F apply, E_MakeLinearPolicy policy) const requires std::invocable<F,T>
{
/*! \param [in] apply Node visitor.
    \param [in] policy How to serialize the lattice ? Breadth first or depth first.

    [Excerpt from <a href="https://en.wikipedia.org/wiki/Topological_sorting" target="_blank">here</a>]
    The usual algorithm for topological sorting has running time linear in the
    number of nodes plus the number of edges (<tt>O(|V|+|E|)</tt>). It uses depth-first
    search. First, find a list of "start nodes" which have no incoming edges and
    insert them into a queue Q. Then,

    \code
    while Q is nonempty do
        remove a node n from Q
        output n
        for each node m with an edge e from n to m do
            remove edge e from the graph
            if m has no other incoming edges then
                insert m into Q \endcode

    If this algorithm terminates without outputting all the nodes of the graph,
    it means the graph has at least one cycle and therefore is not a
    <a href="http://en.wikipedia.org/wiki/Directed_acyclic_graph" target="_blank">DAG</a>,
    so the algorithm can report an error.
*/
    C_ValList   q, r;
    auto        mapping = m_Relation;
    for (auto &i: mapping)
    {
        addUnique(q, i.first);
        addUnique(r, i.second);
    }
    pruneLeftmost(q, r);

    while (!q.empty())
    {
        const T t =q.front();
        q.pop_front();
        apply(t);

        C_ValList l;
        r.clear();
        for (auto i = mapping.begin(), end = mapping.end(); i != end;)
        {
            if (t == i->first)
            {
                addUnique(l, i->second);
                i = mapping.erase(i);
            }
            else
            {
                addUnique(r, i->second);
                ++i;
            }
        }

        pruneLeftmost(l, r);
        switch (policy)
        {
        case MLP_BREADTH_FIRST:
            q.splice(q.end(), l);
            break;
        case MLP_DEPTH_FIRST:
            q.splice(q.begin(), l);
            break;
        }
    }
}

template<std::equality_comparable T>
void C_PartialOrdering<T>::pruneLeftmost(C_ValList &l, const C_ValList &r) const
{
    for (auto i = l.begin(), end = l.end(); i != end;)
    {
        if (find(r, *i))
            i = l.erase(i);
        else
            ++i;
    }
}

template<std::equality_comparable T>
bool C_PartialOrdering<T>::related(const T &a, const T &b) const
{
    for (auto &i: m_Relation)
        if (a == i.first)
        {
            if (i.second == b)
                // aRb
                return true;

            if (related(i.second, b))
                // aRt && tRb
                return true;
        }
    return false;
}

} // namespace bux
