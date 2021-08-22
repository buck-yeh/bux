#include <bux/FileLog.h>    // bux::C_PathFmtLogSnap
#include <bux/Logger.h>     // DEF_PARA_LOGGER
#include <bux/ParaLog.h>    // bux::C_ParaLog
#include <algorithm>        // std::for_each()
#include <execution>        // std::execution::*
#include <iostream>         // std::cout, std::cerr
#include <random>           // std::mt19937
#include <thread>           // std::thread, std::this_thread::sleep_for()
#ifdef _WIN32
#include <conio.h>          // _kbhit(), _getch()
#else
#include <curses.h>         // getch(), ...
#endif

thread_local std::mt19937 g_rng{std::random_device{"/dev/urandom"}()};

static bool g_stop;

static const struct { bux::E_LogLevel ll; const char *msg; } LOG_SRC[] = {
    {LL_FATAL,   "fatal"},
    {LL_ERROR,   "error"},
    {LL_WARNING, "warning"},
    {LL_INFO,    "info"},
    {LL_VERBOSE, "verbose"},
};

bux::C_PathFmtLogSnap fmt_snap(2UL<<20, std::initializer_list{
                                            "logs/%y%m%d-0000.log",
                                            "logs/%y%m%d-%H00.log",
                                            "logs/%y%m%d-%H%M.log"});
DEF_PARA_LOGGER


int main()
{
    bux::user::g_paraLog.addChild(std::make_unique<bux::C_ReenterableOstream>(std::cout, LL_WARNING));
    bux::user::g_paraLog.addChild(std::make_unique<bux::C_ReenterableOstreamSnap>(fmt_snap, LL_VERBOSE));

    std::list<std::thread> loops;
    for (int i = 0; i < 20; ++i)
        loops.emplace_back([]{
            while (!g_stop)
            {
                const auto &src = LOG_SRC[std::uniform_int_distribution<size_t>{0,std::size(LOG_SRC)-1}(g_rng)];
                const auto sleep_ms = std::uniform_int_distribution<size_t>{0,19}(g_rng);
                LOG(src.ll, "Hello {} and wait for {}ms", src.msg, sleep_ms);
                std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
            }
        });

#ifndef _WIN32
    initscr();
    cbreak();
    noecho();
    scrollok(stdscr, TRUE);
    nodelay(stdscr, TRUE);
#endif
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
