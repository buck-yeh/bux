#ifndef bux_XConsole_H_
#define bux_XConsole_H_

#include <string_view>  // std::string_view

namespace bux {

//
//      Externals
//
void getKeyPunch(std::string_view msg, int &key, const char *keyFilter =0);
void pressAKey();
bool testWritability(const char *file);

} // namespace bux

#endif // bux_XConsole_H_
