#ifndef SyncStreamH
#define SyncStreamH

#include "Sync.h"       // bux::I_SyncOstream
#include <mutex>        // std::recursive_mutex

namespace bux {

//
//      Types
//
class C_FileLog;

class C_SyncOstream: public I_SyncOstream
/*! Thread-safe output stream wrapper. Apply bux::C_UseOstream to
    block any other thread from using it.
*/
{
public:

    // Nonvirtuals
    explicit C_SyncOstream(std::ostream &out);

    // Implement I_SyncOstream
    std::ostream &getResource() override;
    void releaseResource(std::ostream &out) override;

private: friend class C_FileLog;

    // Data - Original resource
    std::recursive_mutex   m_LockOut;
    std::ostream           &m_Out;
    size_t                 m_LockLevel{};
};

class C_UseOstream: public C_UseResource<std::ostream&>
/*! Helper class to use an output stream in the current thread and to block
    any other thread from using it in the mean time.
*/
{
public:

    // Nonvirtuals
    C_UseOstream(I_SyncOstream &obj);
    operator std::ostream &() const
        { return m_Resource; }
    template<class T>
    auto &operator<<(T &&t)
    /*! \param [in] t Data of type T to feed std::ostream.
        \return The underlying output stream. This can be useful to write multiple
         heterogeneous data in a single statement. See Example.

        Using prototype std::ostream &operator<<(const T &) in VC6 will cause
        internal compiler error. Live with it.

        Example:
        \code
        // Preconditions:
        //
        // ISyncOstream &m_Log;

        using namespace bux;
        m_hDispatchThread =beginThreadEx(DispatchNetMsgThread, this);
        if (!isValidThreadEx(m_hDispatchThread))
        {
            C_UseOstream(m_Log) <<"Fail to launch the dispatching thread\n";
        }
        \endcode
    */
        { return m_Resource <<std::forward<T>(t); }
    auto &stream() const
        { return m_Resource; }
};

struct C_UseTraceLog: C_UseOstream
/*! Specialized C_UseOstream to always log with timestamp and thread ID.
*/
{
    C_UseTraceLog(I_SyncOstream &obj);
};

} // namespace bux

#endif // SyncStreamH
