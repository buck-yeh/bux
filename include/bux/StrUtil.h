#ifndef StrUtilH
#define StrUtilH

#include <functional>   // std::function<>
#include <string>       // std::string
#include <string_view>  // std::string_view
#include <istream>      // std::basic_istream<>
#include <ostream>      // std::basic_ostream<>

namespace bux {

//
//      Types
//
struct FC_ParseNone
{
    // Types
    typedef std::function<void(std::string_view)> FH_ApplyData;

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
    typedef std::function<void(std::string_view)> FH_ApplyLine;

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
    typedef std::function<void(std::string_view)> FH_ApplyLine;

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
const char *ord_suffix(size_t i);
std::string expand_env(const char *s);

} // namespace bux

#endif  // StrUtilH
