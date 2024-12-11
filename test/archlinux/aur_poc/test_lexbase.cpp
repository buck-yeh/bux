/*
    Test cases are organized according to ZOMBIES rules
    http://blog.wingman-sw.com/tdd-guided-by-zombies
*/
#include <bux/LexBase.h>    // bux::asciiLiteral()
//#include <bux/UnicodeCvt.h> // bux::BOM()
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Regression errors", "[S]")
{
    CHECK(bux::asciiLiteral("::") == "::");
    CHECK(bux::asciiLiteral("::=") == "::=");
}

TEST_CASE("Expectations", "[S]")
{
    CHECK(bux::asciiLiteral("\n") == "\\n");
    CHECK(bux::asciiLiteral("\r") == "\\r");
    CHECK(bux::asciiLiteral("\t") == "\\t");
    //
    CHECK(bux::asciiLiteral((const char*)u8"\u1234") == (const char*)u8"\u1234");
    CHECK(bux::asciiLiteral((const char*)u8"\uABCD") == (const char*)u8"\uABCD");
}
