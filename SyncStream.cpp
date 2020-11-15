#include "SyncStream.h"
#include "LogStream.h"  // bux::logTrace()
#include <ostream>      // std::ostream

namespace bux {

//
//      Class Implementations
//
C_SyncOstream::C_SyncOstream(std::ostream &out): m_Out(out)
/*! \param out -- Reference to the output stream which is going to be used by
     more than one thread.
*/
{
}

std::ostream &C_SyncOstream::getResource()
{
    m_LockOut.lock();
    ++m_LockLevel;
    return m_Out;
}

void C_SyncOstream::releaseResource(std::ostream &)
{
    if (m_LockLevel)
    {
        if (!--m_LockLevel)
            m_Out.flush();
    }
    m_LockOut.unlock();
}

C_UseOstream::C_UseOstream(I_SyncOstream &obj):
    C_UseResource<std::ostream&>(obj)
{
}

C_UseTraceLog::C_UseTraceLog(I_SyncOstream &obj): C_UseOstream(obj)
/*! \param obj Wrpper object representing the real output stream.
*/
{
    logTrace(m_Resource);
}

} // namespace bux
