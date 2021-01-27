#ifndef bux_LogLevel_H_
#define bux_LogLevel_H_

namespace bux {

//
//      Types
//
enum E_LogLevel
{
    LL_FATAL,
    LL_ERROR,
    LL_WARNING,
    LL_INFO,
    LL_VERBOSE
};

} // namespace bux

using bux::LL_FATAL;
using bux::LL_ERROR;
using bux::LL_WARNING;
using bux::LL_INFO;
using bux::LL_VERBOSE;

#endif // bux_LogLevel_H_
