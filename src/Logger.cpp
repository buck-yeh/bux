#include "Logger.h"
#include "AtomiX.h"     // bux::C_SpinLock
#include "LogStream.h"  // bux::logTrace()
#include <exception>    // std::uncaught_exceptions()

namespace {

//
//      In-Module Globals
//
thread_local size_t g_EntryLevel = 0;

} // namespace

namespace bux {

//
//      Globals
//
I_SyncLog &logger()
{
    auto        &ret = user::logger();
    static bool first = true;
    if (first)
    {
        static constinit std::atomic_flag lock = ATOMIC_FLAG_INIT;
        C_SpinLock  _(lock);
        if (first)
        {
            if (C_UseLog u{ret})
            {
                first = false;
                *u <<std::boolalpha <<
#ifndef _WIN32
                    "********** LOGS BEGUN **********\n";
#elif defined(_DEBUG)
                    "********** LOGS BEGUN ********** (Debug)\n";
#else
                    "********** LOGS BEGUN ********** (Release)\n";
#endif
            }
        }
    }
    return ret;
}

//
//      Implement Classes
//
C_EntryLog::C_EntryLog(std::string_view scopeName)
{
    if (C_UseLogger u{LL_VERBOSE})
    {
        m_Id = getId();
        *u <<std::format("@{}@{} {{\n", *m_Id, scopeName);
    }
    deeper();
}

C_EntryLog::~C_EntryLog()
{
    --g_EntryLevel;
    if (m_Id)
    {
        *C_UseLogger(LL_VERBOSE) <<std::format("@{}{}", *m_Id,
            []{
                if (auto n = std::uncaught_exceptions())
                    return std::format("@}} due to {} uncaught exception{}\n", n, (n>1?"s":""));

                return std::string{"@}\n"};
            }());
    }
}

void C_EntryLog::deeper()
{
    ++g_EntryLevel;
}

int C_EntryLog::getId()
{
    static std::atomic<int> id = ATOMIC_VAR_INIT(1);
    return id++;
}

C_UseLogger::C_UseLogger(E_LogLevel level): C_UseLog(logger())
/*! \param [in] level Log level
*/
{
    constexpr static const char FEWIV[] = "FEWIV";
    static_assert(FEWIV[LL_FATAL]   == 'F');
    static_assert(FEWIV[LL_ERROR]   == 'E');
    static_assert(FEWIV[LL_WARNING] == 'W');
    static_assert(FEWIV[LL_INFO]    == 'I');
    static_assert(FEWIV[LL_VERBOSE] == 'V');
    if (auto pout = stream())
        logTrace(*pout, m_obj.tz) <<std::format("{}:{}", FEWIV[level], std::string(g_EntryLevel,'|'));
}

} // namespace bux
