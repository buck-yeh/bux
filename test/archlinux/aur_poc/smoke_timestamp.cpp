#include <bux/LogStream.h>  // bux::timestamp(), bux::logTrace(), LOGTITLE()
#include <iostream>         // std::cout

int main()
{
    //std::cout <<bux::timestamp <<'\n';
    bux::timestamp(std::cout) <<'\n';                       // system time
    bux::timestamp(std::cout, bux::local_zone()) <<'\n';    // local timd
    //std::cout <<bux::logTrace <<'\n';
    bux::logTrace(std::cout) <<'\n';                        // system time
    bux::logTrace(std::cout, bux::local_zone()) <<'\n';     // local time
    LOGTITLE(std::cout, bux::T_LocalZone()) <<"Hello !!!\n";// system time
    LOGTITLE(std::cout, bux::local_zone()) <<"Hello !!!\n"; // local time
}
