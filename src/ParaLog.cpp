#include "ParaLog.h"

namespace bux {

//
//      Class Implementation
//
std::ostream *C_ParaLog::lockLog(const std::function<std::ostream*(I_ReenterableLog&)> &to_log)
{
    m_lock.lock();
    C_UsedPairs used_logs;
    for (auto &i: m_children)
    {
        if (const auto ret = to_log(*i))
            used_logs.emplace_back(i.get(), ret);
    }
    if (!used_logs.empty())
        return &m_lockedStack.emplace_back(std::move(used_logs), std::string{}).second;

    m_lock.unlock();
    return nullptr;
}

std::ostream *C_ParaLog::lockLog()
{
    return lockLog([](auto &child){ return child.useLog(); });
}

std::ostream *C_ParaLog::lockLog(E_LogLevel ll)
{
    return lockLog([ll](auto &child){ return child.useLog(ll); });
}

void C_ParaLog::unlockLog(bool flush)
{
    {
        const auto &back = m_lockedStack.back();
        const auto content = back.second.str();
        for (auto &i: back.first)
        {
            *i.second <<content;
            i.first->unuseLog(flush);
        }
    }
    m_lockedStack.pop_back();
    m_lock.unlock();
}

} // namespace bux
