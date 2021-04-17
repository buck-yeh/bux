#pragma once

#include <string>       // std::string

namespace bux {

//
//      Externs
//
const char *ord_suffix(size_t i);
std::string expand_env(const char *s);

} // namespace bux
