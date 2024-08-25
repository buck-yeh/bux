#pragma once

#include <chrono>       // std::chrono::time_zone
#include <iosfwd>       // fwrd decl std::ostream

namespace bux {

//
//      Externals
//
std::ostream &timestamp(std::ostream &out, const std::chrono::time_zone *tz = nullptr);
std::ostream &logTrace(std::ostream &out, const std::chrono::time_zone *tz = nullptr);

} // namespace bux

#define LOGTITLE(log,tz) (bux::timestamp(log,tz) <<" " __FILE__ "#" <<__LINE__ <<": ")
