#pragma once

#include "SyncLog.h"        // bux::I_SyncLog, bux::C_UseLog
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

//
//      Externs
//
I_SyncLog &logger();
std::ostream &stamp(const C_UseLog &u, E_LogLevel level);

//
//      Implement Class Member Templates
//
template<class T_Fmt, class... T_Args>
C_EntryLog::C_EntryLog(std::string_view scopeName, T_Fmt &&fmtStr, T_Args&&...args)
{
    if (C_UseLog u{logger()})
    {
        m_Id = getId();
        const auto fmtfmt = std::format("@{}@{}({}) {{{{\n", *m_Id, scopeName, fmtStr);
        stamp(u,LL_VERBOSE) << std::vformat(fmtfmt, std::make_format_args(args...));
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

#ifndef TURN_OFF_LOGGER_
#define SCOPELOG_(line,scope) bux::C_EntryLog _gluePair_(_log_,line)(scope)
#define SCOPELOGX_(line,scope,fmtStr, ...) bux::C_EntryLog _gluePair_(_log_,line)(scope,fmtStr, ##__VA_ARGS__)
#define DEF_LOGGER_HEAD_ namespace bux { namespace user { I_SyncLog &logger() {
#define DEF_LOGGER_TAIL_(x) return x; }}}

//
//      End-User Macros
//
#define LOG(ll,fmtStr, ...) do if (bux::C_UseLog u{bux::logger(),ll}) stamp(u,ll) <<std::format(fmtStr, ##__VA_ARGS__) <<'\n'; while(false)
#define LOG_RAW(fmtStr, ...) do if (bux::C_UseLog u{bux::logger()}) *u <<std::format(fmtStr, ##__VA_ARGS__) <<'\n'; while(false)

#ifndef LOGGER_USE_LOCAL_TIME_
#define LOGGER_USE_LOCAL_TIME_ true
/*! \def LOGGER_USE_LOCAL_TIME_
    Valid definitions: (\c #define before including this header)
    - Local Time
        ~~~cpp
        #define LOGGER_USE_LOCAL_TIME_ true
        #define LOGGER_USE_LOCAL_TIME_ std::chrono::get_tzdb().current_zone()
        #define LOGGER_USE_LOCAL_TIME_ std::chrono::get_tzdb().locate_zone("Asia/Taipei")
        ~~~
    - System Clock
        ~~~cpp
        #define LOGGER_USE_LOCAL_TIME_ false
        #define LOGGER_USE_LOCAL_TIME_ nullptr
        ~~~
*/
#endif

#define DEF_LOGGER_OSTREAM(out, ...) DEF_LOGGER_HEAD_ \
    static C_ReenterableOstream ro_{out, ##__VA_ARGS__}; \
    static C_SyncLogger l_{ro_, LOGGER_USE_LOCAL_TIME_}; \
    DEF_LOGGER_TAIL_(l_)

// #include <iotream> before using either of these but never both
#define DEF_LOGGER_COUT(...) DEF_LOGGER_OSTREAM(std::cout, ##__VA_ARGS__)
#define DEF_LOGGER_CERR(...) DEF_LOGGER_OSTREAM(std::cerr, ##__VA_ARGS__)

// #include <fstream> before using this
#define DEF_LOGGER_FILE(path, ...) \
    DEF_LOGGER_HEAD_ \
    static std::ofstream out{path}; \
    static C_ReenterableOstream ro_{out, ##__VA_ARGS__}; \
    static C_SyncLogger l_{ro_, LOGGER_USE_LOCAL_TIME_}; \
    DEF_LOGGER_TAIL_(l_)

// #include <bux/FileLog.h> before using this
#define DEF_LOGGER_FILES(pathfmt, ...) \
    DEF_LOGGER_HEAD_ \
    static C_PathFmtLogSnap snap_{LOGGER_USE_LOCAL_TIME_}; \
    static C_ReenterableOstreamSnap ros_{snap_.configPath(pathfmt), ##__VA_ARGS__}; \
    static C_SyncLogger l_{ros_, LOGGER_USE_LOCAL_TIME_}; \
    DEF_LOGGER_TAIL_(l_)

// #include <bux/FileLog.h> before using this
#define DEF_FALLBACK_LOGGER_FILES(fsize_in_bytes, fallbackPaths) \
    namespace bux { namespace user {  \
    C_PathFmtLogSnap g_snap{LOGGER_USE_LOCAL_TIME_}; \
    C_ReenterableOstreamSnap g_ros{g_snap.configPath(fsize_in_bytes, fallbackPaths)}; \
    I_SyncLog &logger() { \
    static C_SyncLogger l_{g_ros, LOGGER_USE_LOCAL_TIME_}; \
    DEF_LOGGER_TAIL_(l_)

// #include <bux/ParaLog.h> before using this
#define DEF_PARA_LOGGER \
    namespace bux { namespace user {  \
    C_ParaLog g_paraLog(LOGGER_USE_LOCAL_TIME_); \
    I_SyncLog &logger() { \
    DEF_LOGGER_TAIL_(g_paraLog)
#else
#define SCOPELOG_(line,scope)
#define SCOPELOGX_(line,scope,fmtStr, ...)
#define LOG(ll,fmtStr, ...)
#define LOG_RAW(fmtStr, ...)
#define DEF_LOGGER_OSTREAM(out, ...)
#define DEF_LOGGER_FILE(path, ...)
#define DEF_LOGGER_FILES(pathfmt, ...)
#define DEF_FALLBACK_LOGGER_FILES(fsize_in_bytes, fallbackPaths)
#define DEF_PARA_LOGGER
#endif // TURN_OFF_LOGGER_

// #include <iotream> before using either of these but never both
#define DEF_LOGGER_COUT(...) DEF_LOGGER_OSTREAM(std::cout, ##__VA_ARGS__)
#define DEF_LOGGER_CERR(...) DEF_LOGGER_OSTREAM(std::cerr, ##__VA_ARGS__)

#define SCOPELOG(scope) SCOPELOG_(__LINE__,scope)
#define SCOPELOGX(scope,fmtStr, ...) SCOPELOGX_(__LINE__,scope,fmtStr, ##__VA_ARGS__)
#define FUNLOG SCOPELOG(CUR_FUNC_)
#define FUNLOGX(fmtStr, ...) SCOPELOGX(CUR_FUNC_,fmtStr, ##__VA_ARGS__)
