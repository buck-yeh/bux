#ifndef ParserBaseH
#define ParserBaseH

#include "LexBase.h"    // bux::T_LexID, bux::C_LexInfoT<>, ...
#include <functional>   // std::function<>
#include <type_traits>  // std::is_const<>, std::is_pointer<>, std::add_const_t<>
#include <algorithm>    // std::lower_bound()
#include <limits>       // std::numeric_limits<>

namespace bux {

//
//      Types
//
using T_StateID = unsigned;

template<class T, template<class> class C_Ptr>
using F_GetProducedT = std::function<C_LexInfoT<T,C_Ptr> &(size_t)>;

template<class T, template<class> class C_Ptr>
class FC_GetRelLexT
{
public:

    // Nonvirtuals
    FC_GetRelLexT(const F_GetProducedT<T,C_Ptr> &from, int baseInd): m_from(from), m_baseInd(baseInd)
        {}
    auto &operator()(size_t n)
        { return m_from(size_t(n + m_baseInd)); }

private:

    // Data
    const F_GetProducedT<T,C_Ptr>   m_from;
    const int                       m_baseInd;
};

template<class T_Key, class T_Value>
struct C_KVPair
{
    T_Key       m_key;
    T_Value     m_value;
};

template<class T_Key, class T_Value>
union U_K2V
{
    const C_KVPair<T_Key,T_Value> *m_table;
    int (*m_conv)(T_Key);

    constexpr U_K2V(const C_KVPair<T_Key,T_Value> *table): m_table(table) {}
    constexpr U_K2V(int (*conv)(T_Key)): m_conv(conv) {}
};

template<class T, bool>
struct C_PtrTraits_;
template<class T>
using C_PtrTraits = C_PtrTraits_<T,std::is_const<T>::value>;

template<class T>
struct C_PtrTraits_<T,true>
{
    static auto to_voidptr(T *p) { return const_cast<void*>(static_cast<const void*>(p)); }
};

template<class T>
struct C_PtrTraits_<T,false>
{
    static auto to_voidptr(T *p) { return static_cast<void*>(p); }
};

template<class T, bool>
struct T_CoConst_;
template<class T_Data, class T_Dep>
using T_CoConst = typename T_CoConst_<T_Data,std::is_const<T_Dep>::value>::type;

template<class T>
struct T_CoConst_<T,true>
{
    using type = std::add_const_t<T>;
};
template<class T>
struct T_CoConst_<T,false>
{
    using type = T;
};

template<class T, bool>
struct C_BackTraits_;
template<class T>
using C_BackTraits = C_BackTraits_<T,std::is_pointer<T>::value>;

template<class T>
struct C_BackTraits_<T,true>
{
    using T_RetType = T;

    static auto from_voidptr(void *p) { return reinterpret_cast<T_RetType>(p); }
};

template<class T>
struct C_BackTraits_<T,false>
{
    using T_CastType = std::remove_reference_t<T>;
    using T_RetType  = T_CastType &;

    static auto from_voidptr(void *p) { return *reinterpret_cast<T_CastType*>(p); }
};

//
//      Function Templates
//
template<class T>
auto to_voidptr(T *t) { return C_PtrTraits<T>::to_voidptr(t); }
template<class T>
auto to_voidptr(T &t) { return C_PtrTraits<T>::to_voidptr(&t); }

template<class T>
auto voidptr_to(void *p) { return C_BackTraits<T>::from_voidptr(p); }

template<class T_Key, class T_Value, class T_ValueOrSize, class T_Traits>
T_Value index2value(U_K2V<T_Key,T_Value> k2v, T_ValueOrSize valOrSz, T_LexID key)
{
    // Constraints
    static_assert(std::numeric_limits<T_ValueOrSize>::is_signed && sizeof(T_Value) <= sizeof(T_ValueOrSize),
        "T_Value/T_ValueOrSize criteria");

    if (valOrSz < 0)
        // (-valOrSz) is size of index array
    {
        auto end   = k2v.m_table - valOrSz;
        auto found = std::lower_bound(k2v.m_table, end, key,
            [](C_KVPair<T_Key,T_Value> a, T_LexID b)->bool {
                return a.m_key < b;
            });
        if (found != end && found->m_key == key)
            return found->m_value;
    }
    else
        //  valOrSz is THE value
    {
        const auto ind = k2v.m_conv(T_Key(key));
        if (ind >= 0)
            return T_Traits::map(valOrSz, ind);
    }
    return T_Traits::valueError();
}

template<class T>
auto &unlex(I_LexAttr &lex)
{
    return dynamic_cast<C_LexDataT<T>&>(lex).m_data;
}

template<class T>
auto &unlex(const I_LexAttr &lex)
{
    return dynamic_cast<const C_LexDataT<T>&>(lex).m_data;
}

template<class T_Data, class T_Lex, template<class> class C_Ptr>
auto &unlex(const C_Ptr<T_Lex> &lex)
{
    return dynamic_cast<T_CoConst<C_LexDataT<T_Data>,T_Lex>&>(*lex).m_data;
}

template<class T_Data, class T_Lex, template<class> class C_Ptr>
auto &unlex(const C_LexInfoT<T_Lex,C_Ptr> &lex)
{
    return unlex<T_Data>(lex.m_attr);
}

template<class T_Data, class T_Lex, template<class> class C_Ptr>
T_CoConst<T_Data,T_Lex> *tryUnlex(const C_Ptr<T_Lex> &lex)
{
    if (auto p = dynamic_cast<T_CoConst<C_LexDataT<T_Data>,T_Lex>*>(lex.get()))
        return &p->m_data;

    return nullptr;
}

template<class T_Data, class T_Lex, template<class> class C_Ptr>
auto tryUnlex(const C_LexInfoT<T_Lex,C_Ptr> &lex)
{
    return tryUnlex<T_Data>(lex.m_attr);
}

template<class T_Lex, template<class> class C_Ptr>
int toInt(const C_LexInfoT<T_Lex,C_Ptr> &lex)
{
    auto &t = dynamic_cast<const C_IntegerLex&>(*lex);
    return t.value<int>();
}

} //namespace bux

#endif // ParserBaseH
