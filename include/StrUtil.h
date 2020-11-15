#ifndef StrUtilH
#define StrUtilH

#include <cstdlib>      // strtol(), strtoul(), wcstol(), wcstoul()
#include <functional>   // std::function<>
#include <limits>       // std::numeric_limits<>
#include <sstream>      // std::basic_istringstream<>

namespace bux {

//
//      Types
//
struct FC_ParseNone
{
    // Types
    typedef std::function<void(std::string&)> FH_ApplyData;

    // Data
    const FH_ApplyData  m_Apply;

    // Nonvirtuals
    FC_ParseNone(const FH_ApplyData &apply): m_Apply(apply) {}
    void operator()(const char *data, size_t n);
};

class FC_BufferedParse
{
public:

    // Nonvirtuals
    void operator()(const char *data, size_t n);

private:

    // Data
    std::string             m_YetToParse;

    // Pure virtuals
    virtual size_t parse(const char *data, size_t n) =0;
};

class FC_ParseLine: public FC_BufferedParse
{
public:

    // Types
    typedef std::function<void(std::string&)> FH_ApplyLine;

    // Data
    const FH_ApplyLine      m_Apply;
    const char              m_Delim;

    // Nonvirtuals
    FC_ParseLine(const FH_ApplyLine &apply, char delimeter ='\n');

protected:

    // Implement FC_BufferedParse
    size_t parse(const char *data, size_t n) override;
};

class FC_ParseCRLF: public FC_BufferedParse
{
public:

    // Types
    typedef std::function<void(std::string&)> FH_ApplyLine;

    // Data
    const FH_ApplyLine m_Apply;

    // Nonvirtuals
    FC_ParseCRLF(const FH_ApplyLine &apply);

protected:

    // Implement FC_BufferedParse
    size_t parse(const char *data, size_t n) override;
};

template <class _CharT, class _Traits =std::char_traits<_CharT> >
struct C_OMemBuf: std::basic_streambuf<_CharT,_Traits>
{
    C_OMemBuf(_CharT *buffer, size_t size)
        { setp(buffer, buffer+size); }
};

template <class _CharT, class _Traits =std::char_traits<_CharT> >
struct C_OMemBufAsMember
{
    // Data
    C_OMemBuf<_CharT,_Traits> m_Buffer;

    // Ctor
    C_OMemBufAsMember(_CharT *buffer, size_t size): m_Buffer(buffer, size) {}
};

template <class _CharT, class _Traits =std::char_traits<_CharT> >
class C_OMemStream:
    private C_OMemBufAsMember<_CharT,_Traits>,
    public std::basic_ostream<_CharT,_Traits> // Inheritance order matters
{
public:

    // Ctor
    C_OMemStream(_CharT *buffer, size_t size):
        C_OMemBufAsMember<_CharT,_Traits>(buffer, size),
        std::basic_ostream<_CharT,_Traits>(&this->m_Buffer)
        {}
};

template <class _CharT, class _Traits =std::char_traits<_CharT> >
struct C_IMemBuf: std::basic_streambuf<_CharT,_Traits>
{
    C_IMemBuf(const _CharT *buffer, size_t size)
    {
        this->setg(const_cast<_CharT*>(buffer), const_cast<_CharT*>(buffer),
            const_cast<_CharT*>(buffer)+size);
    }
    C_IMemBuf(const _CharT *buffer)
    {
        this->setg(const_cast<_CharT*>(buffer), const_cast<_CharT*>(buffer),
            const_cast<_CharT*>(buffer)+_Traits::length(buffer));
    }
};

template <class _CharT, class _Traits =std::char_traits<_CharT> >
struct C_IMemBufAsMember
{
    // Data
    C_IMemBuf<_CharT,_Traits> m_Buffer;

    // Ctor
    C_IMemBufAsMember(const _CharT *buffer, size_t size): m_Buffer(buffer, size) {}
    C_IMemBufAsMember(const _CharT *buffer): m_Buffer(&buffer) {}
};

template <class _CharT, class _Traits =std::char_traits<_CharT> >
class C_IMemStream:
    private C_IMemBufAsMember<_CharT,_Traits>,
    public std::basic_istream<_CharT,_Traits> // Inheritance order matters
{
public:

    // Ctors
    C_IMemStream(const _CharT *buffer, size_t size):
        C_IMemBufAsMember<_CharT,_Traits>(buffer, size),
        std::basic_istream<_CharT,_Traits>(&this->m_Buffer)
        {}
    C_IMemStream(const _CharT *buffer):
        C_IMemBufAsMember<_CharT,_Traits>(buffer),
        std::basic_istream<_CharT,_Traits>(&this->m_Buffer)
        {}
};

//
//      Externs
//
const char *ordSuffix(size_t i);

//
//      Function Templates
//
#if defined(_MSC_VER) && _MSC_VER < 1300
template<class T_Ch, class T>
bool strToVal(const std::basic_string<T_Ch> &src, T &dst)
{
    std::basic_istringstream<T_Ch> in(src);
    return (in >> dst) && size_t(in.tellg()) == src.size();
}
#else
template<class T_Ch, class T, bool bIntegral, bool bSigned>
struct C_DoStrToVal
{
    static bool run(const std::string &src, T &dst)
    {
        std::basic_istringstream<T_Ch> in(src);
        return (in >> dst) && size_t(in.tellg()) == src.size();
    }
};

template<class T>
struct C_DoStrToVal<char,T,true,true>
{
    static bool run(const std::string &src, T &dst)
    {
        char *end;
        long t =strtol(src.c_str(), &end, 0);
        if (!*end && src.c_str() < end)
        {
            dst =T(t);
            return true;
        }
        return false;
    }
};

template<class T>
struct C_DoStrToVal<char,T,true,false>
{
    static bool run(const std::string &src, T &dst)
    {
        char *end;
        unsigned long t =strtoul(src.c_str(), &end, 0);
        if (!*end && src.c_str() < end)
        {
            dst =T(t);
            return true;
        }
        return false;
    }
};

template<class T>
struct C_DoStrToVal<wchar_t,T,true,true>
{
    static bool run(const std::wstring &src, T &dst)
    {
        wchar_t *end;
        long t =wcstol(src.c_str(), &end, 0);
        if (!*end && src.c_str() < end)
        {
            dst =T(t);
            return true;
        }
        return false;
    }
};

template<class T>
struct C_DoStrToVal<wchar_t,T,true,false>
{
    static bool run(const std::wstring &src, T &dst)
    {
        wchar_t *end;
        unsigned long t =wcstoul(src.c_str(), &end, 0);
        if (!*end && src.c_str() < end)
        {
            dst =T(t);
            return true;
        }
        return false;
    }
};

template<class T_Ch, class T>
inline bool strToVal(const std::basic_string<T_Ch> &src, T &dst)
{
    return C_DoStrToVal<T_Ch, T,
        std::numeric_limits<T>::is_integer,
        std::numeric_limits<T>::is_signed>::run(src, dst);
}
#endif

} // namespace bux

#endif  // StrUtilH
