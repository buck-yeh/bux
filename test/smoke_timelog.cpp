#include <bux/Logger.h>     // DEF_LOGGER_FILES(), DEF_CONFIGURABLE_LOGGER_FILES
#include <bux/FileLog.h>    // bux::C_PathFmtLogSnap
#include <chrono>           // std::chrono::system_clock
#include <random>           // std::mt19937
#include <stdlib.h>         // strtoul()
#include <thread>           // std::this_thread::sleep_for()

#define SET_SIZE_LIMIT_
#ifndef SET_SIZE_LIMIT_
DEF_LOGGER_FILES("timelog/%y%m%d_%H%M.log")
#else
DEF_CONFIGURABLE_LOGGER_FILES
#endif

thread_local std::mt19937 g_rng{std::random_device{"/dev/urandom"}()};

static const struct { bux::E_LogLevel ll; const char *msg; } LOG_SRC[] = {
    {LL_FATAL,   "Hello fatal"},
    {LL_ERROR,   "Hello error"},
    {LL_WARNING, "Hello warning"},
    {LL_INFO,    "Hello info"},
    {LL_VERBOSE, "Hello verbose"},
};

int main(int argc, const char *argv[])
{
    if (argc <= 1)
    {
        for (auto i: LOG_SRC)
#if defined(FMT_VERSION) && FMT_VERSION >= 80000
            LOG(i.ll, fmt::runtime(i.msg));
#else
            LOG(i.ll, i.msg);
#endif
    }
    else
    {
#ifdef SET_SIZE_LIMIT_
        bux::user::g_snap.configPath(65536, std::array{
            "timelog/%y%m%d-%H.log",
            "timelog/%y%m%d-%H-%M.log",
            "timelog/%y%m%d-%H-%M-%S.log"
        });
#endif
        using C_MyClock = std::chrono::system_clock;
        const auto deadline = C_MyClock::now() + std::chrono::seconds(strtoul(argv[1],nullptr,0));
        while (C_MyClock::now() < deadline)
        {
            const auto &i = LOG_SRC[std::uniform_int_distribution<size_t>{0,std::size(LOG_SRC)-1}(g_rng)];
#if defined(FMT_VERSION) && FMT_VERSION >= 80000
            LOG(i.ll, fmt::runtime(i.msg));
#else
            LOG(i.ll, i.msg);
#endif
            std::this_thread::sleep_for(
#ifdef SET_SIZE_LIMIT_
                std::chrono::milliseconds(100)
#else
                std::chrono::seconds(1)
#endif
            );
        }
    }
}
