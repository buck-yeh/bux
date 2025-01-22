#pragma once

#include "LogLevel.h"   // E_LogLevel
#include "XPlatform.h"  // bux::T_LocalZone, bux::local_zone()
#include <concepts>     // std::convertible_to<>, std::derived_from<>
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
    virtual std::ostream *lockLog(E_LogLevel ll) = 0;
            ///< Return non-null pointer if logging is permitted for the given log level \em ll
    virtual void unlockLog(bool flush = true) = 0;
            ///< If the previous call to lockLog() returned null, the behavior is undefined.

    const T_LocalZone tz;

protected:

    I_SyncLog(T_LocalZone tz_): tz(tz_) {}
    ~I_SyncLog() = default;
        ///< Pointer deletion is not expected
};

struct I_ReenterableLog /// Thread-unsafe implementation is preferred for performance
{
    virtual ~I_ReenterableLog() = default;
            ///< Pointer deletion is hereby granted
    virtual std::ostream *useLog() = 0;
            ///< Return non-null pointer if possible
    virtual std::ostream *useLog(E_LogLevel ll) = 0;
            ///< Return non-null pointer if logging is permitted to log level \em ll
    virtual void unuseLog(bool flush) = 0;
            ///< If the previous call to lockLog() returned null, the behavior is undefined.
};

template<class C_SinkRefHolder> requires requires (C_SinkRefHolder holder)
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
    template<typename T>
    explicit C_ReenterableLogger(T &ref, E_LogLevel max_ll = LL_VERBOSE): m_refHolder(ref), m_maxLevel(max_ll)
    {
    }
    auto setLogLevel(E_LogLevel level)
    {
        auto ret = m_maxLevel;
        m_maxLevel = level;
        return ret;
    }
    auto lockedCount() const { return m_lockCount; }

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
    C_SinkRefHolder         m_refHolder;
    int                     m_lockCount{};
    E_LogLevel              m_maxLevel;
};

template<class C_LogImpl>
struct C_AutoSinkHolderT
{
    //using type = ...;
};

template<class C_LogImpl, class C_SinkRefHolder = typename C_AutoSinkHolderT<C_LogImpl>::type>
class C_ReenterableLoggerInside: private C_LogImpl, public C_ReenterableLogger<C_SinkRefHolder>
{
public:

    // Ctor
    template<class...T_Args>
    explicit C_ReenterableLoggerInside(E_LogLevel ll, T_Args&&...args):
        C_LogImpl(std::forward<T_Args>(args)...),
        C_ReenterableLogger<C_SinkRefHolder>(*static_cast<C_LogImpl*>(this), ll)
        {}
    C_LogImpl &impl() { return *this; }
};

class C_SyncLogger: public I_SyncLog
/*! Simplest thread-safe logger wrapper.
    Apply bux::C_UseLog to block any other thread from using it.
*/
{

public:

    // Nonvirtuals
    explicit C_SyncLogger(I_ReenterableLog &impl, T_LocalZone tz_ = T_LocalZone()): I_SyncLog(tz_), m_impl(impl) {}
#if LOCALZONE_IS_TIMEZONE
    explicit C_SyncLogger(I_ReenterableLog &impl, bool use_local_time):
        C_SyncLogger(impl, use_local_time? local_zone(): nullptr) {}
#endif

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
using C_ReenterableOstream = C_ReenterableLogger<C_OstreamHolder>;

template<std::derived_from<std::ostream> T_Sink>
struct C_AutoSinkHolderT<T_Sink>
{
    using type = C_OstreamHolder;
};

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
using C_ReenterableOstreamSnap = C_ReenterableLogger<C_PersistedSnapHolder>;

template<std::derived_from<I_SnapT<std::ostream*>> T_Sink>
struct C_AutoSinkHolderT<T_Sink>
{
    using type = C_PersistedSnapHolder;
};

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
    operator bool() const       { return m_locked != nullptr; }
    auto &operator*() const     { return *m_locked; }
    auto stream() const         { return m_locked; }
    auto timezone() const       { return m_obj.tz; }

private:

    // Data
    I_SyncLog       &m_obj;
    std::ostream    *const m_locked;
};

} // namespace bux
