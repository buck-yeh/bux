#include <bux/Logger.h>     // DEF_LOGGER_CERR()
#include <iostream>         // std::cerr

//DEF_LOGGER_CERR()
DEF_LOGGER_CERR(LL_VERBOSE)

int main()
{
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
