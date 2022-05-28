#pragma once

#include "StrUtil.h"    // HRTN()
#include <limits>       // std::numeric_limits<>
#include <cstdint>      // std::uint8_t, std::uint16_t, std::uint32_t

namespace bux {

//
//      Types
//
template<class T, bool IS_SIGNED>
struct C_FittestVeryLongType
{
    static std::string run(T lb, T ub);
};

template<class T>
struct C_FittestVeryLongType<T,true>
{
    static std::string run(T lb, T ub)
    {
        if (std::numeric_limits<std::int8_t>::min() <= lb &&
            ub <= std::numeric_limits<std::int8_t>::max())
            return "int8_t";
        if (std::numeric_limits<std::int16_t>::min() <= lb &&
            ub <= std::numeric_limits<std::int16_t>::max())
            return "int16_t";
        if (std::numeric_limits<std::int32_t>::min() <= lb &&
            ub <= std::numeric_limits<std::int32_t>::max())
            return "int32_t";
        return HRTN(T);
    }
};

template<class T>
struct C_FittestVeryLongType<T,false>
{
    static std::string run(T, T ub)
    {
        if (ub <= (T)std::numeric_limits<std::uint8_t>::max())
            return "uint8_t";
        if (ub <= (T)std::numeric_limits<std::uint16_t>::max())
            return "uint16_t";
        if (ub <= (T)std::numeric_limits<std::uint32_t>::max())
            return "uint32_t";
        return HRTN(T);
    }
};

template<class T, size_t N>
struct C_FittestTypeN
{
    static std::string run(T lb, T ub)
    {
        // (T(-1) < 0) as a substitute of numeric_limits<T>::is_signed
        return C_FittestVeryLongType<T,T(-1)<0>::run(lb, ub);
    }
};

template<class T>
struct C_FittestTypeN<T,1>
{
    static std::string run(T, T)
    {
        return HRTN(T);
    }
};

template<class T, bool IS_SIGNED>
struct C_FittestType2S
{
    static std::string run(T lb, T ub);
};

template<class T>
struct C_FittestType2S<T,true>
{
    static std::string run(T lb, T ub)
    {
        if (std::numeric_limits<std::int8_t>::min() <= lb &&
            ub <= std::numeric_limits<std::int8_t>::max())
            return "int8_t";
        return HRTN(T);
    }
};

template<class T>
struct C_FittestType2S<T,false>
{
    static std::string run(T, T ub)
    {
        if (ub <= (T)std::numeric_limits<std::uint8_t>::max())
            return "uint8_t";
        return HRTN(T);
    }
};

template<class T>
struct C_FittestTypeN<T,2>
{
    static std::string run(T lb, T ub)
    {
        // (T(-1) < 0) as a substitute of numeric_limits<T>::is_signed
        return C_FittestType2S<T,T(-1)<0>::run(lb, ub);
    }
};

template<class T, bool IS_SIGNED>
struct C_FittestType4S
{
    static std::string run(T lb, T ub);
};

template<class T>
struct C_FittestType4S<T,true>
{
    static std::string run(T lb, T ub)
    {
        if (std::numeric_limits<std::int8_t>::min() <= lb &&
            ub <= std::numeric_limits<std::int8_t>::max())
            return "int8_t";
        if (std::numeric_limits<std::int16_t>::min() <= lb &&
            ub <= std::numeric_limits<std::int16_t>::max())
            return "int16_t";
        return HRTN(T);
    }
};

template<class T>
struct C_FittestType4S<T,false>
{
    static std::string run(T, T ub)
    {
        if (ub <= (T)std::numeric_limits<std::uint8_t>::max())
            return "uint8_t";
        if (ub <= (T)std::numeric_limits<std::uint16_t>::max())
            return "uint16_t";
        return HRTN(T);
    }
};

template<class T>
struct C_FittestTypeN<T,4>
{
    static std::string run(T lb, T ub)
    {
        // (T(-1) < 0) as a substitute of numeric_limits<T>::is_signed
        return C_FittestType4S<T,T(-1)<0>::run(lb, ub);
    }
};

//
//      Fnction Templates
//
template<class T>
inline std::string fittestType(T lb, T ub)
{
    return C_FittestTypeN<T,sizeof(T)>::run(lb, ub);
}

} //namespace bux
