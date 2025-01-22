/*
    Test cases are organized according to ZOMBIES rules
    http://blog.wingman-sw.com/tdd-guided-by-zombies
*/
#include <bux/UnicodeCvt.h>             // bux::to_utf8(), bux::BOM()
#include <catch2/catch_test_macros.hpp>
#include <bit>                          // std::byteswap()

//#define INDEFINITE_UTF16_

TEST_CASE("Empty string to BOM", "[Z]")
{
    CHECK(bux::to_utf8(bux::BOM(L"")).empty());
    CHECK(bux::to_utf8(bux::BOM(U"")).empty());
    CHECK(bux::to_utf8(bux::BOM(u"")).empty());
    CHECK(bux::BOM(u8"") == u8"\uFEFF");
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

    char16_t u16str[] = u"一律轉成 utf-8";
    auto bomed_u16str = bux::BOM(u16str);
#ifndef _WIN32
    static constinit const char *const CHSETS_UTF16LE[] = {"UTF-16LE", "UTF16LE", "UCS-2LE", "USC2LE", 0};
    CHECK(bux::to_utf8(u16str, 0, CHSETS_UTF16LE) == (const char*)u8"一律轉成 utf-8");

#ifdef INDEFINITE_UTF16_
    static constinit const char *const CHSETS_UTF16[] = {"UCS-2", "UTF-16", "USC2", "UTF16", 0};
    CHECK(bux::to_utf8(u16str, 0, CHSETS_UTF16) == (const char*)u8"一律轉成 utf-8");
#endif
    CHECK(bux::to_utf8(bomed_u16str) == (const char*)u8"一律轉成 utf-8");
#endif
#ifdef INDEFINITE_UTF16_
    CHECK(bux::to_utf8(u16str) == (const char*)u8"一律轉成 utf-8");
#endif
    for (auto &ch: u16str)
        ch = std::byteswap(ch);
    for (auto &ch: bomed_u16str)
        ch = std::byteswap(ch);
#ifndef _WIN32
#ifdef INDEFINITE_UTF16_
    static constinit const char *const CHSETS_UTF16BE[] = {"UTF-16BE", "UTF16BE", "UCS-2BE", "USC2BE", 0};
    CHECK(bux::to_utf8(u16str, 0, CHSETS_UTF16BE) == (const char*)u8"一律轉成 utf-8");
#endif
    CHECK(bux::to_utf8(bomed_u16str) == (const char*)u8"一律轉成 utf-8");
#endif
#ifdef INDEFINITE_UTF16_
    CHECK(bux::to_utf8(u16str) == (const char*)u8"一律轉成 utf-8");
#endif
    CHECK(bux::to_utf8(u8"一律轉成 utf-8", 0, bux::ENCODING_UTF8) == (const char*)u8"一律轉成 utf-8");
    CHECK(bux::to_utf8(bux::BOM(u8"一律轉成 utf-8")) == (const char*)u8"一律轉成 utf-8");
    CHECK(bux::to_utf8(u8"一律轉成 utf-8") == (const char*)u8"一律轉成 utf-8");
    CHECK(bux::to_utf8(u8"一律轉成 utf-8"s) == (const char*)u8"一律轉成 utf-8");
    CHECK(bux::to_utf8(u8"一律轉成 utf-8"sv) == (const char*)u8"一律轉成 utf-8");
    CHECK(bux::to_utf8("一律轉成 utf-8") == (const char*)u8"一律轉成 utf-8");
    CHECK(bux::to_utf8("一律轉成 utf-8"s) == (const char*)u8"一律轉成 utf-8");
    CHECK(bux::to_utf8("一律轉成 utf-8"sv) == (const char*)u8"一律轉成 utf-8");
}

