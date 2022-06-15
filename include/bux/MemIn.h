#pragma once

#include <istream>      // std::basic_istream<>
#include <string_view>  // std::basic_string_view<>

namespace bux {

//
//      Types
//
template <class _CharT, class _Traits>
struct C_IMemBufAsMember
{
    // Data
    struct C_IMemBuf: std::basic_streambuf<_CharT,_Traits>
    {
        C_IMemBuf(const _CharT *buffer, size_t size)
        {
            const auto beg = const_cast<_CharT*>(buffer);
            this->setg(beg, beg, beg+size);
        }
        C_IMemBuf(std::basic_string_view<_CharT,_Traits> buffer)
        {
            const auto beg = const_cast<_CharT*>(buffer.data());
            this->setg(beg, beg, beg+buffer.size());
        }
    } m_Buffer;

    // Ctor
    C_IMemBufAsMember(const _CharT *buffer, size_t size): m_Buffer(buffer, size) {}
    C_IMemBufAsMember(std::basic_string_view<_CharT,_Traits> buffer): m_Buffer(buffer) {}
    C_IMemBufAsMember(std::basic_string<_CharT,_Traits> &&) = delete;
};

template <class _CharT, class _Traits = std::char_traits<_CharT>>
class C_IMemStreamT:
    private C_IMemBufAsMember<_CharT,_Traits>,
    public std::basic_istream<_CharT,_Traits> // Inheritance order matters
{
public:

    // Ctors
    C_IMemStreamT(const _CharT *buffer, size_t size):
        C_IMemBufAsMember<_CharT,_Traits>(buffer, size),
        std::basic_istream<_CharT,_Traits>(&this->m_Buffer)
        {}
    C_IMemStreamT(std::basic_string_view<_CharT,_Traits> buffer):
        C_IMemBufAsMember<_CharT,_Traits>(buffer),
        std::basic_istream<_CharT,_Traits>(&this->m_Buffer)
        {}
    C_IMemStreamT(const _CharT *str):
        C_IMemStreamT(std::basic_string_view<_CharT,_Traits>{str})
        {}
    C_IMemStreamT(std::basic_string<_CharT,_Traits> &&) = delete;
};
using C_IMemStream = C_IMemStreamT<char>;

} // namespace bux
