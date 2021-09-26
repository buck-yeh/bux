#pragma once

#include "SyncLog.h"    // bux::I_SyncLog, bux::I_ReenterableLog, bux::C_ReenterableOstream
#include <concepts>     // std::derived_from<>, std::convertible_to<>
#include <functional>   // std::function<>
#include <list>         // std::list<>
#include <memory>       // std::unique_ptr<>
#include <sstream>      // std::ostringstream
#include <string_view>  // std::string_view
#include <vector>       // std::vector<>

namespace bux {

//
//      Types
//
class C_ParaLog: public I_SyncLog
/*! Thread-safe file log which can be configured to automatically change the
    output path according to the current timestamp. Apply bux::C_UseOstream
    to block any other thread WITHIN process from using it.
*/
{
public:

    // Data
    class C_NodeProxy;
    class C_NodeArrayProxy;

    // Nonvirtuals
    template<class...T_Args>
    bool addChild(T_Args&&...args);
    template<class C_LogImpl, class C_Holder = typename C_AutoSinkHolderT<C_LogImpl>::type, E_LogLevel LL = LL_VERBOSE, class...T_Args>
    bool addChildT(T_Args&&...args);
    template<typename F>
    [[nodiscard]]C_NodeArrayProxy partitionBy(F f);

    // Implement I_SyncLog
    std::ostream *lockLog() override;
    std::ostream *lockLog(E_LogLevel ll) override;
    void unlockLog(bool flush) override;

private:

    // Types
    struct C_Node;

    using FC_Accept = std::function<bool(std::string_view)>;
    using C_NodePtr = std::unique_ptr<C_Node>;

    struct C_NodePartition
    {
        std::vector<std::pair<FC_Accept,C_NodePtr>> m_filteredNodes;
        C_NodePtr                                   m_elseNode;
    };

    struct C_Node
    {
        std::vector<std::unique_ptr<I_ReenterableLog>>  m_loggers;
        std::list<C_NodePartition>                      m_partitions;
    };
    friend class C_NodeProxy;

    struct C_LockedNodePartition;

    struct C_LockedNode
    {
        std::vector<std::pair<I_ReenterableLog*,std::ostream*>> m_loggers;
        std::vector<C_LockedNodePartition>                      m_partitions;

        void create_from(const C_Node &node, const std::function<std::ostream*(I_ReenterableLog&)> &to_log);
        bool empty() const { return m_loggers.empty() && m_partitions.empty(); }
        void log(std::string_view s, bool flush) const;
    };

    struct C_LockedNodePartition
    {
        std::vector<std::pair<const FC_Accept&,C_LockedNode>> m_filteredNodes;
        C_LockedNode                                    m_elseNode;

        bool empty() const { return m_filteredNodes.empty() && m_elseNode.empty(); }
    };

    // Data
    std::recursive_mutex    m_lock;
    C_Node                  m_root;
    std::list<std::pair<C_LockedNode,std::ostringstream>> m_lockedStack;

    // Nonvirtuals
    std::ostream *lockLog(const std::function<std::ostream*(I_ReenterableLog&)> &to_log);
};

class C_ParaLog::C_NodeArrayProxy
{
public:

    // Nonvirtuals
    C_NodeArrayProxy(std::recursive_mutex &lock, C_NodePartition &nodePart): m_lock(&lock), m_nodePart(&nodePart) {}
    [[nodiscard]]C_NodeProxy operator[](size_t i) const;
    [[nodiscard]]C_NodeProxy matchedNone() const;

private:

    // Data
    std::recursive_mutex    *m_lock;
    C_NodePartition         *m_nodePart;

    // Nonvirtuals
    C_NodeProxy matchedNone_() const;
};

class C_ParaLog::C_NodeProxy
{
public:

    // Nonvirtuals
    C_NodeProxy(std::recursive_mutex &lock, C_Node &node): m_lock(&lock), m_node(&node) {}
    template<std::derived_from<I_ReenterableLog> T>
    bool addChild(std::unique_ptr<T> &&snap) const
    {
        std::lock_guard _{*m_lock};
        if (snap)
        {
            m_node->m_loggers.emplace_back(std::move(snap));
            return true;
        }
        return false;
    }
    bool addChild(std::ostream &out, E_LogLevel ll = LL_VERBOSE) const
    {
        return addChild(std::make_unique<C_ReenterableOstream>(out, ll));
    }
    bool addChild(I_SnapT<std::ostream*> &snap, E_LogLevel ll = LL_VERBOSE) const
    {
        return addChild(std::make_unique<C_ReenterableOstreamSnap>(snap, ll));
    }
    template<class C_LogImpl, class C_Holder = typename C_AutoSinkHolderT<C_LogImpl>::type, E_LogLevel LL = LL_VERBOSE, class...T_Args>
    bool addChildT(T_Args&&...args) const
    {
        return addChild(std::make_unique<C_ReenterableLoggerInside<C_LogImpl,C_Holder>>(LL, std::forward<T_Args>(args)...));
    }
    [[nodiscard]]C_NodeArrayProxy partitionBy(std::convertible_to<FC_Accept> auto f) const
    {
        std::lock_guard _{*m_lock};
        auto &dst = m_node->m_partitions.emplace_back();
        dst.m_filteredNodes.emplace_back(std::piecewise_construct, std::forward_as_tuple(f), std::forward_as_tuple());
        return {*m_lock, dst};
    }
    template<typename Filters>
    [[nodiscard]]C_NodeArrayProxy partitionBy(Filters fs) const requires requires {
        { *std::begin(fs) }-> std::convertible_to<FC_Accept>;
        { std::size(fs) }-> std::convertible_to<size_t>;
        std::end(fs);
    }
    {
        std::lock_guard _{*m_lock};
        auto &dst = m_node->m_partitions.emplace_back();
        dst.m_filteredNodes.reserve(std::size(fs));
        for (auto &&i: fs)
            dst.m_filteredNodes.emplace_back(std::piecewise_construct, std::forward_as_tuple(i), std::forward_as_tuple());

        return {*m_lock, dst};
    }

private:

    // Data
    std::recursive_mutex    *m_lock;
    C_Node                  *m_node;
};

template<class...T_Args>
bool C_ParaLog::addChild(T_Args&&...args)
{
    return C_NodeProxy{m_lock,m_root}.addChild(std::forward<T_Args>(args)...);
}
template<class C_LogImpl, class C_Holder, E_LogLevel LL, class...T_Args>
bool C_ParaLog::addChildT(T_Args&&...args)
{
    return C_NodeProxy{m_lock,m_root}.addChildT<C_LogImpl,C_Holder,LL>(std::forward<T_Args>(args)...);
}

template<typename F>
auto C_ParaLog::partitionBy(F f)->C_NodeArrayProxy
{
    return C_NodeProxy{m_lock,m_root}.partitionBy(f);
}

} // namespace bux
