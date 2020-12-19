#include "ScannerBase.h"
#include <charconv>         // std::from_chars()

namespace bux {

std::string escseq2str(std::string s)
{
    for (size_t pos =0; pos < s.size();)
    {
        uint32_t c;
        const size_t n = parseEscapeChar(s, c, pos);
        if (n > 1)
        {
            const auto ucstr = to_utf8(c);
            s.replace(pos, n, ucstr);
            pos += ucstr.size();
        }
        else
            ++pos;
    }
    return s;
}

bool isIdentifier(std::string_view s) noexcept
{
    const size_t end = skipIdentifier(s, 0);
    return end && end == s.size();
}

size_t parseEscapeChar(std::string_view s, uint32_t &c, size_t pos)
{
    const size_t n = s.size();
    if (pos >= n)
        RUNTIME_ERROR("Pos {} passes end of string", pos);

    auto i = pos;
    c = static_cast<unsigned char>(s[i++]);
    if (c == '\\' && i < n)
    {
        const unsigned char cc = static_cast<unsigned char>(s[i++]);
        int radix;
        switch (cc)
        {
        case 'a':
            c = '\a';
            break;
        case 'b':
            c = '\b';
            break;
        case 'f':
            c = '\f';
            break;
        case 'n':
            c = '\n';
            break;
        case 'r':
            c = '\r';
            break;
        case 't':
            c = '\t';
            break;
        case 'v':
            c = '\v';
            break;
        case 'x':
        case 'u':
        case 'U':
            radix = 16;
            goto DecodeDigits;
        default:
            if (cc < '0' || '7' < cc)
                goto WastedBackslash;
            radix = 8;
            --i;
        DecodeDigits:
            if (const auto start = s.data() + i; auto off = std::from_chars(start, s.data()+s.size(), c, radix).ptr - start)
            {
                i += off;
                break;
            }
        WastedBackslash:
            c = cc;
        }
    }
    return i - pos;
}

size_t skipIdentifier(std::string_view s, size_t pos) noexcept
{
    for (bool first = true; pos < s.size(); ++pos, first = false)
    {
        auto c = s[pos];
        if (!isascii(c) || c != '_' && !(first? isalpha(c): isalnum(c)))
            break;
    }
    return pos;
}

} //namespace bux
