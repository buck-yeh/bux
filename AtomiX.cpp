#include "AtomiX.h"

namespace bux {

//
//      Implement Classes
//
C_SpinLock::C_SpinLock(std::atomic_flag &lock_): m_lock(&lock_)
{
    while (lock_.test_and_set(std::memory_order_acquire))  // acquire lock
#ifdef __cpp_lib_atomic_flag_test
         while (lock_.test(std::memory_order_relaxed))     // test lock
#endif
             ; // spin
}

void C_SpinLock::unlock()
{
    if (m_lock)
    {
        m_lock->clear(std::memory_order_release); // release lock
        m_lock = nullptr;
    }
}

} // namespace bux
