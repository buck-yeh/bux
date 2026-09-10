/*
    Test cases are organized according to ZOMBIES rules
    http://blog.wingman-sw.com/tdd-guided-by-zombies
*/
#include <bux/StrUtil.h>    // bux::split()
#include <catch2/catch_test_macros.hpp>
#include <set>              // std::set<>
#include <vector>           // std::vector<>

using namespace std::literals;

TEST_CASE("split<char>(\"\", \"\", ...)", "[Z]")
{
    std::vector<std::string> tokens, delims;
    bux::split("", "",
        [&](auto token){ tokens.emplace_back(token); },
        [&](auto delim){ delims.emplace_back(delim); });
    CHECK(tokens.empty());
    CHECK(delims.empty());
    //-------------------------------------------------------
    bux::split("", "",
        [&](auto token){ tokens.emplace_back(token); });
    CHECK(tokens.empty());
}

TEST_CASE("split<wchar_t>(L\"\", L\"\", ...)", "[Z]")
{
    std::vector<std::wstring> tokens, delims;
    bux::split(L"", L"",
        [&](auto token){ tokens.emplace_back(token); },
        [&](auto delim){ delims.emplace_back(delim); });
    CHECK(tokens.empty());
    CHECK(delims.empty());
    //-------------------------------------------------------
    bux::split(L"", L"",
        [&](auto token){ tokens.emplace_back(token); });
    CHECK(tokens.empty());
}

TEST_CASE("split<char32_t>(U\"\", U\"\", ...)", "[Z]")
{
    std::vector<std::u32string> tokens, delims;
    bux::split(U"", U"",
        [&](auto token){ tokens.emplace_back(token); },
        [&](auto delim){ delims.emplace_back(delim); });
    CHECK(tokens.empty());
    CHECK(delims.empty());
    //-------------------------------------------------------
    bux::split(U"", U"",
        [&](auto token){ tokens.emplace_back(token); });
    CHECK(tokens.empty());
}

TEST_CASE("split<wchar_t>() for Tailos", "[I]")
{
    std::vector<std::wstring> tokens;
    std::set<std::wstring> token_set, delim_set;
    bux::split(L"Tâi-uân Tâi-gí", L"- ",
        [&](auto token){
            tokens.emplace_back(token);
            token_set.emplace(token);
            return true;
        },
        [&](auto delim){
            delim_set.emplace(delim);
            return true;
        });
    CHECK(tokens.size() == 4);
    CHECK(tokens == decltype(tokens){L"Tâi", L"uân", L"Tâi", L"gí"});
    CHECK(token_set.size() == 3);
    CHECK(delim_set.size() == 2);
}

TEST_CASE("split<char>(ascii)", "[I]")
{
    std::vector<std::string> tokens;
    bux::split("Hello world!"s, " "s,
        [&](auto token){
            tokens.emplace_back(token);
            return true;
        });
    CHECK(tokens.size() == 2);
    CHECK(tokens == decltype(tokens){"Hello", "world!"});
}

TEST_CASE("split<char32_t>(ascii)", "[I]")
{
    std::vector<std::u32string> tokens;
    bux::split(U"Hello world!"s, U" "s,
        [&](auto token){
            tokens.emplace_back(token);
            return true;
        });
    CHECK(tokens.size() == 2);
    CHECK(tokens == decltype(tokens){U"Hello", U"world!"});
}

TEST_CASE("Verify the final delimeter", "[S]")
{
    std::vector<std::string> strs;
    bux::split("Hello (world)!"s, " ()!"s,
        [&](auto token){
            strs.emplace_back(token);
            return true;
        },
        [&](auto delim){
            strs.emplace_back(delim);
            return true;
        });
    CHECK(strs == std::vector{"Hello"s, " ("s, "world"s, ")!"s});
}
