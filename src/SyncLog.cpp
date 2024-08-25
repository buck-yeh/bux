#include "SyncLog.h"
#include <ostream>      // std::ostream

namespace bux {

//
//      Implement Classes
//
std::ostream *C_SyncLogger::lockLog()
/*! \return the std::ostream representitive of the locked logger

    Lock the logger without log level, for prefix-less log lines
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
    \return the std::ostream representitive of the locked logger

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

} // namespace bux
