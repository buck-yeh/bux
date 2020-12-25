#ifndef bux_PartialOrdering_H_
#define bux_PartialOrdering_H_

#include <functional>   // std::equal_to<>, std::function<>
#include <list>         // std::list<>
#include "XException.h" // LOGIC_ERROR()

namespace bux {

//
//      Types
//
enum E_MakeLinearPolicy
{
    MLP_BREADTH_FIRST,
    MLP_DEPTH_FIRST
};

template<class T, class FC_Equ = std::equal_to<T>>
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
    explicit C_PartialOrdering(const FC_Equ &equator = {});
    [[nodiscard]]bool addOrder(const T &a, const T &b);
        // Return true if given order (a,b) is successfully added
    template<class T2>
    bool anyLeft(const T2 &R) const;
    template<class T2>
    bool anyRight(const T2 &L) const;
    void clear();
    size_t depthToLeft(T t) const;
        // Return 0 for the leftmosts.
    bool empty() const { return m_Relation.empty(); }
    template<class T2>
    void getRelated(T2 &&L, std::function<void(const T&)> R, size_t maxDepth = 0) const;
    void getRelated(C_ValList L, std::function<void(const T&)> R, size_t maxDepth = 0) const;
    template<class T2>
    void getRelated(std::function<void(const T&)> L, T2 &&R, size_t maxDepth = 0) const;
    void getRelated(std::function<void(const T&)> L, C_ValList R, size_t maxDepth = 0) const;
    void makeLinear(const std::function<void(const T&)> &apply, E_MakeLinearPolicy policy = MLP_BREADTH_FIRST) const;
    bool related(const T &a, const T &b) const;

private:

    // Types
    typedef std::pair<T,T> C_Edge;
    typedef std::list<C_Edge>  C_Relation;

    // Data
    const FC_Equ            m_Equ;
    C_Relation              m_Relation;

    // Nonvirtuals
    bool find(const C_ValList &src, T t) const;
    void pruneLeftmost(C_ValList &l, const C_ValList &r) const;
};

//
//      Function Templates
//
template<class C, class T, class FC_Equ>
void addUnique(C &c, T &&value, FC_Equ equ)
{
    for (auto &i: c)
        if (equ(i, value))
            return;

    c.emplace_back(std::forward<T>(value));
}

//
//      Implement Class Templates
//
template<class T, class FC_Equ>
C_PartialOrdering<T,FC_Equ>::C_PartialOrdering(const FC_Equ &equator):
    m_Equ(equator)
{
}

template<class T, class FC_Equ>
bool C_PartialOrdering<T,FC_Equ>::addOrder(const T &a, const T &b)
{
    if (m_Equ(a, b))
        // a == b
        return false;

    if (related(b, a))
        // Looping
        return false;

    for (auto &i: m_Relation)
        if (m_Equ(a, i.first) && m_Equ(b, i.second))
            // Already added
            return true;

    m_Relation.emplace_back(a, b);
    return true;
}

template<class T, class FC_Equ>
size_t C_PartialOrdering<T,FC_Equ>::depthToLeft(T t) const
{
    size_t ret =0;
    for (auto &i: m_Relation)
        if (m_Equ(t, i.second))
        {
            const size_t cur = depthToLeft(i.first) +1;
            if (cur > ret)
                ret =cur;
        }
    return ret;
}

template<class T, class FC_Equ>
void C_PartialOrdering<T,FC_Equ>::clear()
{
    m_Relation.clear();
}

template<class T, class FC_Equ>
bool C_PartialOrdering<T,FC_Equ>::find(const C_ValList &src, T t) const
{
    for (auto &i: src)
        if (m_Equ(t, i))
            return true;

    return false;
}

template<class T, class FC_Equ>
template<class T2>
bool C_PartialOrdering<T,FC_Equ>::anyLeft(const T2 &R) const
{
    for (auto &i: m_Relation)
        if (m_Equ(i.second, R))
            return true;

    return false;
}

template<class T, class FC_Equ>
template<class T2>
bool C_PartialOrdering<T,FC_Equ>::anyRight(const T2 &L) const
{
    for (auto &i: m_Relation)
        if (m_Equ(L, i.first))
            return true;

    return false;
}

template<class T, class FC_Equ>
template<class T2>
void C_PartialOrdering<T,FC_Equ>::getRelated(T2 &&L, std::function<void(const T&)> R, size_t maxDepth) const
{
    getRelated({1, std::forward<T2>(L)}, R, maxDepth);
}

template<class T, class FC_Equ>
template<class T2>
void C_PartialOrdering<T,FC_Equ>::getRelated(std::function<void(const T&)> L, T2 &&R, size_t maxDepth) const
{
    getRelated(L, {1, std::forward<T2>(R)}, maxDepth);
}

template<class T, class FC_Equ>
void C_PartialOrdering<T,FC_Equ>::getRelated(C_ValList L, std::function<void(const T&)> R, size_t maxDepth) const
{
    size_t oldSize = 0;
    C_ValList dst;
    for (size_t i = 0; !maxDepth || i < maxDepth; ++i)
    {
        for (auto &i: m_Relation)
            for (auto &j: L)
                if (m_Equ(j, i.first))
                    addUnique(dst, i.second, m_Equ);

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

template<class T, class FC_Equ>
void C_PartialOrdering<T,FC_Equ>::getRelated(std::function<void(const T&)> L, C_ValList R, size_t maxDepth) const
{
    size_t oldSize = 0;
    C_ValList dst;
    for (size_t i = 0; !maxDepth || i < maxDepth; ++i)
    {
        for (auto &i: m_Relation)
            for (auto &j: R)
                if (m_Equ(j, i.second))
                    addUnique(dst, i.first, m_Equ);

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

template<class T, class FC_Equ>
void C_PartialOrdering<T,FC_Equ>::makeLinear(
    const std::function<void(const T&)> &apply,
    E_MakeLinearPolicy                  policy ) const
{
/*! \param [in] apply Node visitor.
    \param [in] policy How to serialize the lattice ? Breadth first or depth first.

    [Excerpt from <a href="http://en.wikipedia.org/wiki/Topological_sort" target="_blank">here</a>]
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
    C_Relation  mapping = m_Relation;
    for (auto &i: mapping)
    {
        addUnique(q, i.first, m_Equ);
        addUnique(r, i.second, m_Equ);
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
            if (m_Equ(t, i->first))
            {
                addUnique(l, i->second, m_Equ);
                i = mapping.erase(i);
            }
            else
            {
                addUnique(r, i->second, m_Equ);
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
        default:
            LOGIC_ERROR("Unknown policy {}", int(policy));
        }
    }
}

template<class T, class FC_Equ>
void C_PartialOrdering<T,FC_Equ>::pruneLeftmost(C_ValList &l, const C_ValList &r) const
{
    for (auto i = l.begin(), end = l.end(); i != end;)
    {
        if (find(r, *i))
            i = l.erase(i);
        else
            ++i;
    }
}

template<class T, class FC_Equ>
bool C_PartialOrdering<T,FC_Equ>::related(const T &a, const T &b) const
{
    for (auto &i: m_Relation)
        if (m_Equ(a, i.first))
        {
            if (m_Equ(i.second, b))
                // aRb
                return true;

            if (related(i.second, b))
                // aRt && tRb
                return true;
        }
    return false;
}

} // namespace bux

#endif // bux_PartialOrdering_H_
