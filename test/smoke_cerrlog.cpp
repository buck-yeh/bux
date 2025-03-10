//#define TURN_OFF_LOGGER_
//#define LOGGER_USE_LOCAL_TIME_ std::chrono::get_tzdb().current_zone()
//#define LOGGER_USE_LOCAL_TIME_ std::chrono::get_tzdb().locate_zone("Asia/Taipei")
#include <bux/Logger.h>     // DEF_LOGGER_CERR()
#include <iostream>         // std::cerr

DEF_LOGGER_CERR(LL_VERBOSE)

int main()
{
    std::cerr <<std::boolalpha <<"LOGGER_USE_LOCAL_TIME_: " <<LOGGER_USE_LOCAL_TIME_ <<"\n";
    LOG_RAW("Raw beginning");
    LOG(LL_FATAL,   "Hello fatal");
    FUNLOG;
    LOG(LL_ERROR,   "Hello error");
    LOG(LL_WARNING, "Hello warning");
    FUNLOGX("Inner");
    LOG(LL_INFO,    "Hello info");
    LOG(LL_VERBOSE, "Hello verbose");
    LOG_RAW("Raw ending");
}
