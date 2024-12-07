#include "UnicodeCvt.h"
#include "XException.h"     // RUNTIME_ERROR()
#include <bit>              // std::endian::*, std::byteswap()
#include <cstring>          // memcmp()
#include <format>           // std::format()
#include <istream>          // std::istream
#include <memory>           // std::make_unique<>()

#ifdef _WIN32
#pragma comment(lib, "Advapi32.lib")    // IsTextUnicode()
#include <windows.h>                    // Win32 API
#elif defined(__unix__)
#include <errno.h>                      // errno
#endif

namespace {

//
//      In-Module Types
//
class FC_ReadMem
{
public:

    // Nonvirtuals
    FC_ReadMem(std::string_view sv) noexcept: m_Src(sv.data()), m_End(sv.data()+sv.size()) {}
    std::optional<char> operator()() noexcept
    {
        if (m_Src < m_End)
            return *m_Src++;

        return {};
    }

private:

    // Data
    const char *m_Src, *const m_End;
};

//
//      In-Module Constants
//
#ifdef _WIN32
enum
{
    CHSETS_UTF8 = CP_UTF8
};
#elif defined(__unix__)
// shell command `iconv --list` to show available locales "in this host"
constinit const char *const CHSETS_SJIS[] = {"CP932", "EUC-JP", "SHIFT_JIS", "SHIFT-JIS", "SJIS", 0};
constinit const char *const CHSETS_GB[]   = {"CP936", "EUC-CN", "GB18030", "GBK", 0};
constinit const char *const CHSETS_KSC[]  = {"CP949", "EUC-KR", "JOHAB", 0};
constinit const char *const CHSETS_BIG5[] = {"CP950", "EUC-TW", "BIG5-HKSCS", "BIG5HKSCS", "BIG-5", "BIG5", 0};
constinit const char *const CHSETS_UTF8[] = {"UTF-8", "UTF8", 0};
constinit const char *const CHSETS_UTF7[] = {"UTF-7", "UTF7", 0};
constinit const char *const CHSETS_UTF16LE[] = {"UTF-16LE", "UTF16LE", "UCS-2LE", "USC2LE", 0};
constinit const char *const CHSETS_UTF16BE[] = {"UTF-16BE", "UTF16BE", "UCS-2BE", "USC2BE", 0};
constinit const char *const CHSETS_UTF32LE[] = {"UTF-32LE", "UTF32LE", "UCS-4LE", "USC4LE", 0};
constinit const char *const CHSETS_UTF32BE[] = {"UTF-32BE", "UTF32BE", "UCS-4BE", "USC4BE",  0};
#endif

//
//      In-Module Functions
//
int u32toutf8(T_Utf32 c, T_Utf8 *dst) noexcept
{
    int ret;
    if (c < 0x80)
        // 1 byte
    {
        ret =1;
        *dst = T_Utf8(c);
    }
    else if (c < 0x800)
        // 2 bytes
    {
        ret =2;
        goto Encode;
    }
    else if (c < 0x10000)
        // 3 bytes
    {
        ret =3;
        goto Encode;
    }
    else if (c < 0x200000)
        // 4 bytes
    {
        ret =4;
        goto Encode;
    }
    else if (c < 0x4000000)
        // 5 bytes
    {
        ret =5;
        goto Encode;
    }
    else if (c < 0x80000000)
        // 6 bytesguessCodePage()
    {
        ret =6;
        goto Encode;
    }
    else
        ret =-1;

    return ret;
Encode:
    for (int i =ret; --i > 0;)
    {
        dst[i] = T_Utf8((c &0x3F) | 0x80);
        c >>=6;
    }
    dst[0] = T_Utf8(c |((const unsigned char*)"\xC0\xE0\xF0\xF8\xFC")[ret-2]);
    return ret;
}

} // namespace

