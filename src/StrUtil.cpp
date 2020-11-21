#include "StrUtil.h"
#include "XException.h"     // RUNTIME_ERROR()
#include <cstring>          // memchr()
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
    RUNTIME_ERROR("Fail to expand \""<<s<<'"')
}

//
//      Implement Classes
//
void FC_ParseNone::operator()(const char *data, size_t n)
{
    std::string s(data, n);
    m_Apply(s);
}

void FC_BufferedParse::operator()(const char *data, size_t n)
{
    enum
    {
        ON_START,
        ON_APPEND,
        ON_TRUNCATE,
        ON_CLEAR
    } state =ON_START;
    size_t n2;
    try
    {
        if (!m_YetToParse.empty())
        {
            state =ON_APPEND;
            m_YetToParse.append(data, n);
            data =m_YetToParse.data();
            n =m_YetToParse.size();
        }
        n2 =parse(data, n);
        if (n2 < n)
        {
            state =ON_TRUNCATE;
            std::string s(data+n2, n-n2);
            m_YetToParse.swap(s);
        }
        else
        {
            state =ON_CLEAR;
            m_YetToParse.clear();
        }
    }
    catch (const std::bad_alloc &)
    {
        // Out of memory
        const size_t n_parsed =m_YetToParse.size();
        m_YetToParse.clear();
        std::ostringstream out;
        out <<"std::bad_alloc#" <<(int)state;
        switch (state)
        {
        case ON_APPEND:
            out <<"# appending " <<n <<" bytes to " <<n_parsed <<" bytes";
            break;
        case ON_TRUNCATE:
            out <<"# truncating " <<n2 <<" bytes from " <<n_parsed <<" bytes";
            break;
        default:;
        }
        RUNTIME_ERROR(out.str())
    }
}

FC_ParseLine::FC_ParseLine(const FH_ApplyLine &apply, char delimeter):
    m_Apply(apply),
    m_Delim(delimeter)
{
}

size_t FC_ParseLine::parse(const char *data, size_t n)
{
    size_t left =n;
    while (const char *p = static_cast<const char*>(memchr(data, m_Delim, left)))
    {
        std::string s(data, p);
        m_Apply(s);
        const size_t jump =p -data +1;
        left -=jump;
        data +=jump;
    }
    return n -left;
}

FC_ParseCRLF::FC_ParseCRLF(const FH_ApplyLine &apply): m_Apply(apply)
{
}

size_t FC_ParseCRLF::parse(const char *data, size_t n)
{
    size_t left =n;
    const char *p;
    while (left && (p =static_cast<const char*>(memchr(data, '\r', left))) != 0)
    {
        const char *const dend =data +left;
    CheckCRLF:
        if (dend -p < 2)
            break;

        if (p[1] != '\n')
        {
            p =(const char*)memchr(p+1, '\r', left);
            goto CheckCRLF;
        }

        std::string s(data, p);
        m_Apply(s);
        const size_t jump =p -data +2;
        left -=jump;
        data +=jump;
    }
    return n -left;
}

} // namespace bux
