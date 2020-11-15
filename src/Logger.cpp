#include "Logger.h"
#include "AtomiX.h"     // bux::C_SpinLock
#include <exception>    // std::uncaught_exceptions()

namespace {

//
//      In-Module Globals
//
thread_local size_t g_EntryLevel =0;

} // namespace

namespace bux {

//
//      Globals
//
E_LogLevel g_LogLevel = LL_VERBOSE;  // default for now

I_SyncOstream &logger()
{
    static bool first = true;
    if (first)
    {
        static std::atomic_flag lock = ATOMIC_FLAG_INIT;
        C_SpinLock       _(lock);
        if (first)
        {
            C_UseOstream(user::logger()) <<std::boolalpha <<
#ifndef _WIN32
                "********** LOGS BEGUN **********\n";
#elif defined(_DEBUG)
                "********** LOGS BEGUN ********** (Debug)\n";
#else
                "********** LOGS BEGUN ********** (Release)\n";
#endif
            first = false;
        }
    }
    return user::logger();
}

bool shouldLog(E_LogLevel level)
/*! \param [in] level Tagged level to subsequent log data
    \retval true if log data of the specified \em level SHOULD be written to bux::logger()
    \retval false if it SHOULD NOT
*/
{
    return level <= g_LogLevel;
}

//
//      Implement Classes
//
C_EntryLog::C_EntryLog(const char *scopeName)
{
    if (bux::shouldLog(LL_VERBOSE))
    {
        m_ScopeName = scopeName;
        fmt::print(C_UseLogger(LL_VERBOSE).stream(), "@{}@{} {{\n", m_Id, m_ScopeName);
    }
    deeper();
}

C_EntryLog::~C_EntryLog()
{
    --g_EntryLevel;
    if (m_ScopeName)
        fmt::print(C_UseLogger(LL_VERBOSE).stream(), "@{}{}", m_Id,
            []{
                if (auto n = std::uncaught_exceptions())
                    return "@} due to "+std::to_string(n)+" uncaught exceptions\n";

                return std::string{"@}\n"};
            }());
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

C_UseLogger::C_UseLogger(E_LogLevel level): C_UseTraceLog(logger())
{
    fmt::print(m_Resource, "{}:{}", "FEWIV"[level], std::string(g_EntryLevel+1,'|'));
}

} // namespace bux
