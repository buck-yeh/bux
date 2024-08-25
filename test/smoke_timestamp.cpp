#include <bux/LogStream.h>  // bux::timestamp(), bux::logTrace(), LOGTITLE()
#include <iostream>         // std::cout

int main()
{
    //std::cout <<bux::timestamp <<'\n';
    bux::timestamp(std::cout) <<'\n';
    bux::timestamp(std::cout, std::chrono::get_tzdb().current_zone()) <<'\n';
    //std::cout <<bux::logTrace <<'\n';
    bux::logTrace(std::cout) <<'\n';
    bux::logTrace(std::cout, std::chrono::get_tzdb().current_zone()) <<'\n';
    LOGTITLE(std::cout, nullptr) <<"Hello !!!\n";
    LOGTITLE(std::cout, std::chrono::get_tzdb().current_zone()) <<"Hello !!!\n";
}
