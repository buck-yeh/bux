#include "EZScape.h"
#include "XException.h" // THROW_AS()
#include <cctype>       // std::isalnum()
#include <charconv>     // std::from_chars()
#include <cstring>      // std::strchr()
#include <format>       // std::format()

namespace bux {

//
//      Externals
//
std::string easy_escape(std::string_view src)
/*! Replacement of `curl_easy_escape()` in `curl` library
*/
{
    std::string ret;
    for (char c: src)
    {
        if (c && (std::isalnum(c) || std::strchr("-_.~", c)))
            ret += c;
        else
            ret += std::format("%{:02X}", (int)(unsigned char)c);
    }
    return ret;
}

std::string easy_unescape(std::string_view src)
/*! Replacement of `curl_easy_unescape()` in `curl` library
*/
{
    std::string ret;
    for (size_t i = 0, n = src.size(); i < n;)
    {
        const char ch = src[i++];
        if (ch == '%')
        {
            // Convert the two hexadecimal digits after '%'
            unsigned t;
            const auto p = src.data() + i;
            if (i + 2 <= n && std::errc{} == std::from_chars(p, p+2, t, 16).ec)
                ret += char(t);
            else
                THROW_AS(std::invalid_argument, "Invalid percent-encoded string.");

            i += 2;
        }
        else
            ret += ch;
    }
    return ret;
}

} //namespace bux
