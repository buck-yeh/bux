/*
    Test cases are organized according to ZOMBIES rules
    http://blog.wingman-sw.com/tdd-guided-by-zombies
*/
#include <bux/EZArgs.h>     // bux::C_EZArgs, bux::C_ErrorOrIndex

#define CATCH_CONFIG_MAIN   // This tells Catch to provide a main() - only do this in one cpp file
#include <catch2/catch_test_macros.hpp>

#include <charconv>         // std::from_chars()
#include <filesystem>       // std::filesystem::*
#include <ranges>           // std::ranges::views::empty<>

namespace {

//
//      In-Module Constants
//
constexpr const char *const MYARGS[] = {"FOO", "X", "Y", "Z", "AAA"};

} // namespace

TEST_CASE("Null parse", "[Z]")
{
    static constinit const char *const ARGV[]{"foo"};
    REQUIRE(bux::C_EZArgs{}.parse(std::size(ARGV), ARGV));
}

TEST_CASE("Null help", "[Z]")
{
    static constinit const char *const ARGV[]{"foo", "-h"};
    auto ret = bux::C_EZArgs{}.parse(std::size(ARGV), ARGV);
    REQUIRE(!ret);
    CHECK(ret.message() ==
    "USAGE: foo [-h]\n"
    "\n"
    "VALID FLAGS:\n"
    "  -h, --help\n"
	"\tDisplay this help and exit\n");
}

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

TEST_CASE("Scenario: argv[0] with -E -h", "[S]")
{
    bux::C_EZArgs   ezargs;
    ezargs.add_subcommand("foo", []{})
          .position_args(std::array{"eeny"});
    ezargs.add_subcommand("bar", []{})
          .position_args(std::array{"meeny"});
    ezargs.add_flag("eureka", 'E', "123456abcdef", []{});
    const std::string arg0 = std::filesystem::current_path() / "test1.exe";
    const char *const argv[]{arg0.c_str(), "-h"};
    auto ret = ezargs.parse(2, argv);
    REQUIRE(!ret);
    CHECK(ret.message() ==
        "USAGE: test1.exe (foo|bar) ... [-E] [-h]\n"
        "VALID ACTIONS:\n"
        "  foo\n"
        "  bar\n");
}

TEST_CASE("Scenario: Subcommand help", "[S]") // for commit 42ee62ad4e8c3139b9785eee007dcd71030f0b3b
{
    bux::C_EZArgs   ezargs;
    ezargs.add_subcommand("foo", []{})
          .position_args(std::array{"eeny"});
    ezargs.add_subcommand("bar", []{})
          .position_args(std::array{"meeny"});
    const std::string arg0 = std::filesystem::current_path() / "test1.exe";
    const char *const argv[]{arg0.c_str(), "foo", "-h"};
    auto ret = ezargs.parse(3, argv);
    REQUIRE(!ret);
    std::istringstream in{ret.message()};
    std::string line;
    REQUIRE(std::getline(in, line));
    CHECK(line == "USAGE: test1.exe foo <eeny> [-h]");
}
TEST_CASE("Scenario: Parse negative number as flag value", "[S]")
{
    double          x{};
    bux::C_EZArgs   ezargs;
    ezargs.add_flag('x', "foobar", [&](auto v){ // parse
        std::from_chars(v.data(), v.data()+v.size(), x);
    });
    const std::string arg0 = std::filesystem::current_path() / "test1.exe";
    const char *argv[]{arg0.c_str(), "-x", "-.5"};
    REQUIRE(ezargs.parse(3, argv));
    CHECK(x == -.5);
    //-------------------------------------------------------------------------
    ezargs.add_flag('.', "contrived", []{}); // trigger
    x = 0;
    REQUIRE(ezargs.parse(3, argv));
    CHECK(x == -.5);
    //-------------------------------------------------------------------------
    ezargs.add_flag('5', "contrived", []{}); // trigger
    CHECK(!ezargs.parse(3, argv));
    //-------------------------------------------------------------------------
    std::string s6;
    ezargs.add_flag('6', "contrived", [&](auto v){ s6 = v; }); // parse
    argv[2] = "-.6";
    x = 0;
    REQUIRE(ezargs.parse(3, argv));
    CHECK(x == -.6);
}
