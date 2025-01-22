#pragma once

#include "XPlatform.h"  // bux::T_LocalZone
#include <iosfwd>       // fwrd decl std::ostream

namespace bux {

//
//      Externals
//
std::ostream &timestamp(std::ostream &out, T_LocalZone tz = T_LocalZone());
std::ostream &logTrace(std::ostream &out, T_LocalZone tz = T_LocalZone());

} // namespace bux

#define LOGTITLE(log,tz) (bux::timestamp(log,tz) <<" " __FILE__ "#" <<__LINE__ <<": ")
