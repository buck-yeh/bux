#ifndef bux_UnicodeCvt_H_
#define bux_UnicodeCvt_H_

#include "XQue.h"       // bux::C_Queue<>
#include <cstdint>      // std::uint8_t, std::uint16_t, std::uint32_t
#include <functional>   // std::function<>
#include <iosfwd>       // Forwarded std::istream
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

typedef std::function<bool(char&)> FH_ReadChar;

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
    C_UnicodeIn(std::istream &in, T_Encoding codepage =0);
    ~C_UnicodeIn() noexcept;

    // Nonvirtuals
    int get(T_Utf32 &c);
    int get(T_Utf16 *dst);
    int get(T_Utf8 *dst);
    int lastError() const noexcept { return m_GetQ.empty()? m_ErrCode: 1; }

private:

    // Types
    class C_Source
    {
    public:

        // Nonvirtuals
        C_Source(FH_ReadChar &&readc) noexcept;
        const char *buffer() const noexcept;
        T_Utf16 getUtf16(size_t pos, bool reverseWord =false) const;
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
    void ingestMBCS();
    void init();
    void readCodePage();
    void readASCII();
    void readReverseUTF16();
    bool readUTF16(C_Source &src, bool reverseWord);
    void readUTF16();
    void readUTF8();
    void setCodePage(T_Encoding cp);
    bool testCodePage(T_Encoding cp);
#ifdef __unix__
    void reset_iconv();
#endif
};

class C_MBCStr
{
public:

    // Types
    typedef void (*F_PushCh)(std::string &dst, char c);

    // Nonvirtuals
    C_MBCStr(T_Encoding codepage = 0) noexcept: m_codepage(codepage) {}
    C_MBCStr(const C_MBCStr&) = delete;
    C_MBCStr &operator=(const C_MBCStr&) = delete;

    C_MBCStr(C_MBCStr &&other) noexcept;
    void operator=(C_MBCStr &&other) noexcept;

    C_MBCStr(std::string_view s, T_Encoding codepage = 0) noexcept: m_str(s), m_codepage(codepage) {}
    void operator +=(std::string_view s);

    template<typename T> C_MBCStr(const T *ps, size_t size = 0, T_Encoding codepage = 0): m_codepage(codepage)
    { append(ps, size); }
    template<typename T> void operator +=(const T *ps)
    { append(ps, 0); }

    template<typename T> C_MBCStr(const std::basic_string<T> &s, T_Encoding codepage = 0): m_codepage(codepage)
    { append(s.data(), s.size()); }
    template<typename T> void operator +=(const std::basic_string<T> &s)
    { append(s.data(), s.size()); }

    void append(const char *src, size_t srcBytes);
    template<typename T>
    void append(const T *ps, size_t size)
    {
        if (!size)
            size = std::char_traits<T>::length(ps);

        append(reinterpret_cast<const char*>(ps), size*sizeof(T));
    }

    bool empty() const noexcept;
    const std::string &escape(F_PushCh pushCh) const;
    const std::string &escJSON() const;
    const std::string &strU8() const;

private:

    // Data
    std::vector<T_Utf32>    mutable m_u32s;
    std::string             mutable m_str;
    F_PushCh                mutable m_pushCh{};
    T_Encoding              m_codepage{};

    // Nonvirtuals
    void appendNonRaw(const char *src, size_t srcBytes) const;
    void appendStr(T_Utf32 u32) const;
};

//
//      Function Prototypes
//
//int u32toutf8(T_Utf32 c, T_Utf8 *dst) noexcept;

std::string_view    to_utf8(T_Utf32 c);
std::string         to_utf8(std::string_view s, T_Encoding codepage = 0);
std::string         to_utf8(std::istream &s, T_Encoding codepage = 0);
template<typename T>
auto                to_utf8(const T *ps, size_t size = 0, T_Encoding codepage = 0)  { return C_MBCStr{ps, size, codepage}.strU8(); }
template<typename T>
auto                to_utf8(const std::basic_string_view<T> &s, T_Encoding codepage = 0) { return C_MBCStr{s, codepage}.strU8(); }

std::wstring BOM(const std::wstring_view &ws);

} // namespace bux

using bux::T_Utf32;
using bux::T_Utf16;
using bux::T_Utf8;

#endif // bux_UnicodeCvt_H_
