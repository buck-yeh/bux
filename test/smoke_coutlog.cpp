//#define TURN_OFF_LOGGER_
#include <bux/Logger.h>     // DEF_LOGGER_COUT()
#include <iostream>         // std::cout

DEF_LOGGER_COUT()
//DEF_LOGGER_COUT(LL_WARNING)

int main()
{
    LOG(LL_FATAL,   "Hello fatal");
    LOG(LL_ERROR,   "Hello error");
    FUNLOGX("Outer");
    LOG(LL_WARNING, "Hello warning");
    FUNLOG;
    LOG(LL_INFO,    "Hello info");
    LOG(LL_VERBOSE, "Hello verbose");
}
