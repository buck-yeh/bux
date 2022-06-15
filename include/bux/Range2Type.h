#pragma once

#include "StrUtil.h"    // HRTN()
#include <concepts>     // std::integral<>
#include <limits>       // std::numeric_limits<>
#include <cstdint>      // std::uint8_t, std::uint16_t, std::uint32_t

namespace bux {

//
//      Fnction Templates
//
template<std::integral T>
std::string fittestType(T lb, T ub)
/* \param lb Lower bound of value range
   \param ub Upper bound of value range
   \pre <tt> lb <= ub </tt>
   \return Name of the fittest type
*/
{
    if constexpr (std::numeric_limits<T>::is_signed)
    {
        if (std::numeric_limits<std::int8_t>::min() <= lb &&
            ub <= std::numeric_limits<std::int8_t>::max())
            return "int8_t";

        if constexpr (sizeof(T) >= sizeof(std::int16_t))
        {
            if (std::numeric_limits<std::int16_t>::min() <= lb &&
                ub <= std::numeric_limits<std::int16_t>::max())
                return "int16_t";
        }

        if constexpr (sizeof(T) >= sizeof(std::int32_t))
        {
            if (std::numeric_limits<std::int32_t>::min() <= lb &&
                ub <= std::numeric_limits<std::int32_t>::max())
                return "int32_t";
        }

        // There could be more ..
    }
    else
    {
        if (ub <= (T)std::numeric_limits<std::uint8_t>::max())
            return "uint8_t";

        if constexpr (sizeof(T) >= sizeof(std::uint16_t))
        {
            if (ub <= (T)std::numeric_limits<std::uint16_t>::max())
                return "uint16_t";
        }

        if constexpr (sizeof(T) >= sizeof(std::uint32_t))
        {
            if (ub <= (T)std::numeric_limits<std::uint32_t>::max())
                return "uint32_t";
        }

        // There could be more ..
    }
    return HRTN(T);
}

} //namespace bux
