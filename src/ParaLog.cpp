#include "ParaLog.h"

namespace bux {

//
//      Class Implementation
//
auto C_ParaLog::C_NodeArrayProxy::operator[](size_t i) const -> C_NodeProxy
{
    std::lock_guard _{*m_lock};
    if (i < m_nodePart->m_filteredNodes.size())
    {
        auto &dst = m_nodePart->m_filteredNodes.at(i);
        if (!dst.second)
            dst.second = std::make_unique<C_Node>();

        return {*m_lock, *dst.second};
    }
    return matchedNone_();
}

auto C_ParaLog::C_NodeArrayProxy::matchedNone_() const -> C_NodeProxy
{
    if (!m_nodePart->m_elseNode)
        m_nodePart->m_elseNode = std::make_unique<C_Node>();

    return {*m_lock, *m_nodePart->m_elseNode};
}

auto C_ParaLog::C_NodeArrayProxy::matchedNone() const -> C_NodeProxy
{
    std::lock_guard _{*m_lock};
    return matchedNone_();
}

void C_ParaLog::C_LockedNode::create_from(const C_Node &node, const std::function<std::ostream*(I_ReenterableLog&)> &to_log)
{
    for (auto &i: node.m_loggers)
    {
        if (const auto ret = to_log(*i))
            m_loggers.emplace_back(i.get(), ret);
    }
    for (auto &i: node.m_partitions)
    {
        auto &dstPart = m_partitions.emplace_back();
        for (auto &j: i.m_filteredNodes)
        {
            auto &dstPair = dstPart.m_filteredNodes.emplace_back(std::piecewise_construct, std::forward_as_tuple(j.first), std::forward_as_tuple());
            if (j.second)
                dstPair.second.create_from(*j.second, to_log);
        }

        if (i.m_elseNode)
            dstPart.m_elseNode.create_from(*i.m_elseNode, to_log);
        if (dstPart.m_elseNode.empty())
            while (dstPart.m_filteredNodes.back().second.empty())
                dstPart.m_filteredNodes.pop_back();
        if (dstPart.empty())
            m_partitions.pop_back();
    }
}

void C_ParaLog::C_LockedNode::log(std::string_view s, bool flush) const
{
    for (auto &i: m_loggers)
    {
        *i.second <<s;
        i.first->unuseLog(flush);
    }
    for (auto &i: m_partitions)
    {
        bool logged{};
        for (auto &j: i.m_filteredNodes)
        {
            if (j.first(s))
            {
                j.second.log(s, flush);
                logged = true;
                break;
            }
        }
        if (!logged)
            i.m_elseNode.log(s, flush);
    }
}

std::ostream *C_ParaLog::lockLog(const std::function<std::ostream*(I_ReenterableLog&)> &to_log)
{
    m_lock.lock();
    auto &dst = m_lockedStack.emplace_back();
    dst.first.create_from(m_root, to_log);
    if (!dst.first.empty())
        return &dst.second;

    m_lockedStack.pop_back();
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
    const auto &back = m_lockedStack.back();
    back.first.log(back.second.str(), flush);
    m_lockedStack.pop_back();
    m_lock.unlock();
}

} // namespace bux
