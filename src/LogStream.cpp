#include "LogStream.h"
//---------------------------------------------------------------
#include <format>           // std::format()
#include <ostream>          // std::ostream
#include <cinttypes>        // imaxdiv()
#include <cstring>          // std::strcpy()

#define STD_FORMAT_CHRONO_
#ifndef STD_FORMAT_CHRONO_
#include <ctime>            // std::localtime(), std::strftime()
#endif

#ifdef __APPLE__
// based on https://developer.apple.com/library/archive/documentation/Porting/Conceptual/PortingUnix/compiling/compiling.html#//apple_ref/doc/uid/TP40002850-SW13
#include <pthread.h>
#define TID_    pthread_self()
#elifdef __GNUC__
#ifndef _GNU_SOURCE
#define _GNU_SOURCE         // or _BSD_SOURCE or _SVID_SOURCE
#endif
#include <unistd.h>
#include <sys/syscall.h>    // for SYS_xxx definitions
#define TID_    syscall(SYS_gettid)
#elifdef _WIN32
#include <windows.h>        // GetCurrentThreadId()

#define TID_    GetCurrentThreadId()
#else
#include <thread>           // std::this_thread::get_id()

#define TID_    std::this_thread::get_id()
#endif

namespace bux {

std::ostream &timestamp(std::ostream &out, T_LocalZone tz)
/*! \param out Output stream to add timestamp to.
    \param [in] tz Pointer to the time zone to convert system clock to. In case of **nullptr**, the timestamp will use system clock.
    \return \em out

    Write timestamp in format of "yyyy/mm/dd hh:mm:sss " to \em out.
*/
{
    typedef std::chrono::system_clock myclock;
    static char YMDHMS[30];
    static std::chrono::time_point<myclock,std::chrono::milliseconds> old_time;
    static T_LocalZone old_tz{};
    const auto cur_time = time_point_cast<std::chrono::milliseconds>(myclock::now());
    if (cur_time != old_time || tz != old_tz)
    {
#ifdef STD_FORMAT_CHRONO_
        constexpr const std::string_view TIMESTAMP_FMT = "{:%Y/%m/%d %H:%M:%S}";
        std::string tm_str;
        if (tz)
        {
#if LOCALZONE_IS_TIMEZONE
            auto ltm = tz->to_local(cur_time);
#else
            auto sys_t = std::chrono::system_clock::to_time_t(cur_time);
            std::chrono::local_time<std::chrono::milliseconds> ltm(cur_time.time_since_epoch() + std::chrono::seconds(localtime(&sys_t)->tm_gmtoff));
#endif
            tm_str = std::format(TIMESTAMP_FMT, ltm);
        }
        else
            tm_str = std::format(TIMESTAMP_FMT, cur_time);

        std::strcpy(YMDHMS, tm_str.c_str());
#else
        auto d = imaxdiv(cur_time.time_since_epoch().count(), 1000);
        time_t t = d.quot;
        std::sprintf(YMDHMS + std::strftime(YMDHMS, sizeof YMDHMS, "%Y/%m/%d %H:%M:%S", std::localtime(&t)), ".%03" PRIdMAX, d.rem);
#endif
        old_time = cur_time;
        old_tz = tz;
    }
    return out <<YMDHMS;
}

std::ostream &logTrace(std::ostream &out, T_LocalZone tz)
/*! \param out Output stream to add trace(line prefix) to.
    \param tz Pointer to the time zone to convert system clock to. In case of **nullptr**, the timestamp will use system clock.
    \return \em out

    Log timestamp & thread id
*/
{
    return timestamp(out,tz) <<" tid" <<TID_ <<' ';
}

} // namespace bux
