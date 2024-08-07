#pragma once

#include "SyncLog.h"        // bux::I_SyncLog, bux::C_UseLog, bux::C_UseTraceLog
#include "XPlatform.h"      // CUR_FUNC_
#include <format>           // std::format(), std::vformat(), std::make_format_args()
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
I_SyncLog &logger();

//
//      Implement Class Member Templates
//
template<class T_Fmt, class... T_Args>
C_EntryLog::C_EntryLog(std::string_view scopeName, T_Fmt &&fmtStr, T_Args&&...args)
{
    if (C_UseLogger u{LL_VERBOSE})
    {
        m_Id = getId();
        const auto fmtfmt = std::format("@{}@{}({}) {{{{\n", *m_Id, scopeName, fmtStr);
        *u << std::vformat(fmtfmt, std::make_format_args(std::forward<T_Args>(args)...));
    }
    deeper();
}

namespace user {
I_SyncLog &logger();    // provided by user of LOG(), FUNLOG(), SCOPELOG()
} // namespace User

} // namespace bux

//
//      Internal Macros
//
#define _gluePair_(x,y) x##y
#define SCOPELOG_(line,scope) bux::C_EntryLog _gluePair_(_log_,line)(scope)
#define SCOPELOGX_(line,scope,fmtStr, ...) bux::C_EntryLog _gluePair_(_log_,line)(scope,fmtStr, ##__VA_ARGS__)
#define DEF_LOGGER_HEAD_ namespace bux { namespace user { I_SyncLog &logger() {
#define DEF_LOGGER_TAIL_(x) return x; }}}

//
//      End-User Macros
//
#define LOG(ll,fmtStr, ...) do if (bux::C_UseLogger u{ll}) *u <<std::format(fmtStr, ##__VA_ARGS__) <<'\n'; while(false)
#define LOG_RAW(fmtStr, ...) do if (bux::C_UseLog u{bux::logger()}) *u <<std::format(fmtStr, ##__VA_ARGS__) <<'\n'; while(false)
#define SCOPELOG(scope) SCOPELOG_(__LINE__,scope)
#define SCOPELOGX(scope,fmtStr, ...) SCOPELOGX_(__LINE__,scope,fmtStr, ##__VA_ARGS__)

#define DEF_LOGGER_OSTREAM(out, ...) DEF_LOGGER_HEAD_ \
    static C_ReenterableOstream ro_{out, ##__VA_ARGS__}; \
    static C_SyncLogger l_{ro_}; \
    DEF_LOGGER_TAIL_(l_)

// #include <iotream> before using either of these but never both
#define DEF_LOGGER_COUT(...) DEF_LOGGER_OSTREAM(std::cout, ##__VA_ARGS__)
#define DEF_LOGGER_CERR(...) DEF_LOGGER_OSTREAM(std::cerr, ##__VA_ARGS__)

// #include <fstream> before using this
#define DEF_LOGGER_FILE(path, ...) \
    DEF_LOGGER_HEAD_ \
    static std::ofstream out{path}; \
    static C_ReenterableOstream ro_{out, ##__VA_ARGS__}; \
    static C_SyncLogger l_{ro_}; \
    DEF_LOGGER_TAIL_(l_)

// #include <bux/FileLog.h> before using this
#define DEF_LOGGER_FILES(pathfmt, ...) \
    DEF_LOGGER_HEAD_ \
    static C_PathFmtLogSnap snap_{pathfmt}; \
    static C_ReenterableOstreamSnap ros_{snap_, ##__VA_ARGS__}; \
    static C_SyncLogger l_{ros_}; \
    DEF_LOGGER_TAIL_(l_)

// #include <bux/FileLog.h> before using this
#define DEF_FALLBACKABLE_LOGGER_FILES(first,...) \
    namespace bux { namespace user {  \
    C_PathFmtLogSnap g_snap{first, ##__VA_ARGS__}; \
    C_ReenterableOstreamSnap g_ros{g_snap}; \
    I_SyncLog &logger() { \
    static C_SyncLogger l_{g_ros}; \
    DEF_LOGGER_TAIL_(l_)

// #include <bux/ParaLog.h> before using this
#define DEF_PARA_LOGGER \
    namespace bux { namespace user {  \
    C_ParaLog g_paraLog; \
    I_SyncLog &logger() { \
    DEF_LOGGER_TAIL_(g_paraLog)

#define FUNLOG SCOPELOG(CUR_FUNC_)
#define FUNLOGX(fmtStr, ...) SCOPELOGX(CUR_FUNC_,fmtStr, ##__VA_ARGS__)
