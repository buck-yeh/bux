#ifndef bux_MemIn_H_
#define bux_MemIn_H_

#include <istream>      // std::basic_istream<>
#include <string_view>  // std::basic_string_view<>

namespace bux {

//
//      Types
//
template <class _CharT, class _Traits =std::char_traits<_CharT> >
struct C_IMemBuf: std::basic_streambuf<_CharT,_Traits>
{
    C_IMemBuf(const _CharT *buffer, size_t size)
    {
        this->setg(const_cast<_CharT*>(buffer), const_cast<_CharT*>(buffer),
            const_cast<_CharT*>(buffer)+size);
    }
    C_IMemBuf(std::basic_string_view<_CharT,_Traits> buffer)
    {
        this->setg(buffer.begin(), buffer.begin(), buffer.end());
    }
};

template <class _CharT, class _Traits =std::char_traits<_CharT> >
struct C_IMemBufAsMember
{
    // Data
    C_IMemBuf<_CharT,_Traits> m_Buffer;

    // Ctor
    C_IMemBufAsMember(const _CharT *buffer, size_t size): m_Buffer(buffer, size) {}
    C_IMemBufAsMember(std::basic_string_view<_CharT,_Traits> buffer): m_Buffer(buffer) {}
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
    C_IMemStream(std::basic_string_view<_CharT,_Traits> buffer):
        C_IMemBufAsMember<_CharT,_Traits>(buffer),
        std::basic_istream<_CharT,_Traits>(&this->m_Buffer)
        {}
};

} // namespace bux

#endif  // bux_MemIn_H_
