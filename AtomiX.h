#ifndef AtomiX_h_
#define AtomiX_h_

#include <atomic>       // std::atomic_flag
#include <functional>   // std::function<>
#include <map>          // std::map<>
#include <thread>       // std::this_thread::yield()

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
    const T_Value &operator()(const T_Key &key, const std::function<void(T_Value&)> &set_value)
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

#endif // AtomiX_h_
