#include "XException.h"
#ifdef _WIN32
#include "AtomiX.h"     // buc::C_SpinLock
#include <stdlib.h>     // atexit()
#include <windows.h>
#endif

namespace {

#ifdef _WIN32
/*
    Use TLS API instead of __declspec(thread) because the latter doesn't work within dll.
*/
enum
{
    CATCH_SE_CALLED         = 1,
    USE_OLD_USER_SE_FIRST   = 2
};

DWORD TLSInd_FlagsSE = TLS_OUT_OF_INDEXES;
DWORD TLSInd_UserSE  = TLS_OUT_OF_INDEXES;

#if defined(__BORLANDC__)
void _USERENTRY TLSInd_Free()
#elif defined(_MSC_VER)
void __cdecl TLSInd_Free()
#else
void TLSInd_Free()
#endif
{
    TlsFree(TLSInd_FlagsSE);
    TlsFree(TLSInd_UserSE);
    TLSInd_FlagsSE = TLS_OUT_OF_INDEXES;
    TLSInd_UserSE  = TLS_OUT_OF_INDEXES;
}

LONG WINAPI usrSEH(_EXCEPTION_POINTERS *pInfo)
{
    LONG ret = EXCEPTION_CONTINUE_SEARCH;
    if (size_t(TlsGetValue(TLSInd_FlagsSE)) &USE_OLD_USER_SE_FIRST)
    {
        if (auto oldHook = LPTOP_LEVEL_EXCEPTION_FILTER(TlsGetValue(TLSInd_UserSE)))
            ret =oldHook(pInfo);
    }

    if (EXCEPTION_CONTINUE_SEARCH == ret)
    {
        if (const auto er =pInfo->ExceptionRecord)
            RUNTIME_ERROR("code 0x{:x}, flags 0x{:x}, extra {}, ip {}, arg#{:x}",
                er->ExceptionCode,
                er->ExceptionFlags,
                static_cast<void*>(er->ExceptionRecord),
                static_cast<void*>(er->ExceptionAddress),
                er->NumberParameters);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}
#endif

} // namespace

namespace bux {

//
//      Functions
//
#ifdef _WIN32
void catchSE(bool useOldHookFirst)
/*! \param [in] useOldHookFirst Call the previous exception filter first whenever
     SE occurs.

    The first call for a given thread hooks a home-made exception filter, which
    throws a std::exception-derived object instead of crashing the program by SE.
    Subsequent calls will not change anything.
*/
{
    if (TLS_OUT_OF_INDEXES == TLSInd_FlagsSE)
    {
        static std::atomic_flag lock = ATOMIC_FLAG_INIT;
        C_SpinLock       _(lock);
        if (TLS_OUT_OF_INDEXES == TLSInd_FlagsSE)
        {
            TLSInd_UserSE   = TlsAlloc();
            TLSInd_FlagsSE  = TlsAlloc();
            atexit(TLSInd_Free);
        }
    }

    if (!TlsGetValue(TLSInd_FlagsSE))
    {
        TlsSetValue(TLSInd_UserSE, LPVOID(SetUnhandledExceptionFilter(usrSEH)));
        size_t flags = CATCH_SE_CALLED;
        if (useOldHookFirst)
            flags |= USE_OLD_USER_SE_FIRST;

        TlsSetValue(TLSInd_FlagsSE, LPVOID(flags));
    }
}
#endif

} // namespace bux
