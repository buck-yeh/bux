#pragma once

#include "SyncLog.h"    // bux::I_SyncLog, bux::I_ReenterableLog
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
    bool addChild(auto &&snap)
    {
        std::lock_guard _{m_lock};
        if (snap)
        {
            m_children.emplace_back(std::move(snap));
            return true;
        }
        return false;
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
