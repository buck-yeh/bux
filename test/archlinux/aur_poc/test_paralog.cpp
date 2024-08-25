/*
    Test cases are organized according to ZOMBIES rules
    http://blog.wingman-sw.com/tdd-guided-by-zombies
*/
#include <bux/Logger.h>     // DEF_LOGGER_TAIL_, LOG(), LOG_RAW()
#include <bux/ParaLog.h>    // bux::C_ParaLog
#include <random>           // std::mt19937

#define CATCH_CONFIG_MAIN   // This tells Catch to provide a main() - only do this in one cpp file
#include <catch2/catch_test_macros.hpp>

namespace bux { namespace user {    // Mildly modified from definition of DEF_PARA_LOGGER
std::unique_ptr<C_ParaLog> g_log;
I_SyncLog &logger() {
DEF_LOGGER_TAIL_(*g_log)

namespace {

//
//      In-Module Globals
//
thread_local std::mt19937 g_rng{std::random_device{"/dev/urandom"}()};

//
//      In-Module Functions
//
void reset_log()
{
    bux::user::g_log = std::make_unique<bux::C_ParaLog>();
}

void test_test()
{
    LOG_RAW("Test Test");
    LOG(LL_FATAL, "Fatal Test");
}

} // namespace

TEST_CASE("Empty paralog", "[Z]")
{
    reset_log();
    REQUIRE_NOTHROW(test_test());
}
