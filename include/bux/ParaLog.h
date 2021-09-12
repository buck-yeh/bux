#pragma once

#include "SyncLog.h"    // bux::I_SyncLog, bux::I_ReenterableLog, bux::C_ReenterableOstream
#include <concepts>     // std::derived_from<>
#include <functional>   // std::function<>
#include <list>         // std::list<>
#include <memory>       // std::unique_ptr<>
#include <sstream>      // std::ostringstream
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

    // Types
    using T_ChildLogger = std::unique_ptr<I_ReenterableLog>;

    // Nonvirtuals
    template<std::derived_from<I_ReenterableLog> T>
    bool addChild(std::unique_ptr<T> &&snap)
    {
        std::lock_guard _{m_lock};
        if (snap)
        {
            m_children.emplace_back(std::move(snap));
            return true;
        }
        return false;
    }
    bool addChild(std::ostream &out, E_LogLevel ll = LL_VERBOSE)
    {
        return addChild(std::make_unique<C_ReenterableOstream>(out, ll));
    }
    bool addChild(I_SnapT<std::ostream*> &snap, E_LogLevel ll = LL_VERBOSE)
    {
        return addChild(std::make_unique<C_ReenterableOstreamSnap>(snap, ll));
    }
    template<class C_SinkImpl, class C_Holder = typename C_AutoSinkHolderT<C_SinkImpl>::type, E_LogLevel LL = LL_VERBOSE, class...T_Args>
    bool addChildT(T_Args&&...args)
    {
        return addChild(std::make_unique<C_ReenterableLoggerInside<C_SinkImpl,C_Holder>>(LL, std::forward<T_Args>(args)...));
    }

    // Implement I_SyncLog
    std::ostream *lockLog() override;
    std::ostream *lockLog(E_LogLevel ll) override;
    void unlockLog(bool flush) override;

private:

    // Types
    using C_UsedPairs = std::vector<std::pair<I_ReenterableLog*,std::ostream*>>;

    // Data
    std::recursive_mutex                            m_lock;
    std::vector<std::unique_ptr<I_ReenterableLog>>  m_children;
    std::list<std::pair<C_UsedPairs,std::ostringstream>> m_lockedStack;

    // Nonvirtuals
    std::ostream *lockLog(const std::function<std::ostream*(I_ReenterableLog&)> &to_log);
};

} // namespace bux
