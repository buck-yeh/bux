#include "Logger.h"
#include "AtomiX.h"     // bux::C_SpinLock
#include "LogStream.h"  // bux::logTrace()
#include "XException.h" // RUNTIME_ERROR()

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
    auto &ret = user::logger();
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
                stamp(u, LL_VERBOSE) <<std::boolalpha <<
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

std::ostream &stamp(const C_UseLog &u, E_LogLevel level)
{
    constexpr static const char FEWIV[] = "FEWIV";
    static_assert(FEWIV[LL_FATAL]   == 'F');
    static_assert(FEWIV[LL_ERROR]   == 'E');
    static_assert(FEWIV[LL_WARNING] == 'W');
    static_assert(FEWIV[LL_INFO]    == 'I');
    static_assert(FEWIV[LL_VERBOSE] == 'V');
    if (auto pout = u.stream())
    {
        logTrace(*pout, u.timezone()) <<std::format("{}:{}", FEWIV[level], std::string(g_EntryLevel,'|'));
        return *pout;
    }
    RUNTIME_ERROR("Null stream from C_UseLog");
}

//
//      Implement Classes
//
C_EntryLog::C_EntryLog(std::string_view scopeName)
{
    if (C_UseLog u{logger()})
    {
        m_Id = getId();
        stamp(u,LL_VERBOSE) <<std::format("@{}@{} {{\n", *m_Id, scopeName);
    }
    deeper();
}

C_EntryLog::~C_EntryLog()
{
    --g_EntryLevel;
    if (m_Id)
    {
        if (C_UseLog u{logger()})
            stamp(u, LL_VERBOSE) << std::format("@{}{}", *m_Id, [] {
                if (auto n = std::uncaught_exceptions())
                    return std::format("@}} due to {} uncaught exception{}\n", n, (n > 1 ? "s" : ""));

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

} // namespace bux
