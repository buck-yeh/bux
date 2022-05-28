#pragma once

#include <string>       // std::string
#include <typeinfo>     // return type of typeid()
#include <stdexcept>    // std::exception

namespace bux {

//
//      Externs
//
const char *ord_suffix(size_t i);
std::string expand_env(const char *s);

std::string _HRTN(const char *originalName);
std::string OXCPT(const std::exception &e);

} // namespace bux

/*! HRTN stands for Human Readable Type Name
*/
#define HRTN(t) bux::_HRTN(typeid(t).name())
using bux::OXCPT;
