#ifndef bux_XPlatform_H_
#define bux_XPlatform_H_

#if defined(_MSC_VER)
#   if _MSC_VER >= 1300   // VC++ 7.0 or newer
#       define CUR_FUNC_ __FUNCTION__
#   else
#       define CUR_FUNC_ "?func?"
#   endif
#elif defined(__BORLANDC__)
#   define CUR_FUNC_ __FUNC__
#elif defined(__GNUC__)
#   define CUR_FUNC_ __PRETTY_FUNCTION__
#else
#   define CUR_FUNC_ "?func?"
#endif

#endif // bux_XPlatform_H_
