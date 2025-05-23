//#define LOGGER_USE_LOCAL_TIME_ false
#include <bux/FileLog.h>    // bux::C_PathFmtLogSnap
#include <bux/Logger.h>     // DEF_PARA_LOGGER
#include <bux/ParaLog.h>    // bux::C_ParaLog
#include <iostream>         // std::cout, std::cerr
#include <random>           // std::mt19937
#include <thread>           // std::thread, std::this_thread::sleep_for()
#ifdef _WIN32
#include <conio.h>          // _kbhit(), _getch()
#else
#include <curses.h>         // getch(), ...
#endif

DEF_PARA_LOGGER // local/system time already dominated by LOGGER_USE_LOCAL_TIME_

namespace {

thread_local std::mt19937 g_rng{std::random_device{"/dev/urandom"}()};
bool g_stop{};

} // namespace

int main()
{
    using bux::user::g_paraLog;
    g_paraLog.addChild(std::cout, LL_WARNING);                          // to console
    g_paraLog.addChildT<bux::C_PathFmtLogSnap>({}, LL_VERBOSE, false);  // default path formatted in system time
    g_paraLog.addChildT<bux::C_PathFmtLogSnap>([](auto &logger)         // fallback path formats on log size 2MB, each formatted in local time
    {
        logger.configPath(2UL<<20, std::array{
            "logs/st{:%Y-%m-%d/%y%m%d}.log",
            "logs/st{:%Y-%m-%d/%y%m%d-%H}.log",
            "logs/st{:%Y-%m-%d/%y%m%d-%H-%M}.log"});
    });
    g_paraLog.addChildT<std::ofstream,bux::C_OstreamHolder>({}, LL_ERROR, "errors.txt"); // to log file "errors.txt"

    if (bux::C_UseLog u{bux::logger()})
        *u <<std::boolalpha <<"LOGGER_USE_LOCAL_TIME_: " <<LOGGER_USE_LOCAL_TIME_ <<"\n";

#ifndef _WIN32
    initscr();
    cbreak();
    noecho();
    scrollok(stdscr, TRUE);
    nodelay(stdscr, TRUE);
#endif
    std::list<std::thread> loops;
    for (int i = 0; i < 20; ++i)
        loops.emplace_back([]{
            while (!g_stop)
            {
                static constinit const struct { bux::E_LogLevel ll; const char *msg; } LOG_SRC[] = {
                    {LL_FATAL,   "fatal"},
                    {LL_ERROR,   "error"},
                    {LL_WARNING, "warning"},
                    {LL_INFO,    "info"},
                    {LL_DEBUG,   "debug"},
                    {LL_VERBOSE, "verbose"},
                };
                const auto &src = LOG_SRC[std::uniform_int_distribution<size_t>{0,std::size(LOG_SRC)-1}(g_rng)];
                const auto sleep_ms = std::uniform_int_distribution<size_t>{0,19}(g_rng);
                LOG(src.ll, "Hello {} and wait for {}ms", src.msg, sleep_ms);
                std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
            }
        });

    while (!g_stop)
    {
#ifdef _WIN32
        if (_kbhit()) switch (_getch())
#else
        switch (getch())
#endif
        {
        case 'Q':
        case 'q':
        case '\x1b':
            g_stop = true;
            break;
        default:
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
#ifndef _WIN32
    endwin();
#endif

    for (auto &i: loops)
        i.join();
}
