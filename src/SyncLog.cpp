#include "SyncLog.h"
#include "LogStream.h"  // bux::logTrace()
#include <ostream>      // std::ostream

namespace bux {

//
//      Implement Classes
//
std::ostream *C_SyncLogger::lockLog()
{
    m_lock.lock();
    if (const auto ret = m_impl.useLog()) [[likely]]
        return ret;

    m_lock.unlock();
    return nullptr;
}

std::ostream *C_SyncLogger::lockLog(E_LogLevel ll)
{
    m_lock.lock();
    if (const auto ret = m_impl.useLog(ll)) [[likely]]
        return ret;

    m_lock.unlock();
    return nullptr;
}

void C_SyncLogger::unlockLog(bool flush)
{
    m_impl.unuseLog(flush);
    m_lock.unlock();
}

C_UseTraceLog::C_UseTraceLog(I_SyncLog &obj, E_LogLevel level): C_UseLog(obj, level)
/*! \param obj Wrpper object representing the real logger.
*/
{
    if (auto pout = stream())
        logTrace(*pout);
}

} // namespace bux
