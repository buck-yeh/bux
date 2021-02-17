#ifndef bux_StrUtil_H_
#define bux_StrUtil_H_

#include <string>       // std::string

namespace bux {

//
//      Externs
//
const char *ord_suffix(size_t i);
std::string expand_env(const char *s);

} // namespace bux

#endif  // bux_StrUtil_H_
