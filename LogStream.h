#ifndef LogStreamH
#define LogStreamH

#include <iosfwd>       // fwrd decl std::ostream
#include <string>       // std::string
#include <typeinfo>     // return type of typeid()
#include <stdexcept>    // std::exception

namespace bux {

//
//      Externals
//
extern std::ostream &timestamp(std::ostream &);

std::string _HRTN(const char *originalName);
std::ostream &logTrace(std::ostream &);

std::string OXCPT(const std::exception &e);

} // namespace bux

using bux::timestamp;

#define LOGTITLE(log) (bux::timestamp(log) <<__FILE__ <<'#' <<__LINE__ <<": ")

/*! HRTN stands for Human Readable Type Name
*/
#define HRTN(t) bux::_HRTN(typeid(t).name())
using bux::OXCPT;

#endif // LogStreamH
