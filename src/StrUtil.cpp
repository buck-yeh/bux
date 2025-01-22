#ifdef _WIN32
    #include <windows.h>    // to evade "No Target Architecture" error
#endif
#include "StrUtil.h"
//------------------------------------------------------------------------
#include <cstring>          // strstr(), strlen()
#ifdef _WIN32
    #include <processenv.h> // ExpandEnvironmentStringsA()
#elif defined(__unix__) || defined(__unix) || defined(__gnu_linux__)
    #include <wordexp.h>    // wordexp(), wordfree()
#else
    #define DISABLE_EXPAND_ENV_
#endif
#ifdef __unix__
    #include <cxxabi.h>         // abi::__cxa_demangle()
#endif


namespace bux {

//
//      Functions
//
const char *ord_suffix(size_t i)
/*! \param [in] i Ordinal number
    \retval "st" if \em i == 1
    \retval "nd" if \em i == 2
    \retval "rd" if \em i == 3
    \retval "th" else
*/
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

#ifndef DISABLE_EXPAND_ENV_
std::string expand_env(const char *s)
/*! \param [in] s Input string containing environment variables.
    \return Expanded string
*/
{
#ifdef _WIN32
    char buf[2048];
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
    throw std::runtime_error(std::string("Fail to expand \"")+s+'"');
}
#endif //DISABLE_EXPAND_ENV_

std::string _HRTN(const char *originalName)
/*! \param [in] originalName Compiler mangled or expanded type name, depending on which compiler you are using.
    \return Human readable type name
*/
{
    std::string ret;
#ifdef __unix__
    int status;
    char *const name = abi::__cxa_demangle(originalName, NULL, NULL, &status);
    if (!status)
    {
        ret = name;
        free(name);
    }
    else
    {
        ret.assign(originalName).append(" with demangling error ") += std::to_string(status);
    }
#else
    struct C_KeyMap // POD
    {
        const char      *m_Key;
        const char      *m_Value;
    };
    static const C_KeyMap MAP[] ={
        {typeid(std::string).name(), "std::string"}
    };

    for (auto i: MAP)
    {
        std::string t;
        const char *cur = originalName;
        const char *const key = i.m_Key;
        for (const char *pos; *cur && (pos = strstr(cur,key)) != 0;)
        {
            if (cur < pos)
                t.append(cur, pos);

            t.append(i.m_Value);
            cur = pos + strlen(key);
            while (*cur == ' ') ++cur;
        }

        if (!t.empty())
        {
            if (*cur)
                t.append(cur);

            ret = t;
            originalName = ret.c_str();
        }
    }
    if (ret.empty())
        ret = originalName;
#endif
    return ret;
}

std::string OXCPT(const std::exception &e)
/*! \param [in] e Instance of std::exception descent
    \return Printable form with reason
*/
{
    return HRTN(e) + ": " + e.what();
}

} // namespace bux
