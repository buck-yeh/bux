#pragma once

#include "XQue.h"       // bux::C_Queue<>
#include <cstdint>      // std::uint8_t, std::uint16_t, std::uint32_t
#include <functional>   // std::function<>
#include <iosfwd>       // Forwarded std::istream
#include <optional>     // std::optional<>
#include <string>       // std::string
#include <string_view>  // std::string_view
#include <vector>       // std::vector<>

#ifdef __unix__
#include <iconv.h>      // iconv_t
#endif

namespace bux {

//
//      Constants
//
enum
{
    MAX_UTF16                   = 2,
    MAX_UTF8                    = 6,    // UTF-8 encoding limit (31 bits UCS-4)
    MAX_UTF8_BMP                = 3,    // Unicode Plane 0: Basic Multilingual Plane
    MAX_UTF8_VALID              = 4,    // Unicode Plane 0~16

    // Error for C_UnicodeIn::get()
    UIE_EOF                     = 0,
    UIE_ILLFORMED_UNICODE       = -1,
    UIE_INCOMPLETE_UNICODE      = -2,
    UIE_NO_UNICODE_TRANSLATION  = -3,
    UIE_INTERNAL                = -9
};

//
//      Types
//
typedef std::uint32_t T_Utf32;  ///< UTF-32 to cover the full range of codespace U+0000 ~ U+10FFFF
typedef std::uint16_t T_Utf16;  ///< UTF-16: You need T_Utf16[2] to hold full range of unicode.
typedef std::uint8_t  T_Utf8;   ///< UTF-8: You need T_Utf8[4] to hold full range of unicode.

typedef std::function<std::optional<char>()> FH_ReadChar;

#ifdef _WIN32
typedef unsigned T_Encoding;
#elif defined(__unix__)
typedef const char *const *T_Encoding; // null-terminated const array of const char pointers
#endif

class C_UnicodeIn
{
public:

    // Ctor/Dtor
    C_UnicodeIn(FH_ReadChar &&readc, T_Encoding codepage =0);
    C_UnicodeIn(std::string_view sv, T_Encoding codepage =0);
    C_UnicodeIn(std::string &&s, T_Encoding codepage =0) = delete;
    C_UnicodeIn(const char *s, T_Encoding codepage =0): C_UnicodeIn(std::string_view(s), codepage) {}
    C_UnicodeIn(std::istream &in, T_Encoding codepage =0);
    ~C_UnicodeIn() noexcept;

    // Nonvirtuals
    int get(T_Utf32 &c);
    int get(T_Utf16 *dst);
    int get(T_Utf8 *dst);
    int lastError() const noexcept { return m_GetQ.empty()? m_ErrCode: 1; }
    T_Encoding encoding() const noexcept { return m_CodePage; }

private:

    // Types
    class C_Source
    {
    public:

        // Nonvirtuals
        C_Source(FH_ReadChar &&readc) noexcept;
        const char *buffer() const noexcept;
        T_Utf16 getUtf16(size_t pos, bool reverseWord) const;
        T_Utf32 getUtf32(size_t pos, bool reverseWord) const;
        void pop(size_t bytes);
        void read(size_t bytes);
        void readTillCtrl();
        size_t size() const noexcept;

    private:

        // Data
        FH_ReadChar         m_ReadCh;
        std::string         m_ReadBuf;
        size_t              m_AvailBeg;
    };

    // Data
    C_Source                m_Src;
    C_Queue<T_Utf32>        m_GetQ;
    void                    (C_UnicodeIn::*m_ReadMethod)(){};
    T_Encoding              m_CodePage;
#ifdef __unix__
    iconv_t                 m_iconv{(iconv_t)-1};   // changed according to m_CodePage
#endif
    int                     m_ErrCode{UIE_EOF};     ///< Positive number indicates no error.

    // Nonvirtuals
    bool guessCodePage();
    void ingestMBCS();
    void init();
    void readCodePage();
    void readASCII();
    void readReverseUTF16();
    void readReverseUTF32();
    void readUTF16();
    bool readUTF16(C_Source &src, bool reverseWord);
    void readUTF32();
    bool readUTF32(C_Source &src, bool reverseWord);
    void readUTF8();
    void setCodePage(T_Encoding cp);
#ifdef __unix__
    void reset_iconv();
#endif
};

//
//      Externs
//
extern const T_Encoding ENCODING_UTF8;

std::string_view to_utf8(T_Utf32 c);
std::string to_utf8(C_UnicodeIn &&uin);

template<typename T>
std::string to_utf8(const T *ps, size_t size = 0, T_Encoding codepage = 0)
{
    if (!size)
        size = std::char_traits<T>::length(ps);

    return to_utf8(C_UnicodeIn(std::string_view{reinterpret_cast<const char*>(ps), size*sizeof(T)}, codepage));
}
template<typename T>
std::string to_utf8(const std::basic_string<T> &s, T_Encoding codepage = 0)
{
    return to_utf8(C_UnicodeIn(std::string_view{reinterpret_cast<const char*>(s.data()), s.size()*sizeof(T)}, codepage));
}
template<typename T>
std::string to_utf8(std::basic_string_view<T> s, T_Encoding codepage = 0)
{
    return to_utf8(C_UnicodeIn(std::string_view{reinterpret_cast<const char*>(s.data()), s.size()*sizeof(T)}, codepage));
}

template<typename T>
std::basic_string<T> BOM(std::basic_string_view<T> sv)
{
    if constexpr (sizeof(T) > 1)
        return T(0xFEFF) + std::basic_string<T>(sv); 
    else
        return std::basic_string<T>{(const T*)u8"\uFEFF"}.append(sv);
}
template<typename T>
std::basic_string<T> BOM(const T *p)
{
    if constexpr (sizeof(T) > 1)
        return T(0xFEFF) + std::basic_string<T>(p);
    else
        return std::basic_string<T>{(const T*)u8"\uFEFF"} += p;
}

} // namespace bux

using bux::T_Utf32;
using bux::T_Utf16;
using bux::T_Utf8;
