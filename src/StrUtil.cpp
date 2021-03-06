#include "StrUtil.h"
#include "XException.h"     // RUNTIME_ERROR()
#ifdef _WIN32
    #include <processenv.h> // ExpandEnvironmentStringsA()
#elif defined(__unix__) || defined(__unix) || defined(__gnu_linux__)
    #include <wordexp.h>    // wordexp(), wordfree()
#else
    #error "Platform other than Windows and Unix-like not supported yet"
#endif

namespace bux {

//
//      Functions
//
const char *ord_suffix(size_t i)
{
    switch (i)
    {
    case 1:
        return "st";
    case 2:
        return "nd";
    case 3:
        return "rd";
    }
    return "th";
}

std::string expand_env(const char *s)
{
#ifdef _WIN32
    const char buf[2048];
    if (const auto n = ExpandEnvironmentStringsA(s, buf, sizeof buf))
        return {buf, n};
#elif defined(__unix__) || defined(__unix) || defined(__gnu_linux__)
    wordexp_t p;
    wordexp(s, &p, 0);
    std::string ret;
    bool done{};
    if (p.we_wordc == 1)
    {
        ret = p.we_wordv[0];
        done = true;
    }
    wordfree(&p);
    if (done)
        return ret;
#endif
    RUNTIME_ERROR("Fail to expand \"{}\"", s);
}

} // namespace bux
