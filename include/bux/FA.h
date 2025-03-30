#pragma once

//  FA stands for Finite Automoton

#include <algorithm>    // std::set_difference()
#include <concepts>     // std::convertible_to<>, std::invocable<>
#include <iterator>     // std::inserter()
#include <list>         // std::list<>
#include <map>          // std::map<>
#include <set>          // std::set<>
#include <stdexcept>    // std::runtime_error
#include <string>       // std::to_string()
#include <vector>       // std::vector<>

namespace bux {

//
//      Types
//
class C_NfaState
/*! State of NFA ( NFA is defined below )
*/
{
public:

    // Nonvirtuals
    C_NfaState();
    C_NfaState(const C_NfaState &a) = default;
    C_NfaState &operator=(const C_NfaState &a) = default;
    auto operator<=>(const C_NfaState &a) const { return id <=> a.id; }
    bool operator==(const C_NfaState &a) const  { return id == a.id; }

private:

    // Data
    unsigned                id;
    static unsigned         nextId;
};

enum
{
    FA_OPTIONAL     = 1<<0,
    FA_REPEATABLE   = 1<<1
};

template<class T_Inputs>
struct C_FA_Traits
{
    // Class Methods
    static bool equalInput(const T_Inputs &a, const T_Inputs &b);
    static void inputDifference(T_Inputs &dst, const T_Inputs &src);
    static void inputIntersection(T_Inputs &dst, const T_Inputs &src);
    static void inputUnion(T_Inputs &dst, const T_Inputs &src);
    static bool isEmptyInput(const T_Inputs &src);
};

template<class T_Inputs, class T_Action, class C_Traits> class C_DFA;

template<class T_Inputs, class T_Action, class C_Traits = C_FA_Traits<T_Inputs>>
class C_NFA
/*! \param T_Inputs Set of inputs
    \param T_Action Type of action on final state.
    \param C_Traits Collection of compile-time decisions

    NFA stands for <em><B>N</B>ondeterministic <B>F</B>inite <B>A</B>utomoton</em>
*/
{
public:

    // Nonvirtuals
    C_NFA() = default;
    C_NFA(const C_NFA &a);
    C_NFA(C_NFA &&a);
    C_NFA(const T_Inputs &inputs);
    void operator=(const C_NFA &a);
    void operator=(C_NFA &&a);
    void operator|=(const C_NFA &a);
    void operator+=(const C_NFA &a) { (void)append(a); }
    void operator+=(const T_Inputs &inputs) { (void)append(inputs); }
    C_NFA &append(const C_NFA &a);
    C_NFA &append(const T_Inputs &inputs);
    C_NFA &changeTo(int options);
    template<class T> void setAction(T &&action);
    size_t totalFinalStates() const { return F.size(); }

private:

    // Types
    struct C_FinalState: C_NfaState
    {
        // Data
        T_Action    m_Tag;

        // Nonvirtuals
        C_FinalState() = default;
        C_FinalState(const C_NfaState &a, const T_Action &tag): C_NfaState(a), m_Tag(tag) {}
    };
    typedef std::vector<C_FinalState> C_FinalStates;

    struct C_NfaInputSet
    {
        // Data
        T_Inputs        m_UserInputs;
        bool            m_Epsilon;

        // Ctor
        C_NfaInputSet(bool epsilon = false): m_Epsilon(epsilon) {}
    };
    typedef std::map<C_NfaState,C_NfaInputSet> C_Transitions;
    typedef std::map<C_NfaState,C_Transitions> C_TransitionMap;

    // Data
    C_NfaState          q0;     // starting point index of Q
    C_FinalStates       F;      // final states of Q
    C_TransitionMap     delta;  // transition relation

    // Nonvirtuals
    void gatherStates(std::set<C_NfaState> &usedStates) const;
    const C_NFA &checkReuse(const std::set<C_NfaState> &usedStates, C_NFA &replacement) const;

    friend class C_DFA<T_Inputs,T_Action,C_Traits>;
};

template<class T_Inputs, class T_Action, class C_Traits = C_FA_Traits<T_Inputs>>
class C_DFA
/*! \param T_Inputs Set of inputs
    \param T_Action Type of action on final state.
    \param C_Traits Collection of compile-time decisions

    DFA stands for <em><B>D</B>eterministic <B>F</B>inite <B>A</B>utomoton</em>
*/
{
public:

    // Types
    typedef std::set<T_Action> C_Conflict;

    // Nonvirtuals
    template<class F_PickAction>
    C_DFA(const C_NFA<T_Inputs,T_Action,C_Traits> &nfa, F_PickAction pickAction) requires
        requires (int tag, const C_Conflict &conflict) {
            { pickAction(tag, conflict) }->std::convertible_to<T_Action>;
        }
    {
        C_FinalMap      Fraw;
        C_TransitionMap deltaRaw;
        auto n = nfa2dfa(nfa, Fraw, deltaRaw, pickAction);
        minDfa(Fraw, deltaRaw, n, F, delta);
    }
    void eachFinalState(std::invocable<int,const T_Action&> auto get) const
    {
        for (const auto &i: F)
            get(i.first, i.second);
    }
    void eachTransition(std::invocable<int,const T_Inputs&,int> auto get) const
    {
        for (const auto &i: delta)
            for (const auto &j: i.second)
                get(i.first, j.second, j.first);
    }
    bool isFinal(int state, T_Action &action) const;
        // Return true and assign action if state is final; return false otherwise
    static constexpr int startingState() { return 0; }
    size_t totalFinalStates() const { return F.size(); }

private:

    // Types
    typedef std::set<C_NfaState> C_NfaClosure;

    struct C_TaggedNfaClosure
    {
        C_NfaClosure        m_Closure;
        int                 m_Tag;

        C_TaggedNfaClosure(const C_NfaClosure &closure):
            m_Closure(closure), m_Tag(0)
            {}
    };

    typedef typename C_NFA<T_Inputs,T_Action,C_Traits>::C_TransitionMap C_SourceDelta;
    typedef std::map<int,T_Action> C_FinalMap;
    typedef std::map<int,T_Inputs> C_State2Inputs;
    typedef std::map<int,C_State2Inputs> C_TransitionMap;

    typedef std::set<int> C_DfaClosure;
    struct C_TaggedDfaClosure
    {
        C_DfaClosure        m_Closure;
        int                 m_Tag;

        C_TaggedDfaClosure()
            {}
        C_TaggedDfaClosure(const C_DfaClosure& closure): m_Closure(closure)
            {}
    };
    typedef std::list<C_TaggedDfaClosure> C_DfaClosures;
    typedef std::map<T_Action,C_DfaClosure> C_Action2Closure;

    //Data
    C_FinalMap              F;
    C_TransitionMap         delta;

    // Nonvirtuals
    static void buildA2C(const C_FinalMap &Ffat, const C_DfaClosure &closure,
        C_Action2Closure &a2c);
    static void createClosure(C_NfaClosure &c, const C_SourceDelta &delta);
    static typename C_DfaClosures::const_iterator findDfaClosure(const C_DfaClosures &Q, int state);
    static void minDfa(const C_FinalMap &Ffat, const C_TransitionMap &deltaFat,
        int totalFatStates, C_FinalMap &Fmin, C_TransitionMap &deltaMin);
    template<class F_PickAction>
    static int nfa2dfa(const C_NFA<T_Inputs,T_Action,C_Traits> &nfa, C_FinalMap &F,
        C_TransitionMap &delta, F_PickAction pickAction);
        // Return total of the resulting DFA states
};

//
//      Class Template Implementations
//
template<class T_Inputs, class T_Action, class C_Traits>
C_NFA<T_Inputs,T_Action,C_Traits>::C_NFA(const C_NFA &a):
    q0(a.q0), F(a.F), delta(a.delta)
{
}

template<class T_Inputs, class T_Action, class C_Traits>
C_NFA<T_Inputs,T_Action,C_Traits>::C_NFA(C_NFA &&a): q0(a.q0)
{
    F.swap(a.F);
    delta.swap(a.delta);
}

template<class T_Inputs, class T_Action, class C_Traits>
C_NFA<T_Inputs,T_Action,C_Traits>::C_NFA(const T_Inputs &inputs)
{
    C_FinalState f;
    delta[q0][f].m_UserInputs = inputs;
    F.emplace_back(f);
}

template<class T_Inputs, class T_Action, class C_Traits>
void C_NFA<T_Inputs,T_Action,C_Traits>::operator=(const C_NFA &a)
{
    q0      = a.q0;
    F       = a.F;
    delta   = a.delta;
}

template<class T_Inputs, class T_Action, class C_Traits>
void C_NFA<T_Inputs,T_Action,C_Traits>::operator=(C_NFA &&a)
{
    q0 = a.q0;
    F.swap(a.F);
    delta.swap(a.delta);
}

template<class T_Inputs, class T_Action, class C_Traits>
void C_NFA<T_Inputs,T_Action,C_Traits>::operator|=(const C_NFA &_a)
{
    if (F.empty())
        *this = _a;
    else
    {
        std::set<C_NfaState> usedStates;
        gatherStates(usedStates);
        C_NFA _;
        const C_NFA &a = _a.checkReuse(usedStates, _);
        F.insert(F.end(), a.F.begin(), a.F.end());
        delta.insert(a.delta.begin(), a.delta.end());
        delta[q0][a.q0].m_Epsilon =true;
    }
}

template<class T_Inputs, class T_Action, class C_Traits>
C_NFA<T_Inputs,T_Action,C_Traits> &C_NFA<T_Inputs,T_Action,C_Traits>::append(const C_NFA &_a)
{
    if (F.empty())
        *this = _a;
    else
    {
        std::set<C_NfaState> usedStates;
        gatherStates(usedStates);
        C_NFA _;
        const C_NFA &a = _a.checkReuse(usedStates, _);
        delta.insert(a.delta.begin(), a.delta.end());
        for (const auto &i: F)
            delta[i][a.q0].m_Epsilon = true;
        F = a.F;
    }
    return *this;
}

template<class T_Inputs, class T_Action, class C_Traits>
C_NFA<T_Inputs,T_Action,C_Traits> &C_NFA<T_Inputs,T_Action,C_Traits>::append(const T_Inputs &inputs)
{
    C_FinalState f;
    if (F.empty())
        delta[q0][f].m_UserInputs = inputs;
    else
    {
        for (const auto &i: F)
            delta[i][f].m_UserInputs = inputs;

        F.clear();
    }
    F.push_back(f);
    return *this;
}

template<class T_Inputs, class T_Action, class C_Traits>
C_NFA<T_Inputs,T_Action,C_Traits> &C_NFA<T_Inputs,T_Action,C_Traits>::changeTo(int options)
{
    for (const auto &i: F)
    {
        if (FA_REPEATABLE &options)
            delta[i][q0].m_Epsilon = true;
        if (FA_OPTIONAL &options)
            delta[q0][i].m_Epsilon = true;
    }
    return *this;
}

template<class T_Inputs, class T_Action, class C_Traits>
const C_NFA<T_Inputs,T_Action,C_Traits> &C_NFA<T_Inputs,T_Action,C_Traits>::checkReuse(
    const std::set<C_NfaState>  &usedStates,
    C_NFA<T_Inputs,T_Action,C_Traits> &replacement ) const
{
    typedef std::map<C_NfaState,C_NfaState> C_Map;
    C_Map map;
    for (const auto &i: delta)
    {
        if (usedStates.find(i.first) != usedStates.end())
            map[i.first];

        for (const auto &j: i.second)
            if (usedStates.find(j.first) != usedStates.end())
                map[j.first];
    }
    if (map.empty())
        // No conflict
        return *this;

    C_Map::const_iterator found = map.find(q0);
    replacement.q0 = found != map.end()? found->second: q0;

    for (const auto &i: F)
    {
        found = map.find(i);
        replacement.F.emplace_back(found != map.end()? found->second: i, i.m_Tag);
    }

    for (const auto &i: delta)
    {
        found = map.find(i.first);
        auto &t =replacement.delta[found != map.end()? found->second: i.first];
        for (const auto &j: i.second)
        {
            found = map.find(j.first);
            t[found != map.end()? found->second: j.first] =j.second;
        }
    }
    return replacement;
}

template<class T_Inputs, class T_Action, class C_Traits>
void C_NFA<T_Inputs,T_Action,C_Traits>::gatherStates(std::set<C_NfaState> &usedStates) const
{
    for (const auto &i: delta)
    {
        usedStates.insert(i.first);
        for (const auto &j: i.second)
            usedStates.insert(j.first);
    }
}

template<class T_Inputs, class T_Action, class C_Traits>
template<class T>
void C_NFA<T_Inputs,T_Action,C_Traits>::setAction(T &&action)
{
    for (auto &i: F)
        i.m_Tag = std::forward<T>(action);
}

template<class T_Inputs, class T_Action, class C_Traits>
void C_DFA<T_Inputs,T_Action,C_Traits>::buildA2C(
    const C_FinalMap        &Ffat,
    const C_DfaClosure      &closure,
    C_Action2Closure        &a2c        )
{
    for (const auto &i: Ffat)
    {
        if (closure.find(i.first) != closure.end())
            a2c[i.second].insert(i.first);
    }
}

template<class T_Inputs, class T_Action, class C_Traits>
void C_DFA<T_Inputs,T_Action,C_Traits>::createClosure(C_NfaClosure &c, const C_SourceDelta &delta)
{
    C_NfaClosure test(c);
    while (true)
    {
        C_NfaClosure add;
        for (const auto &i: test)
        {
            const auto found = delta.find(i);
            if (found != delta.end())
            {
                for (const auto &j: found->second)
                    if (j.second.m_Epsilon && c.find(j.first) == c.end())
                        add.insert(j.first);
            }
        }
        if (add.empty())
            break;

        c.insert(add.begin(), add.end());
        test.swap(add);
    }
}

template<class T_Inputs, class T_Action, class C_Traits>
auto C_DFA<T_Inputs,T_Action,C_Traits>::findDfaClosure(const C_DfaClosures &Q, int state) -> typename C_DfaClosures::const_iterator
{
    for (auto i = Q.begin(), end = Q.end(); i != end; ++i)
    {
        if (i->m_Closure.find(state) != i->m_Closure.end())
            // Found
            return i;
    }
    throw std::runtime_error{"State " + std::to_string(state) + " not found"};
}

template<class T_Inputs, class T_Action, class C_Traits>
bool C_DFA<T_Inputs,T_Action,C_Traits>::isFinal(int state, T_Action &action) const
{
    const auto i =F.find(state);
    if (i != F.end())
    {
        action = i->second;
        return true;
    }
    return false;
}

template<class T_Inputs, class T_Action, class C_Traits>
void C_DFA<T_Inputs,T_Action,C_Traits>::minDfa(
    const C_FinalMap        &Ffat,
    const C_TransitionMap   &deltaFat,
    int                     totalFatStates,
    C_FinalMap              &Fmin,
    C_TransitionMap         &deltaMin       )
{
    // Figure 7.7: Algorithm MinDFA
    C_DfaClosures Qfat;
    Qfat.emplace_back();
    Qfat.emplace_back();
    auto &part0 = Qfat.front().m_Closure;
    auto &part1 = Qfat.back().m_Closure;
    int tag =0;
    for (const auto &i: Ffat)
    {
        part0.insert(i.first);
        while (tag < i.first)
            part1.insert(tag++);

        ++tag;
    }
    while (tag < totalFatStates)
        part1.insert(tag++);

    bool changed;
    do
    {
        changed = false;
        C_DfaClosures Qmin;
        for (const auto &i: Qfat)
        {
            auto fat = i.m_Closure;
            while (!fat.empty())
            {
                C_DfaClosure slim;
                auto j = fat.begin();
                slim.insert(*j);
                typedef std::map<const C_DfaClosure*,T_Inputs> C_CmpMap;
                C_CmpMap cmpmap;
                auto k = deltaFat.find(*j);
                if (k != deltaFat.end())
                    for (const auto &m: k->second)
                        cmpmap[&findDfaClosure(Qfat, m.first)->m_Closure] = m.second;

                while (++j != fat.end())
                {
                    C_CmpMap map;
                    bool deal = true;
                    k = deltaFat.find(*j);
                    if (k != deltaFat.end())
                        for (const auto &m: k->second)
                        {
                            const auto *t = &findDfaClosure(Qfat, m.first)->m_Closure;
                            auto found = cmpmap.find(t);
                            if (found != cmpmap.end() &&
                                C_Traits::equalInput(m.second, found->second))
                            {
                                map[t] = m.second;
                                continue;
                            }
                            deal = false;
                            break;
                        }
                    if (deal)
                    {
                        auto i_map = map.begin();
                        for (auto &j_cmp: cmpmap)
                        {
                            if (i_map == map.end() ||
                                i_map->first != j_cmp.first ||
                                !C_Traits::equalInput(i_map->second, j_cmp.second))
                            {
                                deal = false;
                                break;
                            }
                            ++i_map;
                        }
                        if (deal && i_map == map.end())
                            slim.insert(*j);
                    }
                }
                Qmin.emplace_back(slim);
                C_DfaClosure temp;
                std::set_difference(fat.begin(), fat.end(),
                    slim.begin(), slim.end(), std::inserter(temp, temp.begin()));
                fat.swap(temp);
            }
        }
        if (Qmin.size() == Qfat.size())
        {
            for (auto i =Qmin.begin(), end =Qmin.end(); i != end; ++i)
            {
                C_Action2Closure a2c;
                buildA2C(Ffat, i->m_Closure, a2c);
                if (a2c.size() > 1)
                {
                    // Solve conflict
                    auto j = a2c.begin();
                    i->m_Closure.swap(j->second); // overwrite the superset
                    auto next = i;
                    ++next;
                    while (++j != a2c.end())
                        i = Qmin.insert(next, j->second);
                }
            }
        }
        if (Qmin.size() != Qfat.size())
        {
            Qfat.swap(Qmin);
            changed =true;
        }
    } while (changed);

    // Tagging
    tag =0;
    for (auto i = Qfat.begin(), end = Qfat.end(); i != end; ++i, ++tag)
    {
        if (tag && i->m_Closure.find(0) != i->m_Closure.end())
            // Q0 is always tagged 0
        {
            i->m_Tag = 0;
            Qfat.front().m_Tag = tag;
            auto t = Fmin.find(0);
            if (t != Fmin.end())
            {
                const auto a = t->second;
                Fmin.erase(t);
                Fmin[tag] = a;
            }
        }
        else
            i->m_Tag = tag;

        C_Action2Closure a2c;
        buildA2C(Ffat, i->m_Closure, a2c);
        if (!a2c.empty())
        {
            Fmin[i->m_Tag] = a2c.begin()->first;
            if (a2c.size() > 1)
                // Impossible
                throw std::runtime_error{"a2c.size() == " + std::to_string(a2c.size())};
        }
    }
    for (const auto &i: deltaFat)
    {
        auto &dst_i = deltaMin[findDfaClosure(Qfat,i.first)->m_Tag];
        for (const auto &j: i.second)
        {
            const auto value = findDfaClosure(Qfat,j.first)->m_Tag;
            for (const auto &dst_j: dst_i)
            {
                if (C_Traits::equalInput(dst_j.second, j.second))
                {
                    if (dst_j.first == value)
                        // Aleady added
                        goto PostInsertion;

                    throw std::runtime_error{"Contradicted mapping"};
                }
            }
            C_Traits::inputUnion(dst_i[value], j.second);
        PostInsertion:;
        }
    }
}

template<class T_Inputs, class T_Action, class C_Traits>
template<class F_PickAction>
int C_DFA<T_Inputs,T_Action,C_Traits>::nfa2dfa(const C_NFA<T_Inputs,T_Action,C_Traits> &nfa,
    C_FinalMap &F, C_TransitionMap &delta, F_PickAction pickAction)
{
    // Figure 7.5 (p245): Algorithm NFA->DFA
    std::list<C_TaggedNfaClosure> Q;    // collector of all states
    Q.emplace_back(C_NfaClosure{});
    auto *const q0 = &Q.front().m_Closure; // starting point index of Q
    q0->insert(nfa.q0);
    createClosure(*q0, nfa.delta);

    typedef std::pair<T_Inputs, const C_TaggedNfaClosure*> C_InputClosurePair;
    typedef std::vector<C_InputClosurePair> C_Input2Closure;
    std::map<const C_TaggedNfaClosure*, C_Input2Closure,
            bool (*)(const C_TaggedNfaClosure*,const C_TaggedNfaClosure*)>
            deltaNfa([](const C_TaggedNfaClosure *a, const C_TaggedNfaClosure *b)->bool {
                return a->m_Closure < b->m_Closure;
            });  // transition relation
    bool    changed;
    do
    {
        changed =false;
        for (auto &q: Q)
        {
            if (q.m_Tag)
                continue;

            typedef std::map<C_NfaClosure,T_Inputs> C_ShiftMap;
            C_ShiftMap map;
            q.m_Tag = true;
            for (const auto &j: q.m_Closure)
            {
                auto found = nfa.delta.find(j);
                if (found != nfa.delta.end())
                    for (const auto &k: found->second)
                    {
                        if (!C_Traits::isEmptyInput(k.second.m_UserInputs))
                        {
                            C_NfaClosure key;
                            key.insert(k.first);
                            C_Traits::inputUnion(map[key], k.second.m_UserInputs);
                        }
                    }
            }
            // Here map is already created with keys of state singletons and values without epsilon
            C_ShiftMap partitionedMap;
        MergeAgain:
            switch (map.size())
            {
            case 1:
                partitionedMap.insert(*map.begin());
                break;
            case 0:
                break;
            default: // > 1
                for (auto i = map.begin(), end = map.end(); i != end; ++i)
                    for (auto j = i; ++j != end;)
                    {
                        // Emptyness of (i - j)
                        T_Inputs insetsect(i->second);
                        C_Traits::inputIntersection(insetsect, j->second);
                        if (C_Traits::isEmptyInput(insetsect))
                            continue;

                        if (map.begin() != i)
                            // Move away the disjointed part first
                        {
                            partitionedMap.insert(map.begin(), i);
                            map.erase(map.begin(), i);
                        }
                        C_NfaClosure key(i->first);
                        key.insert(j->first.begin(), j->first.end());

                        T_Inputs t(i->second);
                        C_Traits::inputDifference(t, insetsect);
                        if (C_Traits::isEmptyInput(t))
                            map.erase(i);
                        else
                            i->second = t;

                        t = j->second;
                        C_Traits::inputDifference(t, insetsect);
                        if (C_Traits::isEmptyInput(t))
                            map.erase(j);
                        else
                            j->second = t;

                        map[key] =insetsect;
                        goto MergeAgain;
                    }
                partitionedMap.insert(map.begin(), map.end());
            } // switch (map.size())
            map.swap(partitionedMap);
            /* Here map is with
             *  keys union of which equals to union of original keys
             * and
             *  values which together partition the union of all original values except episilon.
             */
            for (const auto &j: map)
            {
                C_TaggedNfaClosure t(j.first);
                createClosure(t.m_Closure, nfa.delta);
                const C_TaggedNfaClosure *next;
                for (const auto &k: Q)
                    if (t.m_Closure == k.m_Closure)
                    {
                        next = &k;
                        goto postAddClosure;
                    }
                Q.emplace_back(t); // new state t
                next = &Q.back();
            postAddClosure:
                deltaNfa[&q].emplace_back(j.second, next);
                changed =true;
            }
        }
    } while (changed);

    int tag = 0;
    for (auto &i: Q)
    {
        i.m_Tag = tag++;
        C_Conflict conflict;
        for (const auto &j: nfa.F)
        {
            if (i.m_Closure.find(j) != i.m_Closure.end())
                conflict.insert(j.m_Tag);
        }
        if (!conflict.empty())
        {
            if (conflict.size() > 1)
                // Solve conflict
            {
                auto action = pickAction(i.m_Tag, conflict);
                if (action == T_Action())
                    throw std::runtime_error{"Interrupted"};

                F[i.m_Tag] = action;
            }
            else
                F[i.m_Tag] = *conflict.begin();
        }
    }

    for (const auto &i: deltaNfa)
    {
        auto &dstMap = delta[i.first->m_Tag];
        for (const auto &j: i.second)
            C_Traits::inputUnion(dstMap[j.second->m_Tag], j.first);
    }
    return tag;
}

} // namespace bux
