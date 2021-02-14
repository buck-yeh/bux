#include "LexBase.h"
#include "UnicodeCvt.h" // bux::C_UnicodeIn
#include <cctype>       // isprint()
#include <cstring>      // strchr()
#include <charconv>     // std::to_chars()

namespace {

using namespace bux;

//
//      Functions
//
void appendAsciiLiteral(uint32_t utf32, std::string &dst)
{
    if (utf32 < 0x100U)
    {
        if (isprint(int(utf32)))
        {
            if (strchr("\\\'\"", int(utf32)))
                dst += '\\';

            dst += char(utf32);
        }
        else switch (utf32)
        {
        case '\a':
            dst += "\\a";
            break;
        case '\b':
            dst += "\\b";
            break;
        case '\f':
            dst += "\\f";
            break;
        case '\n':
            dst += "\\n";
            break;
        case '\r':
            dst += "\\r";
            break;
        case '\t':
            dst += "\\t";
            break;
        case '\v':
            dst += "\\v";
            break;
        default:
            dst += "\\x";
            addAsHex(dst, utf32);
        }
    }
    else
    {
        char buf[10];
        const auto t = std::to_chars(buf, buf+sizeof(buf), utf32, 16);
        if (t.ec == std::errc())
            // On success
        {
            const auto n = t.ptr - buf;
            dst += n <= 4? "\\u": "\\U";
            if (const auto nonzeros = n % 4)
                dst.append(size_t(4-nonzeros), '0');

            dst.append(buf, t.ptr);
        }
        else
        {
            const auto ecode = make_error_code(t.ec);
            RUNTIME_ERROR("std::to_chars() error: {}:{}", ecode.category().name(), ecode.value());
        }
    }
}

}

namespace bux {

//
//      Functions
//
bool operator==(const C_SourcePos &a, const C_SourcePos &b) noexcept
{
    return  a.m_Col == b.m_Col &&
            a.m_Line == b.m_Line &&
            a.m_Source == b.m_Source;
}

bool operator<(const C_SourcePos &a, const C_SourcePos &b) noexcept
{
    if (a.m_Source == b.m_Source)
        return  a.m_Line < b.m_Line || a.m_Line == b.m_Line && a.m_Col < b.m_Col;

    return false;
}

std::string asciiLiteral(uint32_t utf32)
{
    std::string ret;
    appendAsciiLiteral(utf32, ret);
    return ret;
}

std::string asciiLiteral(std::string_view utf8)
{
    C_UnicodeIn             uin{utf8};
    std::string             ret;
    T_Utf32                 u32;
    while (const auto t = uin.get(u32))
    {
        if (t > 0)
            appendAsciiLiteral(u32, ret);
        else
            // On error
            RUNTIME_ERROR("C_UnicodeIn::get() error {}", t);
    }
    return ret;
}

void addAsHex(std::string &dst, uint32_t ch32)
{
    static constexpr const char HEX[]{"0123456789abcdef"};
    dst += HEX[ch32/16];
    dst += HEX[ch32%16];
}

//
//      Implement Classes
//
C_SourcePos::C_SourcePos(std::string_view sourcePath, unsigned line, unsigned col) noexcept:
    m_Source(sourcePath), m_Line(line), m_Col(col)
{
}

C_IntegerLex::C_IntegerLex(std::string_view numstr, int _radix) noexcept:
    m_numStr(numstr), m_radix(_radix)
{
}

void C_IntegerLex::negate()
{
    m_numStr.insert(std::string::size_type(), 1, '-');
}

void C_IntegerLex::prependPlus()
{
    m_numStr.insert(std::string::size_type(), 1, '+');
}

long long C_IntegerLex::value_() const
{
    size_t end;
    auto ret = std::stoll(m_numStr, &end, m_radix);
    if (end < m_numStr.size())
        RUNTIME_ERROR("Not entirely number {}", m_numStr);

    return ret;
}

} //namespace bux
