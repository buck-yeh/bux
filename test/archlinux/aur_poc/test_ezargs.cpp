/*
    Test cases are organized according to ZOMBIES rules
    http://blog.wingman-sw.com/tdd-guided-by-zombies

    Initially a push then a proof to testability of global logger
*/
#include <bux/EZArgs.h>     // bux::C_EZArgs, bux::C_ErrorOrIndex

#define CATCH_CONFIG_MAIN   // This tells Catch to provide a main() - only do this in one cpp file
#include <catch2/catch.hpp>

namespace {

//
//      In-Module Constants
//
constexpr const char *const MYARGS[] = {"FOO", "X", "Y", "Z", "AAA"};

} // namespace

TEST_CASE("Scenario: position_args() with one argument", "[S][I]")
{
    bux::C_EZArgs   ezargs;
    ezargs.position_args(std::array{"a", "b", "c"});
    REQUIRE(!ezargs.parse(1, MYARGS));
    REQUIRE(!ezargs.parse(2, MYARGS));
    REQUIRE(!ezargs.parse(3, MYARGS));
    REQUIRE(ezargs.parse(4, MYARGS));
    REQUIRE(ezargs.parsed_position_argc() == 4);
    REQUIRE(!ezargs.parse(5, MYARGS));
}

TEST_CASE("Scenario: position_args() with two argument", "[S][I]")
{
    bux::C_EZArgs   ezargs;
    ezargs.position_args(std::array{"a", "b", "c"}, std::array{1,2});
    REQUIRE(!ezargs.parse(1, MYARGS));
    REQUIRE(ezargs.parse(2, MYARGS));
    REQUIRE(ezargs.parsed_position_argc() == 2);
    REQUIRE(ezargs.parse(3, MYARGS));
    REQUIRE(ezargs.parsed_position_argc() == 3);
    REQUIRE(ezargs.parse(4, MYARGS));
    REQUIRE(ezargs.parsed_position_argc() == 4);
    REQUIRE(!ezargs.parse(5, MYARGS));
}

TEST_CASE("Scenario: add_flag() with trigger only", "[S][I]")
{
    bux::C_EZArgs   ezargs;
    ezargs.add_flag("foo", 'f', "123456abcdef", []{});
    REQUIRE(ezargs.parsed_position_argc() == 0);
}
