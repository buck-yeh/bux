/*
    Test cases are organized by ZOMBIES rules
    http://blog.wingman-sw.com/tdd-guided-by-zombies
*/
#include "bux/AtomiX.h"
#include "bux/ParaUtil.h"   // bux::C_NumIter<>
#include <execution>        // std::execution::par_unseq

#define CATCH_CONFIG_MAIN   // This tells Catch to provide a main() - only do this in one cpp file
#include <catch2/catch.hpp>

TEST_CASE("Empty cache", "[Z]")
{
    bux::C_SpinCacheT<int,std::string> cache;
    REQUIRE(cache.size() == 0);
}

TEST_CASE("Cache for one thread", "[O]")
{
    bux::C_SpinCacheT<int,std::string> cache;
    for (int i = 0; i < 100; ++i)
    {
        const int t = i % 10;
        cache(t, [t](std::string &s){ s = std::to_string(t); });
    }
    REQUIRE(cache.size() == 10);
}

TEST_CASE("Cache for many threads", "[M]")
{
    bux::C_SpinCacheT<int,std::string> cache;
    using C_Iter = bux::C_NumIter<int>;
    std::for_each(std::execution::par, C_Iter(0), C_Iter(1000000), [&](auto i){
        const int t = i % 10;
        cache(t, [t](std::string &s){ s = std::to_string(t); });
    });
    REQUIRE(cache.size() == 10);
}

/* No case
TEST_CASE("Boundary checking", "[B]")
{
}

TEST_CASE("Interface", "[I]")
{
}

TEST_CASE("Exceptions", "[E]")
{
}

TEST_CASE("Scenario: ???", "[S]")
{
}
*/
