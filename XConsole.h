#ifndef XConsoleH
#define XConsoleH

#include <string>   // std::string

namespace bux {

//
//      Externals
//
void getKeyPunch(const std::string &msg, int &key, const char *keyFilter =0);
void pressAKey();
bool testWritability(const char *file);

} // namespace bux

#endif // XConsoleH
