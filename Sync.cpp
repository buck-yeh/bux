#include "Sync.h"
#include "XException.h" // TIMEOUT_ERROR

namespace bux {

//
//      Implement Classes
//
C_RefObject::C_RefObject(size_t initRefCount):
    m_pOnZeroRef(0),
    m_RefCount(initRefCount),
    m_IncRefEnabled(true)
{
}

void C_RefObject::decRef()
{
    C_LockTillEnd guard(*this);
    if (!guard)
        TIMEOUT_ERROR

    if (m_RefCount)
    {
        if (!--m_RefCount)
        {
            onZeroRef();
            if (m_pOnZeroRef)
                m_pOnZeroRef(m_pZeroRefData);
        }
    }
}

bool C_RefObject::incRef()
{
    C_LockTillEnd guard(*this);
    if (!guard || !m_IncRefEnabled)
        return false;

    if (!m_RefCount++)
        onNonzeroRef();

    return true;
}

void C_RefObject::setZeroRefHandler(void (*pOnZeroRef)(void*), void *pData)
{
    C_LockTillEnd guard(*this);
    if (!guard)
        TIMEOUT_ERROR

    m_pOnZeroRef =pOnZeroRef;
    m_pZeroRefData =pData;
    if (!m_RefCount && pOnZeroRef)
        pOnZeroRef(pData);
}

C_LockTillEnd::C_LockTillEnd(I_Lockable &obj):
    m_Obj(obj),
    m_Locked(obj.lock())
{
}

C_LockTillEnd::~C_LockTillEnd()
{
    unlock();
}

void C_LockTillEnd::unlock()
{
    if (m_Locked)
    {
        m_Obj.unlock();
        m_Locked =false;
    }
}

} //namespace bux
