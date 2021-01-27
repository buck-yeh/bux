#ifndef bux_ParserBase_H_
#define bux_ParserBase_H_

#include "LexBase.h"    // bux::T_LexID, bux::C_LexInfoT<>, ...
#include "LogLevel.h"   // bux::E_LogLevel
#include <algorithm>    // std::lower_bound()
#include <array>        // std::array<>
#include <functional>   // std::function<>
#include <limits>       // std::numeric_limits<>
#include <string_view>  // std::string_view
#include <type_traits>  // std::is_const<>, std::is_pointer<>, std::add_const_t<>

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

class C_ParserLogCount
{
public:

    // Nonvirtuals
    auto getCount(E_LogLevel ll) const { return m_count.at(ll); }
    void log(E_LogLevel ll, const C_SourcePos &pos, std::string_view message);

protected:

    // Data
    std::array<unsigned,5>  m_count{}; static_assert(LL_VERBOSE+1 == 5);

    // Virtuals
    virtual std::string toStr(const C_SourcePos &pos) const;
    virtual void println(const std::string &line) = 0;
};

class C_ParserOStreamCount: public C_ParserLogCount
{
public:

    // Ctor
    C_ParserOStreamCount(std::ostream &out): m_out(out) {}

private:

    // Data
    std::ostream &m_out;

    // Implement C_ParserLogCount
    void println(const std::string &line) override;
};

//
//      Function Templates
//
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

#endif // bux_ParserBase_H_
