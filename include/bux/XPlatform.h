#pragma once

#if defined(_MSC_VER)
#   define CUR_FUNC_ __FUNCTION__
//#   define CUR_FUNC_ __FUNCSIG__
#elif defined(__BORLANDC__)
#   define CUR_FUNC_ __FUNC__
#elif defined(__GNUC__)
#   define CUR_FUNC_ __PRETTY_FUNCTION__
#else
#   define CUR_FUNC_ "?func?"
#endif
