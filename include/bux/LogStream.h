#pragma once

#include <iosfwd>       // fwrd decl std::ostream
#include <string>       // std::string
#include <typeinfo>     // return type of typeid()
#include <stdexcept>    // std::exception

namespace bux {

//
//      Externals
//
std::ostream &timestamp(std::ostream &); // manipulator
std::ostream &logTrace(std::ostream &);  // manipulator

std::string _HRTN(const char *originalName);
std::string OXCPT(const std::exception &e);

} // namespace bux

using bux::timestamp;

#define LOGTITLE(log) (bux::timestamp(log) <<__FILE__ <<'#' <<__LINE__ <<": ")

/*! HRTN stands for Human Readable Type Name
*/
#define HRTN(t) bux::_HRTN(typeid(t).name())
using bux::OXCPT;
