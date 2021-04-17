#pragma once

#include <atomic>   // std::atomic_flag
#include <concepts> // std::constructible_from<>, std::invocable<>
#include <map>      // std::map<>
#include <thread>   // std::this_thread::yield()

namespace bux {

//
//      Types
//
class C_SpinLock
{
public:

    // Ctor/Dtor
    C_SpinLock(std::atomic_flag &lock_);
    ~C_SpinLock() { unlock(); }
    void operator=(const C_SpinLock&) = delete;
    void unlock();

private:

    // Data
    std::atomic_flag    *m_lock;
};

template<typename T_Key, typename T_Value, bool YIELD_BEFORE_RETRY = false>
class C_SpinCacheT
{
public:

    // Nonvirtuals
    template<class T_KeyIn, class F>
    const T_Value &operator()(T_KeyIn key, F set_value) requires
        std::constructible_from<T_Key,T_KeyIn> && std::invocable<F,T_Value&>
    {
        bux::C_SpinLock  lockMap{m_lock};
        const auto ins_ret = m_map.try_emplace(key);
        auto &ret = ins_ret.first->second;
        lockMap.unlock();
        if (ins_ret.second)
        {
            set_value(ret.m_value);
            ret.m_ready = true;
        }
        else while (!ret.m_ready)
        {
            if constexpr (YIELD_BEFORE_RETRY)
                std::this_thread::yield();
        }
        return ret.m_value;
    }
    auto size() const
    {
        bux::C_SpinLock  _{m_lock};
        return m_map.size();
    }

private:

    // Types
    struct C_Mapped
    {
        T_Value                 m_value;
        std::atomic<bool>       m_ready{false};  // oneway: false -> true
    };

    // Data
    std::map<T_Key, C_Mapped>   m_map;
    std::atomic_flag            mutable m_lock = ATOMIC_FLAG_INIT;
};

} // namespace bux
