#pragma once
#include <chrono>       // std::chrono::time_zone, std::chrono::get_tzdb()

#if defined(_MSC_VER)
#   define CUR_FUNC_ __FUNCTION__
//#   define CUR_FUNC_ __FUNCSIG__
#elif defined(__BORLANDC__)
#   define CUR_FUNC_ __FUNC__
#elif defined(__GNUC__)
#   define CUR_FUNC_ __PRETTY_FUNCTION__
#else
#   define CUR_FUNC_ "?func?"
#endif

namespace bux {

#ifndef _LIBCPP_HAS_NO_TIME_ZONE_DATABASE
using T_LocalZone = const std::chrono::time_zone *;
T_LocalZone local_zone() { return std::chrono::get_tzdb().current_zone(); }
#define LOCALZONE_IS_TIMEZONE (1)
#else
using T_LocalZone = bool;
consteval bool local_zone() { return true; }
#define LOCALZONE_IS_TIMEZONE (0)
#endif

} // namespace bux
