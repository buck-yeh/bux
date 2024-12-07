/*
    Test cases are organized according to ZOMBIES rules
    http://blog.wingman-sw.com/tdd-guided-by-zombies
*/
#include <bux/EZScape.h>    // bux::easy_escape(), bux::easy_unescape()
#include <curl/curl.h>      // curl_easy_escape(), curl_easy_unescape()
#include <catch2/catch_test_macros.hpp>

namespace {

std::string cmp_escape(std::string_view src)
{
    std::string ret;
    if (auto curl = curl_easy_init())
    {
        if (auto output = curl_easy_escape(curl, src.data(), int(src.size())))
        {
            ret = output;
            curl_free(output);
        }
        curl_easy_cleanup(curl);
    }
    return ret;
}

#if 0
std::string cmp_unescape(std::string_view src)
{
    std::string ret;
    if (auto curl = curl_easy_init())
    {
        int decodelen;
        if (auto p = curl_easy_unescape(curl, src.data(), int(src.size()), &decodelen))
        {
            ret.assign(p, decodelen);
            curl_free(p);
        }
        curl_easy_cleanup(curl);
    }
    return ret;
}
#endif

void escape_roundtrip(std::string_view s)
{
    auto t = bux::easy_escape(s);
    CHECK(t == cmp_escape(s));
    CHECK(bux::easy_unescape(t) == s);
}

} // namespace

using namespace std::literals;

TEST_CASE("Zero char", "[Z]")
{
    escape_roundtrip({});
}

TEST_CASE("One char ok", "[O][B]")
{
    for (auto i: {"1"sv, "a"sv, "Z"sv, "%"sv, "_"sv, "\n"sv, "\r"sv, "\t"sv, "\xff"sv, {"\0",1}})
        escape_roundtrip(i);

    CHECK(bux::easy_unescape("%1b") == "\x1b"s);
    CHECK(bux::easy_unescape("%1B") == "\x1b"s);
}

TEST_CASE("One char exception", "[O][E]")
{
    CHECK_THROWS_AS(bux::easy_unescape("%"), std::invalid_argument);
    CHECK_THROWS_AS(bux::easy_unescape("%1"), std::invalid_argument);
}

TEST_CASE("Sentence & paragraphs", "[M][S]")
{
    escape_roundtrip("Hello world!!!"sv);
    escape_roundtrip("廢宅世界歡迎你 🙏"sv);
}
