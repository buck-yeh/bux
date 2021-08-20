#pragma once

#include "LogLevel.h"   // E_LogLevel
#include <concepts>     // std::convertible_to<>
#include <mutex>        // std::recursive_mutex
#include <ostream>      // std::ostream

namespace bux {

//
//      Types
//
class I_SyncLog ///</// Thread safety is expected
{
public:

    virtual std::ostream *lockLog() = 0;
            ///< Return non-null pointer if possible
    virtual std::ostream *lockLog([[maybe_unused]]E_LogLevel ll) = 0;
            ///< Return non-null pointer if logging is permitted for the log level
    virtual void unlockLog(bool flush = true) = 0;
            ///< If the previous call to lockLog() returned null, the behavious is undefined.

protected:

    ~I_SyncLog() = default;
        ///< Pointer deletion is not expected
};

struct I_ReenterableLog /// Thread-unsafe implementaion is preferred for performance
{
    virtual ~I_ReenterableLog() = default;
            ///< Pointer deletion is hereby granted
    virtual std::ostream *useLog() = 0;
            ///< Return non-null pointer if possible
    virtual std::ostream *useLog(E_LogLevel ll) = 0;
            ///< Return non-null pointer if logging is permitted for the log level
    virtual void unuseLog(bool flush) = 0;
            ///< If the previous call to lockLog() returned null, the behavious is undefined.
};

template<typename T_Sink, typename T_SinkRefHolder> requires requires (T_SinkRefHolder holder)
    {
        { holder.stream() }-> std::convertible_to<std::ostream*>;
        holder.reset();
    }
class C_ReenterableLogger: public I_ReenterableLog
/*! Thread-unsafe logger wrapper, which is basic building block of bux::C_ParaLog
    Instead, the derived C_SyncLogger implements thread safety.
*/
{
public:

    // Nonvirtuals
    explicit C_ReenterableLogger(T_Sink &ref, E_LogLevel max_ll = LL_VERBOSE): m_refHolder(ref), m_maxLevel(max_ll)
    {
    }
    E_LogLevel setLogLevel(E_LogLevel level)
    {
        auto ret = m_maxLevel;
        m_maxLevel = level;
        return ret;
    }

    // Implement I_ReenterableLog
    std::ostream *useLog() override
    {
        if (const auto ret = m_refHolder.stream()) [[likely]]
        {
            ++m_lockCount;
            return ret;
        }
        return nullptr;
    }
    std::ostream *useLog(E_LogLevel ll) override
    {
        return ll <= m_maxLevel? useLog(): nullptr;
    }
    void unuseLog(bool flush) override
    {
        if (flush)
            m_refHolder.stream()->flush();

        if (!--m_lockCount) [[likely]]
            m_refHolder.reset();
    }

private:

    // Data
    T_SinkRefHolder         m_refHolder;
    int                     m_lockCount{};
    E_LogLevel              m_maxLevel;
};

class C_SyncLogger: public I_SyncLog
/*! Simplest thread-safe logger wrapper.
    Apply bux::C_UseLog to block any other thread from using it.
*/
{

public:

    // Nonvirtuals
    explicit C_SyncLogger(I_ReenterableLog &impl): m_impl(impl) {}

    // Implement I_SyncLog
    std::ostream *lockLog() override;
    std::ostream *lockLog(E_LogLevel ll) override;
    void unlockLog(bool flush) override;

private:

    // Data
    std::recursive_mutex    m_lock;
    I_ReenterableLog        &m_impl;
};

struct C_OstreamHolder
{
    // Data
    std::ostream    &m_out;

    // Nonvirtuals
    C_OstreamHolder(std::ostream &out): m_out(out) {}
    std::ostream *stream() const { return &m_out; }
    void reset() {}
};
using C_ReenterableOstream = C_ReenterableLogger<std::ostream, C_OstreamHolder>;

template<typename T>
struct I_SnapT         ///< Parasitic Type
{
    virtual ~I_SnapT() = default;
        ///< Pointer deletion is hereby granted
    virtual T snap() = 0;
        ///< Snap the current T value
};

class C_PersistedSnapHolder
{
public:

    // Nonvirtuals
    C_PersistedSnapHolder(I_SnapT<std::ostream*> &snap): m_snap(snap) {}
    std::ostream *stream() {
        if (!m_saved)
            m_saved = m_snap.snap();

        return m_saved;
    }
    void reset() { m_saved = nullptr; }

private:

    // Data
    I_SnapT<std::ostream*>  &m_snap;
    std::ostream            *m_saved{};
};
using C_ReenterableOstreamSnap = C_ReenterableLogger<I_SnapT<std::ostream*>, C_PersistedSnapHolder>;

class C_UseLog
/*! Helper class to use logger in the current thread while blocking any other thread from using it.
*/
{
public:

    // Nonvirtuals
    C_UseLog(I_SyncLog &obj): m_obj(obj), m_locked(obj.lockLog()) {}
    C_UseLog(I_SyncLog &obj, E_LogLevel level): m_obj(obj), m_locked(obj.lockLog(level)) {}
    ~C_UseLog()
    {
        if (m_locked)
            m_obj.unlockLog();
    }
    operator bool() const           { return m_locked != nullptr; }
    auto &operator*() const         { return *m_locked; }
    auto stream() const             { return m_locked; }

private:

    // Data
    I_SyncLog       &m_obj;
    std::ostream    *const m_locked;
};

struct C_UseTraceLog: C_UseLog
/*! Specialized C_UseLog to always log with timestamp and thread ID.
*/
{
    C_UseTraceLog(I_SyncLog &obj, E_LogLevel level);
};

} // namespace bux