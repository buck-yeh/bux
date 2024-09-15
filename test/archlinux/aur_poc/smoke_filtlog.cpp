#define LOGGER_USE_LOCAL_TIME_ false
#include <bux/FileLog.h>    // bux::C_PathFmtLogSnap
#include <bux/Logger.h>     // DEF_PARA_LOGGER
#include <bux/ParaLog.h>    // bux::C_ParaLog
#include <random>           // std::mt19937

DEF_PARA_LOGGER

namespace {

thread_local std::mt19937 g_rng{std::random_device{"/dev/urandom"}()};
#define RAND_ARRAY_INDEX(arr)  std::uniform_int_distribution<size_t>{0,std::size(arr)-1}(g_rng)

struct FC_MatchStr
{
    const std::string       m_key;

    FC_MatchStr(auto key): m_key(key) {}
    bool operator()(std::string_view s) const { return s.contains(m_key); }
};

} // namespace

int main()
{
    bux::user::g_paraLog.addChildT<bux::C_PathFmtLogSnap>([](auto &logger)
    {
        logger.configPath(2UL<<20, std::array{
                                    "logs/all/{:%y%m%d}.log",
                                    "logs/all/{:%y%m%d-%H}.log",
                                    "logs/all/{:%y%m%d-%H-%M}.log"});
    });
    auto
    nodes = bux::user::g_paraLog.partitionBy(std::initializer_list<FC_MatchStr>{"[foo]", "[bar]"});
    nodes[0].addChildT<bux::C_PathFmtLogSnap>([](auto &logger){ logger.configPath("logs/{:%y-%m-%d_foo.log"); });
    nodes[1].addChildT<bux::C_PathFmtLogSnap>([](auto &logger){ logger.configPath("logs/{:%y-%m-%d_bar.log"); });
    nodes[2].addChildT<bux::C_PathFmtLogSnap>([](auto &logger){ logger.configPath("logs/{:%y-%m-%d_no_foo_bar.log"); });
    nodes = bux::user::g_paraLog.partitionBy(std::initializer_list<FC_MatchStr>{"[eeny]", "[meeny]", "[miny]", "[moe]"});
    nodes[0].addChildT<bux::C_PathFmtLogSnap>([](auto &logger){ logger.configPath("logs/{:%y-%m-%d}_eeny.log"); });
    nodes[1].addChildT<bux::C_PathFmtLogSnap>([](auto &logger){ logger.configPath("logs/{:%y-%m-%d}_meeny.log"); });
    nodes[2].addChildT<bux::C_PathFmtLogSnap>([](auto &logger){ logger.configPath("logs/{:%y-%m-%d}_miny.log"); });
    nodes[3].addChildT<bux::C_PathFmtLogSnap>([](auto &logger){ logger.configPath("logs/{:%y-%m-%d}_moe.log"); });
    nodes.matchedNone().addChildT<bux::C_PathFmtLogSnap>([](auto &logger)
    {
        logger.configPath("logs/{:%y-%m-%d}_no_eeny_meeny_miny_moe.log");
    });

    auto
    subnodes = nodes[0].partitionBy(std::initializer_list<FC_MatchStr>{"[foo]", "[bar]"});
    subnodes[0].addChildT<bux::C_PathFmtLogSnap>([](auto &logger){ logger.configPath("logs/eeny/{:%y-%m-%d_foo.log"); });
    subnodes[1].addChildT<bux::C_PathFmtLogSnap>([](auto &logger){ logger.configPath("logs/eeny/{:%y-%m-%d_bar.log"); });
    subnodes[2].addChildT<bux::C_PathFmtLogSnap>([](auto &logger){ logger.configPath("logs/eeny/{:%y-%m-%d_no_foo_bar.log"); });
    subnodes = nodes[1].partitionBy(std::initializer_list<FC_MatchStr>{"[foo]", "[bar]"});
    subnodes[0].addChildT<bux::C_PathFmtLogSnap>([](auto &logger){ logger.configPath("logs/meeny/{:%y-%m-%d_foo.log"); });
    subnodes[1].addChildT<bux::C_PathFmtLogSnap>([](auto &logger){ logger.configPath("logs/meeny/{:%y-%m-%d_bar.log"); });
    subnodes[2].addChildT<bux::C_PathFmtLogSnap>([](auto &logger){ logger.configPath("logs/meeny/{:%y-%m-%d_no_foo_bar.log"); });
    subnodes = nodes[2].partitionBy(std::initializer_list<FC_MatchStr>{"[foo]", "[bar]"});
    subnodes[0].addChildT<bux::C_PathFmtLogSnap>([](auto &logger){ logger.configPath("logs/miny/{:%y-%m-%d_foo.log"); });
    subnodes[1].addChildT<bux::C_PathFmtLogSnap>([](auto &logger){ logger.configPath("logs/miny/{:%y-%m-%d_bar.log"); });
    subnodes[2].addChildT<bux::C_PathFmtLogSnap>([](auto &logger){ logger.configPath("logs/miny/{:%y-%m-%d_no_foo_bar.log"); });
    subnodes = nodes[3].partitionBy(std::initializer_list<FC_MatchStr>{"[foo]", "[bar]"});
    subnodes[0].addChildT<bux::C_PathFmtLogSnap>([](auto &logger){ logger.configPath("logs/moe/{:%y-%m-%d_foo.log"); });
    subnodes[1].addChildT<bux::C_PathFmtLogSnap>([](auto &logger){ logger.configPath("logs/moe/{:%y-%m-%d_bar.log"); });
    subnodes[2].addChildT<bux::C_PathFmtLogSnap>([](auto &logger){ logger.configPath("logs/moe/{:%y-%m-%d_no_foo_bar.log"); });
    subnodes = nodes[4].partitionBy(std::initializer_list<FC_MatchStr>{"[foo]", "[bar]"});
    subnodes[0].addChildT<bux::C_PathFmtLogSnap>([](auto &logger){ logger.configPath("logs/no_eeny_meeny_miny_moe/{:%y-%m-%d_foo.log"); });
    subnodes[1].addChildT<bux::C_PathFmtLogSnap>([](auto &logger){ logger.configPath("logs/no_eeny_meeny_miny_moe/{:%y-%m-%d_bar.log"); });
    subnodes[2].addChildT<bux::C_PathFmtLogSnap>([](auto &logger){ logger.configPath("logs/no_eeny_meeny_miny_moe/{:%y-%m-%d_no_foo_bar.log"); });

    if (bux::C_UseLog u{bux::logger()})
        *u <<std::boolalpha <<"LOGGER_USE_LOCAL_TIME_: " <<LOGGER_USE_LOCAL_TIME_ <<"\n";

    for (int i = 0; i < 200;)
        for (auto j: {"[foo]", "[bar]", ""})
            for (auto k: {"[eeny]", "[meeny]", "[miny]", "[moe]", ""})
            {
                static const struct { bux::E_LogLevel ll; const char *msg; } LOG_SRC[] = {
                    {LL_FATAL,   "fatal"},
                    {LL_ERROR,   "error"},
                    {LL_WARNING, "warning"},
                    {LL_INFO,    "info"},
                    {LL_VERBOSE, "verbose"},
                };
                const auto &src = LOG_SRC[RAND_ARRAY_INDEX(LOG_SRC)];
                LOG(src.ll, "{} Hello {} {:010} {}", j, src.msg, ++i, k);
            }
}