namespace bux {

//
//      Constants
//
const T_Encoding ENCODING_UTF8 = CHSETS_UTF8;

//
//      Function Defitions
//
std::string_view to_utf8(T_Utf32 uc)
{
    static thread_local T_Utf8 buf[MAX_UTF8];
    const auto bytes = u32toutf8(uc, buf);
    if (bytes <= 0)
        RUNTIME_ERROR("u32toutf8(u+{:x}) returns {}", uc, bytes);

    return {reinterpret_cast<char*>(buf), size_t(bytes)};
}

std::string to_utf8(C_UnicodeIn &&uin)
{
    T_Utf8 u8[MAX_UTF8];
    int n;
    std::string ret;
    while ((n = uin.get(u8)) > 0)
        ret.append(reinterpret_cast<char*>(u8), size_t(n));

    if (n < 0)
        RUNTIME_ERROR("UTF-8 conversion error {}", n);

    return ret;
}

//
//      Class Implementations
//
C_UnicodeIn::C_UnicodeIn(FH_ReadChar &&readc, T_Encoding codepage):
    m_Src(std::move(readc)),
    m_CodePage(codepage)
{
    init();
}

C_UnicodeIn::C_UnicodeIn(std::string_view sv, T_Encoding codepage):
    C_UnicodeIn(FC_ReadMem(sv), codepage)
{
}

C_UnicodeIn::C_UnicodeIn(std::istream &in, T_Encoding codepage):
    m_Src([&]()->std::optional<char> {
        char ch;
        if (static_cast<bool>(in.get(ch)))
            return ch;
        return {};
    }),
    m_CodePage(codepage)
/*! \param in Reference of input stream
    \param codepage Encoding type of input stream. Value 0 to guess the finest.

    \b HEADSUP: When \em codepage is 0 and \em in reference to a stream instance of type `std::ifstream`,
    the instance should be opened in binary mode with `std::ios::binary` for the case that the encoding is actually UTf-16(LE or BE) and
    input stream contains '\x1a' bytes (ascii EOF), e.g. as part of '\uff1a'. Or the input stream might hit EOF earlier than it should.

    \code{.cpp}
    void foo(const std::string &path)
    {
        std::ifstream fin{path, std::ios::binary};
        bux::C_UnicodeIn uin(fin);
        //... Rest of your code
    }
    \endcode
 */
{
    init();
}

C_UnicodeIn::~C_UnicodeIn() noexcept
{
#ifdef __unix__
    reset_iconv();
#endif
}

int C_UnicodeIn::get(T_Utf32 &c)
{
    if (m_GetQ.empty())
    {
        if (m_ErrCode < 0)
            // Error code persistsiconv --list
            return m_ErrCode;

        if (m_ReadMethod)
            (this->*m_ReadMethod)();

        if (lastError() <= 0)
            // Error happens
            return m_ErrCode;
    }
    c = m_GetQ.front();
    m_GetQ.pop();
    return 1;
}

int C_UnicodeIn::get(T_Utf16 *dst)
{
    T_Utf32 c;
    int ret = get(c);
    if (ret > 0)
    {
        if (c < 0x10000)
            // Encode into a single word
            *dst =T_Utf16(c);
        else
            // Encode into two words
        {
            ret =2;
            c -=0x10000;
            dst[0] =T_Utf16((c >>10) |0xD800);
            dst[1] =T_Utf16((c &0x3FF) |0xDC00);
        }
    }
    return ret;
}

int C_UnicodeIn::get(T_Utf8 *dst)
{
    T_Utf32 c;
    int ret = get(c);
    if (ret > 0)
        ret = u32toutf8(c, dst);

    return ret;
}

void C_UnicodeIn::ingestMBCS()
/*! \pre m_GetQ.empty() is true
*/
{
    if (auto size = m_Src.size())
    {
#ifdef _WIN32
        const auto utf16 = std::make_unique<wchar_t[]>(size);
        if (int wn = MultiByteToWideChar(m_CodePage, MB_ERR_INVALID_CHARS, m_Src.buffer(), int(size), utf16.get(), int(size)))
        {
            FC_ReadMem      read({reinterpret_cast<char*>(utf16.get()), size_t(wn*2)});
            C_Source        src(std::move(read));
            while (readUTF16(src, false));
            m_Src.pop(size);
        }
        else
            m_ErrCode = UIE_NO_UNICODE_TRANSLATION;
#elif defined(__unix__)
        static constinit const char *const TO_UCS4 = std::endian::native == std::endian::little? "UCS-4LE": "UCS-4BE";
        static_assert(std::endian::native == std::endian::little || std::endian::native == std::endian::big);
        for (T_Encoding i = m_CodePage; *i && m_iconv == (iconv_t)(-1); ++i)
            m_iconv = iconv_open(TO_UCS4, *i);

        if (m_iconv == (iconv_t)(-1))
            // Fail to initialize the corresponding iconv_t descriptor
        {
            m_ErrCode = UIE_NO_UNICODE_TRANSLATION;
            return;
        }
        const auto ucs4 = std::make_unique<T_Utf32[]>(size);
        size_t size_ucs4 = size * 4;
        auto src = const_cast<char*>(m_Src.buffer());
        auto dst = reinterpret_cast<char*>(ucs4.get());
        if (size_t(-1) != iconv(m_iconv, &src, &size, &dst, &size_ucs4))
            // Fully converted
        {
            for (const T_Utf32 *i = ucs4.get(); i < reinterpret_cast<T_Utf32*>(dst); m_GetQ.push(*i++));
            m_Src.pop(m_Src.size());
        }
        else switch (errno)
        {
        case EILSEQ: // invalid multibyte sequence
            m_ErrCode = UIE_NO_UNICODE_TRANSLATION;
            break;
        case EINVAL: // incomplete multibyte sequence
            for (const T_Utf32 *i = ucs4.get(); i < reinterpret_cast<T_Utf32*>(dst); m_GetQ.push(*i++));
            m_Src.pop(m_Src.size()-size);
            break;
        case E2BIG: // output buffer overflow, which is impossible.
        default:
            m_ErrCode = UIE_INTERNAL;
        }
#endif
    }
}

void C_UnicodeIn::init()
{
    // BOM encodings from https://en.wikipedia.org/wiki/Byte_order_mark#Byte-order_marks_by_encoding
    m_Src.read(4);
    switch (m_Src.size())
    {
    case 4:
        switch (m_Src.getUtf32(0, false))
        {
        case 0xFEFF: // UTF-32 with BOM
            m_Src.pop(4);
            m_ReadMethod = &C_UnicodeIn::readUTF32;
            return;
        case 0xFFFE0000: // Reverse UTF-32 with BOM
            m_Src.pop(4);
            m_ReadMethod = &C_UnicodeIn::readReverseUTF32;
            return;
        }
        [[fallthrough]];
    case 3:
    case 2:
        switch (m_Src.getUtf16(0, false))
        {
        case 0xFEFF: // UTF-16 with BOM
            m_Src.pop(2);
            m_ReadMethod = &C_UnicodeIn::readUTF16;
            return;
        case 0xFFFE: // Reverse UTF-16 with BOM
            m_Src.pop(2);
            m_ReadMethod = &C_UnicodeIn::readReverseUTF16;
            return;
        default:
            if (m_Src.size() >= 3 && 0 == memcmp(m_Src.buffer(), u8"\uFEFF", 3))
                // UTF-8 with BOM
            {
                m_Src.pop(3);
                setCodePage(CHSETS_UTF8);
                m_ReadMethod = &C_UnicodeIn::readCodePage;
                return;
            }

            // Infer the encoding from first-1000-bytes chunk (UTF-8, ACP, or something else ?)
            if (!m_CodePage)
            {
                m_Src.read(1000);
#ifdef _WIN32
                const auto size = m_Src.size();
                int mask = IS_TEXT_UNICODE_UNICODE_MASK;
                if (IsTextUnicode(m_Src.buffer(), int(size), &mask) || mask)
                {
                    m_ReadMethod = &C_UnicodeIn::readUTF16;
                    return;
                }
                mask = IS_TEXT_UNICODE_REVERSE_MASK;
                if (IsTextUnicode(m_Src.buffer(), int(size), &mask) || mask)
                {
                    m_ReadMethod = &C_UnicodeIn::readReverseUTF16;
                    return;
                }
#endif
                if (guessCodePage())
                    return;
            }

            // Read heading asciis off then we can distinguish CP_UTF8 from CP_ACP.
            m_ReadMethod = &C_UnicodeIn::readASCII;
        }
        break;
    case 1: // has to be ascii
        if (*m_Src.buffer() &0x80)
            m_ErrCode = UIE_INCOMPLETE_UNICODE;
        else
            m_GetQ.push(T_Utf8(*m_Src.buffer()));
        break;
    case 0: // empty string
        break;
    default:
        m_ErrCode = UIE_INTERNAL;
    }
}

void C_UnicodeIn::readASCII()
{
    m_Src.read(1);
    if (m_Src.size())
    {
        const auto c = T_Utf8(*m_Src.buffer());
#ifdef _WIN32
        if (!(c &0x80))
#elif defined(__unix__)
        if (!(c &0x80) && c)
#else
#   error Niether win32 nor unix
#endif
        {
            // Still an ASCII
            m_GetQ.push(c);
            return m_Src.pop(1);
        }
    }

    // Is the rest CP_UTF8 or CP_ACP ?
    m_Src.readTillCtrl();
    if (m_CodePage)
    {
        ingestMBCS();
        if (m_ErrCode != UIE_NO_UNICODE_TRANSLATION)
            // Should be m_CodePage
        {
            m_ReadMethod = &C_UnicodeIn::readCodePage;
            return;
        }
    }
    guessCodePage();
}

bool C_UnicodeIn::guessCodePage()
{
    static constinit const T_Encoding MBCS_CODEPAGES[] ={
#ifdef _WIN32
        CP_UTF8, CP_ACP,
        932, 936, 949, 950, 951, // from https://en.wikipedia.org/wiki/Windows_code_page#East_Asian_multi-byte_code_pages
        CP_UTF7
#elif defined(__unix__)
        CHSETS_UTF32LE, CHSETS_UTF32BE, CHSETS_UTF8, CHSETS_SJIS, CHSETS_GB, CHSETS_KSC, CHSETS_BIG5, CHSETS_UTF7, CHSETS_UTF16LE, CHSETS_UTF16BE
#endif
    };
    for (auto i: MBCS_CODEPAGES)
    {
        m_ErrCode = UIE_EOF; // reset error code
        setCodePage(i);
        ingestMBCS();
        if (m_ErrCode != UIE_NO_UNICODE_TRANSLATION)
        {
            m_ReadMethod = &C_UnicodeIn::readCodePage;
            return true;
        }
    }
    return false;
}

bool C_UnicodeIn::readUTF16(C_Source &src, bool reverseWord)
{
    bool ret = false;
    src.read(2);
    const size_t read = src.size();
    if (read >= 2)
    {
        const auto uc = src.getUtf16(0,reverseWord);
        if (0xD800 <= uc && uc < 0xDC00)
            // Hi word of 2-word encoding
        {
            src.read(4);
            if (src.size() >= 4)
            {
                const T_Utf16 uc2 = src.getUtf16(1,reverseWord);
                if (0xDC00 <= uc2 && uc2 < 0xE000)
                    // Low word of 2-word encoding
                {
                    src.pop(4);
                    m_GetQ.push(T_Utf32((((uc&0x3FF)<<10)|(uc2&0x3FF))+0x10000));
                    ret =true;
                }
                else
                    // Anything lese is ill-formed
                    m_ErrCode = UIE_ILLFORMED_UNICODE;
            }
            else
                m_ErrCode = UIE_INCOMPLETE_UNICODE;
        }
        else if (0xDC00 <= uc && uc < 0xE000)
            // Low word of 2-word encoding - ill-fomed
            m_ErrCode = UIE_ILLFORMED_UNICODE;
        else
        {
            src.pop(2);
            m_GetQ.push(uc);
            ret = true;
        }
    }
    else if (read > 0)
        m_ErrCode = UIE_INCOMPLETE_UNICODE;

    return ret;
}

void C_UnicodeIn::readUTF16()
{
    readUTF16(m_Src, false);
}

void C_UnicodeIn::readReverseUTF16()
{
    readUTF16(m_Src, true);
}

bool C_UnicodeIn::readUTF32(C_Source &src, bool reverseWord)
{
    bool ret = false;
    src.read(4);
    const size_t read = src.size();
    if (read >= 4)
    {
        m_GetQ.push(src.getUtf32(0,reverseWord));
        src.pop(4);
        ret = true;
    }
    else if (read > 0)
        m_ErrCode = UIE_INCOMPLETE_UNICODE;

    return ret;
}

void C_UnicodeIn::readUTF32()
{
    readUTF32(m_Src, false);
}

void C_UnicodeIn::readReverseUTF32()
{
    readUTF32(m_Src, true);
}

void C_UnicodeIn::readCodePage()
{
    m_Src.readTillCtrl();
    ingestMBCS();
}

void C_UnicodeIn::setCodePage(T_Encoding cp)
{
    m_CodePage = cp;
#ifdef __unix__
    reset_iconv();
#endif
}

#ifdef __unix__
void C_UnicodeIn::reset_iconv()
{
    if (m_iconv != (iconv_t)(-1))
    {
        iconv_close(m_iconv);
        m_iconv = (iconv_t)(-1);
    }
}
#endif

C_UnicodeIn::C_Source::C_Source(FH_ReadChar &&readc) noexcept:
    m_ReadCh(std::move(readc)),
    m_AvailBeg(0)
{
}

const char *C_UnicodeIn::C_Source::buffer() const noexcept
{
    return m_ReadBuf.data() + m_AvailBeg;
}

T_Utf16 C_UnicodeIn::C_Source::getUtf16(size_t pos, bool reverseWord) const
{
    const size_t off = m_AvailBeg + pos * 2;
    if (off + 2 > m_ReadBuf.size())
        RUNTIME_ERROR("End of char {} passes end of buffer", off+2);

    auto ret = *reinterpret_cast<const T_Utf16*>(m_ReadBuf.data() + off);
    return reverseWord? std::byteswap(ret): ret;
}

T_Utf32 C_UnicodeIn::C_Source::getUtf32(size_t pos, bool reverseWord) const
{
    const size_t off = m_AvailBeg + pos * 4;
    if (off + 4 > m_ReadBuf.size())
        RUNTIME_ERROR("End of char {} passes end of buffer", off+4);

    auto ret = *reinterpret_cast<const T_Utf32*>(m_ReadBuf.data() + off);
    return reverseWord? std::byteswap(ret): ret;
}

void C_UnicodeIn::C_Source::pop(size_t bytes)
{
    m_AvailBeg += bytes;
    if (m_AvailBeg > m_ReadBuf.size())
    {
        m_AvailBeg -= bytes; // rollback
        RUNTIME_ERROR("m_AvailBeg overflow");
    }
}

void C_UnicodeIn::C_Source::read(size_t bytes)
{
    if (m_AvailBeg + bytes > m_ReadBuf.size())
    {
        if (m_AvailBeg)
        {
            m_ReadBuf.erase(0, m_AvailBeg);
            m_AvailBeg = 0;
        }
        bytes -= m_ReadBuf.size();
        for (size_t i = 0; i < bytes; ++i)
            if (auto c = m_ReadCh())
                m_ReadBuf += *c;
            else
                break;
    }
}

void C_UnicodeIn::C_Source::readTillCtrl()
{
    if (m_AvailBeg == m_ReadBuf.size())
    {
        m_ReadBuf.clear();
        m_AvailBeg = 0;
    }

    while (auto c = m_ReadCh())
    {
        m_ReadBuf += *c;
        if (0 == (*c &0xE0))
            // Control char
            break;
    }
}

size_t C_UnicodeIn::C_Source::size() const noexcept
{
    return m_ReadBuf.size() - m_AvailBeg;
}

} // namespace bux
