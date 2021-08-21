#include "Logger.h"
#include "AtomiX.h"     // bux::C_SpinLock
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
        *u <<fmt::format("@{}@{} {{\n", *m_Id, scopeName);
    }
    deeper();
}

C_EntryLog::~C_EntryLog()
{
    --g_EntryLevel;
    if (m_Id)
    {
        *C_UseLogger(LL_VERBOSE) <<fmt::format("@{}{}", *m_Id,
            []{
                if (auto n = std::uncaught_exceptions())
                    return fmt::format("@}} due to {} uncaught exceptions\n", n);

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

C_UseLogger::C_UseLogger(E_LogLevel level): C_UseTraceLog(logger(), level)
{
    if (auto pout = stream())
        *pout <<fmt::format("{}:{}", "FEWIV"[level], std::string(g_EntryLevel,'|'));
}

} // namespace bux
