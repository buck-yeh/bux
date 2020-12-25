#ifndef bux_MemOut_H_
#define bux_MemOut_H_

#include <ostream>      // std::basic_ostream<>
#include <span>         // std::span<>

namespace bux {

//
//      Types
//
template <class _CharT, class _Traits =std::char_traits<_CharT> >
struct C_OMemBuf: std::basic_streambuf<_CharT,_Traits>
{
    C_OMemBuf(_CharT *buffer, size_t size)
        { setp(buffer, buffer+size); }
    C_OMemBuf(std::span<_CharT> buffer)
        { setp(buffer.begin(), buffer.end()); }
};

template <class _CharT, class _Traits =std::char_traits<_CharT> >
struct C_OMemBufAsMember
{
    // Data
    C_OMemBuf<_CharT,_Traits> m_Buffer;

    // Ctor
    C_OMemBufAsMember(_CharT *buffer, size_t size): m_Buffer(buffer, size) {}
    C_OMemBufAsMember(std::span<_CharT> buffer): m_Buffer(buffer) {}
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
    C_OMemStream(std::span<_CharT> buffer):
        C_OMemBufAsMember<_CharT,_Traits>(buffer),
        std::basic_ostream<_CharT,_Traits>(&this->m_Buffer)
        {}
};

} // namespace bux

#endif  // bux_MemOut_H_
