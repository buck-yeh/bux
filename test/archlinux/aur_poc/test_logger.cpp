/*
    Test cases are organized according to ZOMBIES rules
    http://blog.wingman-sw.com/tdd-guided-by-zombies

    Initially a push then a proof to testability of global logger
*/
#include <bux/Logger.h>     // LOG(), LOG_RAW(), ...
#include <random>           // std::mt19937
#include <catch2/catch_test_macros.hpp>

namespace {

//
//      In-Module Types
//
class C_GizmoLogger: public bux::C_SyncLogger
{
public:

    // Nonvirtuals
    C_GizmoLogger(bux::C_ReenterableOstream &logger): bux::C_SyncLogger(logger), m_underLogger(logger) {}
    int entryDepth() const { return m_macCount; }

    // Re-work bux::C_SyncLogger
    std::ostream *lockLog() override
    {
        auto ret = bux::C_SyncLogger::lockLog();
        const auto count = m_underLogger.lockedCount();
        if (m_macCount < count)
            m_macCount = count;

        return ret;
    }
    std::ostream *lockLog(bux::E_LogLevel ll) override
    {
        auto ret = bux::C_SyncLogger::lockLog(ll);
        const auto count = m_underLogger.lockedCount();
        if (m_macCount < count)
            m_macCount = count;

        return ret;
    }
    //void unlockLog(bool flush) override;

private:

    // Data
    bux::C_ReenterableOstream   &m_underLogger;
    int                         m_macCount{};
};

//
//      In-Module Globals
//
thread_local std::mt19937 g_rng{std::random_device{"/dev/urandom"}()};

//
//      In-Module Functions
//
int inner_2nd_return()
{
    LOG_RAW("Inner 2nd Log");
    return 20;
}

int inner_return()
{
    LOG_RAW("Inner Log {}", inner_2nd_return());
    return 10;
}

std::string ignore_prelog(const std::string &s)
{
    static constexpr const char PRELOG[] = "********** LOGS BEGUN **********";
    auto found = s.find(PRELOG);
    if (found != std::string::npos)
    {
        found = s.find('\n', found + strlen(PRELOG));
        if (found != std::string::npos)
            return s.substr(found + 1);
    }
    return s;
}

} // namespace

namespace bux { namespace user {
std::unique_ptr<C_GizmoLogger> g_log;
I_SyncLog &logger() {
DEF_LOGGER_TAIL_(*g_log)

struct C_Fixture
{
    std::ostringstream          m_out;      // prior to m_logger
    bux::C_ReenterableOstream   m_logger;

    C_Fixture(): m_logger(m_out)
    {
        bux::user::g_log = std::make_unique<C_GizmoLogger>(m_logger);
    }
    auto ignore_prelog() const { return ::ignore_prelog(m_out.str()); }
};

TEST_CASE_METHOD(C_Fixture, "Empty log", "[Z]")
{
    REQUIRE(m_out.str().empty());
    REQUIRE(bux::user::g_log->entryDepth() == 0);
}

TEST_CASE_METHOD(C_Fixture, "One log", "[O]")
{
    LOG_RAW("The one log");
    REQUIRE(ignore_prelog() == "The one log\n");
    REQUIRE(bux::user::g_log->entryDepth() == 1);
}

TEST_CASE_METHOD(C_Fixture, "Scenario: Reentered Logs", "[S]")
{
    LOG_RAW("Outer: Inner Return = {}", inner_return());
    REQUIRE(ignore_prelog() == "Inner 2nd Log\nInner Log 20\nOuter: Inner Return = 10\n");
    REQUIRE(bux::user::g_log->entryDepth() == 3);
}
