#include <bux/Logger.h>     // DEF_LOGGER_FILE()
#include <fstream>          // std::ofstream

DEF_LOGGER_FILE("log/test.log") //, LL_INFO)

int main()
{
    if (bux::C_UseLog u{bux::logger()})
        *u <<std::boolalpha <<"LOGGER_USE_LOCAL_TIME_: " <<LOGGER_USE_LOCAL_TIME_ <<"\n";

    LOG(LL_FATAL,   "Hello fatal");
    FUNLOG;
    LOG(LL_ERROR,   "Hello error");
    LOG(LL_WARNING, "Hello warning");
    FUNLOGX("Inner");
    LOG(LL_INFO,    "Hello info");
    LOG(LL_VERBOSE, "Hello verbose");
}
