#pragma once

namespace bux {

//
//      Types
//
enum E_LogLevel
{
    LL_FATAL,   ///< Error serious enough that the program should shut down immediately after reporting this.
    LL_ERROR,   ///< Error not serious enough that the program can continue to run.
    LL_WARNING, ///< Situation that should be warned but should not have sabotaged anything already.
    LL_INFO,    ///< Information worth mentioning about the current status, be it normal or abnormal.
    LL_DEBUG,   ///< Debug only logs which can be suppressed easily for releases
    LL_VERBOSE  ///< More detailed or advanced information probably considered too much by some.
};

} // namespace bux

using bux::LL_FATAL;
using bux::LL_ERROR;
using bux::LL_WARNING;
using bux::LL_INFO;
using bux::LL_DEBUG;
using bux::LL_VERBOSE;
