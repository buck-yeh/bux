#pragma once

#include <limits>       // std::numeric_limits<>

namespace bux {

//
//      Types
//
namespace Helper_ {

template<bool FLAG_signed, class T>
struct C_NoWrap
{
    static T add(T a, T b);
};

template<bool FLAG_integer, bool FLAG_signed, bool FLAG_overflow, class T>
struct C_CmpRet
{
    static inline int cast(T a)
    { return a < T()? -1: a > T() ?1: 0; }
};

} // namespace Helper_

//
//      Function Templates
//
template<class T>
inline T addNoWrap(T a, T b)
{
    return Helper_::C_NoWrap<std::numeric_limits<T>::is_signed,T>::add(a, b);
}

template<class T>
inline int compareReturn(T a)
{
    return Helper_::C_CmpRet<
            std::numeric_limits<T>::is_integer,
            std::numeric_limits<T>::is_signed,
            (std::numeric_limits<T>::radix == std::numeric_limits<int>::radix &&
            std::numeric_limits<T>::digits > std::numeric_limits<int>::digits),
            T>::cast(a);
}

//
//      Specialize class templates
//
namespace Helper_ {

template<class T>
struct C_NoWrap<true,T>
{
    static T add(T a, T b)
    {
        const bool aneg =a < T();
        const bool bneg =b < T();
        if (aneg != bneg)
            return T(a +b);

        if (aneg)
            return  std::numeric_limits<T>::min() -a < b? T(a +b):
                    std::numeric_limits<T>::min();

        // a == 0 || b == 0 is considered a rare case
        return  std::numeric_limits<T>::max() -a > b? T(a +b):
                std::numeric_limits<T>::max();
    }
};

template<class T>
struct C_NoWrap<false,T>
{
    static T add(T a, T b)
    {
        return  T(std::numeric_limits<T>::max() -a) > b? T(a +b):
                std::numeric_limits<T>::max();
    }
};

template<bool FLAG_signed, class T>
struct C_CmpRet<true,FLAG_signed,false,T>
{
    static inline int cast(T a)
    { return int(a); }
};

template<bool FLAG_integer, bool FLAG_overflow, class T>
struct C_CmpRet<FLAG_integer,false,FLAG_overflow,T>
{
    static inline int cast(T a)
    { return a != T()? 1: 0; }
};

} // namespace Helper_

} //namespace bux
