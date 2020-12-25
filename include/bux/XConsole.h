#ifndef bux_XConsole_H_
#define bux_XConsole_H_

#include <string>   // std::string

namespace bux {

//
//      Externals
//
void getKeyPunch(const std::string &msg, int &key, const char *keyFilter =0);
void pressAKey();
bool testWritability(const char *file);

} // namespace bux

#endif // bux_XConsole_H_
