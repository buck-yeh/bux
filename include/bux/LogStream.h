#pragma once

#include <iosfwd>       // fwrd decl std::ostream

namespace bux {

//
//      Externals
//
std::ostream &timestamp(std::ostream &); // manipulator
std::ostream &logTrace(std::ostream &);  // manipulator

} // namespace bux

using bux::timestamp;

#define LOGTITLE(log) (bux::timestamp(log) <<__FILE__ <<'#' <<__LINE__ <<": ")
