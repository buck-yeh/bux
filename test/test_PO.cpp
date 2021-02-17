/*
    Test cases are organized by ZOMBIES rules
    http://blog.wingman-sw.com/tdd-guided-by-zombies
*/
#include "bux/PartialOrdering.h"
#include <random>           // std::mt19937
#include <string>           // std::string

#define CATCH_CONFIG_MAIN   // This tells Catch to provide a main() - only do this in one cpp file
#include <catch2/catch.hpp>

thread_local std::mt19937 g_rng{std::random_device{"/dev/urandom"}()};

TEST_CASE("Empty relation", "[Z]")
{
    bux::C_PartialOrdering<unsigned char> po;
    REQUIRE(po.empty());

    std::uniform_int_distribution<unsigned char> gen{0,255};
    for (int i = 0; i < 10; ++i)
        REQUIRE(!po.related(gen(g_rng), gen(g_rng)));
}

TEST_CASE("One relation", "[O][I]")
{
    bux::C_PartialOrdering<char> po;
    REQUIRE(po.empty());
    REQUIRE(po.addOrder('a', 'b'));
    REQUIRE(!po.empty());
    //------------------------------
    REQUIRE(po.related('a', 'b'));
    REQUIRE(!po.related('b', 'a'));
    REQUIRE(!po.related('a', 'a'));
    REQUIRE(!po.related('b', 'b'));
}

TEST_CASE("Multiple relations", "[M][I][B]")
{
    bux::C_PartialOrdering<char> po;
    REQUIRE(po.addOrder('a', 'b'));
    REQUIRE(po.addOrder('b', 'c'));
    REQUIRE(po.addOrder('c', 'd'));
    REQUIRE(po.addOrder('c', 'e'));
    // Anti-looping ----------------
    REQUIRE(!po.addOrder('b', 'a'));
    REQUIRE(!po.addOrder('c', 'a'));
    REQUIRE(!po.addOrder('e', 'a'));
    // Related ---------------------
    REQUIRE(po.related('a', 'd'));
    REQUIRE(po.related('a', 'e'));
    REQUIRE(!po.related('d', 'e'));
    REQUIRE(!po.anyLeft('a'));
    REQUIRE(!po.anyRight('d'));
    REQUIRE(!po.anyRight('e'));
    size_t count = 0;
    po.getRelated('a', [&](auto){ ++count; });
    REQUIRE(count == 4);
    count = 0;
    po.getRelated([&](auto){ ++count; }, 'e');
    REQUIRE(count == 3);
    // Depth -----------------------
    REQUIRE(po.depthToLeft('d') == 3);
    REQUIRE(po.depthToLeft('e') == 3);
    // Irreflexibility -------------
    for (char i = 'a'; i <= 'e'; ++i)
        REQUIRE(!po.related(i, i));
}

TEST_CASE("Boundary checking", "[B]")
{
    bux::C_PartialOrdering<char> po;
    REQUIRE(po.addOrder('a', 'b'));
    REQUIRE(po.addOrder('a', 'c'));
    REQUIRE(!po.anyLeft('d'));
    REQUIRE(!po.anyRight('d'));
    size_t count = 0;
    po.getRelated('d', [&](auto){ ++count; });
    REQUIRE(count == 0);
    count = 0;
    po.getRelated([&](auto){ ++count; }, 'e');
    REQUIRE(count == 0);
}

TEST_CASE("Interface", "[I]")
{
    bux::C_PartialOrdering<char> po;
    REQUIRE(po.addOrder('a', 'b'));
    REQUIRE(po.addOrder('a', 'c'));
    REQUIRE(!po.empty());
    po.clear();
    REQUIRE(po.empty());
}

/* No case
TEST_CASE("Exceptions", "[E]")
{
}
*/

TEST_CASE("Scenario: Calling makeLinear() method", "[S][I]")
{
    bux::C_PartialOrdering<char> po;
    REQUIRE(po.addOrder('a', 'b'));
    REQUIRE(po.addOrder('a', 'c'));
    REQUIRE(po.addOrder('b', 'd'));
    REQUIRE(po.addOrder('b', 'e'));
    REQUIRE(po.addOrder('c', 'f'));
    REQUIRE(po.addOrder('c', 'g'));
    //-----------------------------
    std::string out_breadth1st;
    po.makeLinear([&](auto c) { out_breadth1st += c; }, bux::MLP_BREADTH_FIRST);
    REQUIRE(out_breadth1st == "abcdefg");
    //-----------------------------
    std::string out_depth1st;
    po.makeLinear([&](auto c) { out_depth1st += c; }, bux::MLP_DEPTH_FIRST);
    REQUIRE(out_depth1st == "abdecfg");
    //-----------------------------
    std::string out_default;
    po.makeLinear([&](auto c) { out_default += c; });
    REQUIRE(out_default == out_breadth1st);
}
