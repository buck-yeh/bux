#pragma once

#include <ostream>      // std::basic_ostream<>
#include <span>         // std::span<>

namespace bux {

//
//      Types
//
template <class _CharT, class _Traits>
struct C_OMemBufAsMemberT
{
    // Data
    struct C_OMemBuf: std::basic_streambuf<_CharT,_Traits>
    {
        C_OMemBuf(_CharT *buffer, size_t size)
        { this->setp(buffer, buffer+size); }
        C_OMemBuf(std::span<_CharT> buffer)
        { this->setp(buffer.begin(), buffer.end()); }
    }   m_Buffer;

    // Ctor
    C_OMemBufAsMemberT(_CharT *buffer, size_t size): m_Buffer(buffer, size) {}
    C_OMemBufAsMemberT(std::span<_CharT> buffer): m_Buffer(buffer) {}
};

template <class _CharT, class _Traits =std::char_traits<_CharT>>
class C_OMemStreamT:
    private C_OMemBufAsMemberT<_CharT,_Traits>,
    public std::basic_ostream<_CharT,_Traits> // Inheritance order matters
{
public:

    // Ctor
    C_OMemStreamT(_CharT *buffer, size_t size):
        C_OMemBufAsMemberT<_CharT,_Traits>(buffer, size),
        std::basic_ostream<_CharT,_Traits>(&this->m_Buffer)
        {}
    C_OMemStreamT(std::span<_CharT> buffer):
        C_OMemBufAsMemberT<_CharT,_Traits>(buffer),
        std::basic_ostream<_CharT,_Traits>(&this->m_Buffer)
        {}
};
using C_OMemStream = C_OMemStreamT<char>;

} // namespace bux
