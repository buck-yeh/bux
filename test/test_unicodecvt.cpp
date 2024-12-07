/*
    Test cases are organized according to ZOMBIES rules
    http://blog.wingman-sw.com/tdd-guided-by-zombies
*/
#include <bux/LexBase.h>    // bux::asciiLiteral()
#include <bux/UnicodeCvt.h> // bux::to_utf8(), bux::BOM()
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Empty string to BOM", "[Z]")
{
    CHECK(bux::to_utf8(bux::BOM(L"")).empty());
    CHECK(bux::to_utf8(bux::BOM(U"")).empty());
    CHECK(bux::to_utf8(bux::BOM(u"")).empty());
    CHECK(bux::BOM(u8"") == u8"\xef\xbb\xbf");
}

TEST_CASE("String to utf-8 vs stringview to utf-8", "[S]")
{
    using namespace std::literals;
    wchar_t wstr[] = L"一律轉成 utf-8";
    CHECK(bux::to_utf8(wstr) == (const char*)u8"一律轉成 utf-8");
    for (auto &ch: wstr)
        ch = std::byteswap(ch);
    CHECK(bux::to_utf8(wstr) == (const char*)u8"一律轉成 utf-8");

    char32_t u32str[] = U"一律轉成 utf-8";
    CHECK(bux::to_utf8(u32str) == (const char*)u8"一律轉成 utf-8");
    for (auto &ch: u32str)
        ch = std::byteswap(ch);
    CHECK(bux::to_utf8(u32str) == (const char*)u8"一律轉成 utf-8");

    static constinit const char *const CHSETS_UTF16LE[] = {"UCS-2LE", "UTF-16LE", "USC2LE", "UTF16LE", 0};
    char16_t u16str[] = u"一律轉成 utf-8";
    CHECK(bux::to_utf8(u16str, 0, CHSETS_UTF16LE) == (const char*)u8"一律轉成 utf-8");

    static constinit const char *const CHSETS_UTF16[] = {"UCS-2", "UTF-16", "USC2", "UTF16", 0};
    CHECK(bux::to_utf8(u16str, 0, CHSETS_UTF16) == (const char*)u8"一律轉成 utf-8");

    CHECK(bux::to_utf8(u16str) == (const char*)u8"一律轉成 utf-8"); // %%%%% fail
    for (auto &ch: u16str)
        ch = std::byteswap(ch);
    CHECK(bux::to_utf8(u16str) == (const char*)u8"一律轉成 utf-8"); // %%%%% fail

    CHECK(bux::to_utf8(u8"一律轉成 utf-8", 0, bux::ENCODING_UTF8) == (const char*)u8"一律轉成 utf-8");
    CHECK(bux::to_utf8(bux::BOM(u8"一律轉成 utf-8")) == (const char*)u8"一律轉成 utf-8");
    CHECK(bux::to_utf8(u8"一律轉成 utf-8") == (const char*)u8"一律轉成 utf-8");
    CHECK(bux::to_utf8(u8"一律轉成 utf-8"s) == (const char*)u8"一律轉成 utf-8");
    CHECK(bux::to_utf8(u8"一律轉成 utf-8"sv) == (const char*)u8"一律轉成 utf-8");
    CHECK(bux::to_utf8("一律轉成 utf-8") == (const char*)u8"一律轉成 utf-8");
    CHECK(bux::to_utf8("一律轉成 utf-8"s) == (const char*)u8"一律轉成 utf-8");
    CHECK(bux::to_utf8("一律轉成 utf-8"sv) == (const char*)u8"一律轉成 utf-8");
}

