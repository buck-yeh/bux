#ifndef ImplGLRH
#define ImplGLRH

/*! \file
    This header is constantly included by ParserGen-generated *.cpp files.
*/

#include "ParserBase.h" // bux::FC_GetRelLexT<>, LexBase.h
#include <memory>       // std::shared_ptr<>
#include <vector>       // std::vector<>

namespace bux {
namespace GLR {

//
//      Types
//
typedef FC_GetRelLexT<const I_LexAttr,std::shared_ptr> FC_GetRelLex;
typedef C_RetLvalT<const I_LexAttr,std::shared_ptr> C_RetLval;

//
//      Function Templates
//
using bux::index2value;

template<class T_Key, class T_Value, class T_ValueOrSize, class T_Traits>
std::vector<T_Value> index2values(U_K2V<T_Key,T_Value> k2v, T_ValueOrSize valOrSz, T_LexID key)
{
    // Constraints
    static_assert(std::numeric_limits<T_ValueOrSize>::is_signed && sizeof(T_Value) <= sizeof(T_ValueOrSize),
        "T_Value/T_ValueOrSize criteria");

    std::vector<T_Value> ret;
    if (valOrSz < 0)
        // (-valOrSz) is size of index array
    {
        const auto end = k2v.m_table - valOrSz;
        auto found = std::lower_bound(k2v.m_table, end, key,
            [](C_KVPair<T_Key,T_Value> a, T_LexID b)->bool {
                return a.m_key < b;
            });
        while (found < end && found->m_key == key)
        {
            ret.emplace_back(found->m_value);
            ++found;
        }
    }
    else
        //  valOrSz is THE value
    {
        const auto ind = k2v.m_conv(T_Key(key));
        if (ind >= 0)
            ret.emplace_back(T_Traits::map(valOrSz, ind));
    }
    return ret;
}

template<class T>
std::shared_ptr<C_LexDataT<T>> dupLex(const I_LexAttr &lex)
{
    return std::make_shared<C_LexDataT<T>>(unlex<T>(lex));
}

template<class T>
std::shared_ptr<C_LexDataT<T>> tryDupLex(const std::shared_ptr<const I_LexAttr> &lex)
{
    if (auto t = tryUnlex<T>(lex))
        return std::make_shared<C_LexDataT<T>>(*t);

    return {};
}

template<class T>
std::shared_ptr<C_LexDataT<T>> tryDupLex(const C_LexInfoT<const I_LexAttr,std::shared_ptr> &lex)
{
    if (auto t = tryUnlex<T>(lex))
        return std::make_shared<C_LexDataT<T>>(*t);

    return {};
}

} // namespace GLR
} //namespace bux

#endif // ImplGLRH
