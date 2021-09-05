#include "SyncLog.h"
#include "LogStream.h"  // bux::logTrace()
#include <ostream>      // std::ostream

namespace bux {

//
//      Implement Classes
//
std::ostream *C_SyncLogger::lockLog()
/*! Lock the logger without log level, for prefix-less log lines
*/
{
    m_lock.lock();
    if (const auto ret = m_impl.useLog()) [[likely]]
        return ret;

    m_lock.unlock();
    return nullptr;
}

std::ostream *C_SyncLogger::lockLog(E_LogLevel ll)
/*! \param [in] ll Log level

    Lock the logger with log level, for prefixed log lines
*/
{
    m_lock.lock();
    if (const auto ret = m_impl.useLog(ll)) [[likely]]
        return ret;

    m_lock.unlock();
    return nullptr;
}

void C_SyncLogger::unlockLog(bool flush)
/*! \param [in] flush Indicating if the logger should flush upon unlock.

    Unlock the logger, with or without log level.
*/
{
    m_impl.unuseLog(flush);
    m_lock.unlock();
}

C_UseTraceLog::C_UseTraceLog(I_SyncLog &obj, E_LogLevel level): C_UseLog(obj, level)
/*! \param obj Wrpper object representing the real logger.
    \param [in] level Log level
*/
{
    if (auto pout = stream())
        logTrace(*pout);
}

} // namespace bux
