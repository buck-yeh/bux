#ifndef bux_Logger_H_
#define bux_Logger_H_

#include "LogLevel.h"       // bux::E_LogLevel
#include "SyncStream.h"     // bux::C_UseTraceLog
#include "XPlatform.h"      // CUR_FUNC_
#include <fmt/ostream.h>    // fmt::print() for std::ostream
#include <optional>         // std::optional<>
#include <string_view>      // std::string_view

namespace bux {

//
//      Types
//
class C_EntryLog
/*! \brief Log on both declaration point and end of block scope with an unique id
*/
{
public:

    // Nonvirtuals
    explicit C_EntryLog(std::string_view scopeName);
    template<class T_Fmt, class... T_Args> C_EntryLog(std::string_view scopeName, T_Fmt &&fmtStr, T_Args&&...args);
    ~C_EntryLog();

private:

    // Data
    std::optional<int>  m_Id;

    // Nonvirtuals
    static void deeper();
    static int getId();
};

struct C_UseLogger: C_UseTraceLog
{
    C_UseLogger(E_LogLevel level);
};

//
//      Externs
//
extern E_LogLevel g_LogLevel;

I_SyncOstream &logger();
bool shouldLog(E_LogLevel level);

//
//      Implement Class Member Templates
//
template<class T_Fmt, class... T_Args>
C_EntryLog::C_EntryLog(std::string_view scopeName, T_Fmt &&fmtStr, T_Args&&...args)
{
    deeper();
    if (bux::shouldLog(LL_VERBOSE))
    {
        m_Id = getId();
        fmt::print(C_UseLogger(LL_VERBOSE).stream(), fmt::format("@{}@{}({}) {{\n",*m_Id,scopeName,fmtStr), std::forward<T_Args>(args)...);
    }
}

namespace user {
I_SyncOstream &logger();    // provided by user of LOG(), FUNLOG(), SCOPELOG()
} // namespace User

} // namespace bux

//
//      Internal Macros
//
#define _gluePair_(x,y) x##y
#define SCOPELOG_(line,scope) bux::C_EntryLog _gluePair_(_log_,line)(scope)
#define SCOPELOGX_(line,scope,fmtStr, ...) bux::C_EntryLog _gluePair_(_log_,line)(scope,fmtStr, ##__VA_ARGS__)
#define DEF_LOGGER_HEAD_ namespace bux { namespace user { I_SyncOstream &logger() {
#define DEF_LOGGER_TAIL_(x) return x; }}}

//
//      End-User Macros
//
#define LOG(ll,fmtStr, ...) if (!shouldLog(ll)); else fmt::print(bux::C_UseLogger(ll).stream(),std::string(fmtStr)+'\n', ##__VA_ARGS__)
#define SCOPELOG(scope) SCOPELOG_(__LINE__,scope)
#define SCOPELOGX(scope,fmtStr, ...) SCOPELOGX_(__LINE__,scope,fmtStr, ##__VA_ARGS__)

#define DEF_LOGGER_OSTREAM(out) DEF_LOGGER_HEAD_ static C_SyncOstream l_{out}; DEF_LOGGER_TAIL_(l_)
#define DEF_LOGGER_COUT DEF_LOGGER_OSTREAM(std::cout)
#define DEF_LOGGER_CERR DEF_LOGGER_OSTREAM(std::cerr)

#define DEF_LOGGER_FILE(path) DEF_LOGGER_HEAD_                                                     \
    static std::ofstream out{path};                                                                \
    static C_SyncOstream l_{out};                                                                  \
    DEF_LOGGER_TAIL_(l_)

#define DEF_LOGGER_FILES(pathglob) DEF_LOGGER_HEAD_ static C_FileLog l_(pathglob); DEF_LOGGER_TAIL_(l_)

#define FUNLOG SCOPELOG(CUR_FUNC_)
#define FUNLOGX(fmtStr, ...) SCOPELOGX(CUR_FUNC_,fmtStr, ##__VA_ARGS__)

#endif // bux_Logger_H_
