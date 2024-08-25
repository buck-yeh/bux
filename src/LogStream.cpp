#include "LogStream.h"
//---------------------------------------------------------------
#include <ostream>          // std::ostream
#include <cinttypes>        // imaxdiv()
#include <cstring>          // std::strcpy()
#include <ctime>            // std::localtime(), std::strftime()

#ifdef __GNUC__
#ifndef _GNU_SOURCE
#define _GNU_SOURCE         // or _BSD_SOURCE or _SVID_SOURCE
#endif
#include <unistd.h>
#include <sys/syscall.h>    // for SYS_xxx definitions

#define TID_    syscall(SYS_gettid)
#elif defined(_WIN32)
#include <windows.h>        // GetCurrentThreadId()

#define TID_    GetCurrentThreadId()
#else
#include <thread>           // std::this_thread::get_id()

#define TID_    std::this_thread::get_id()
#endif

#define STD_FORMAT_CHRONO_

namespace bux {

std::ostream &timestamp(std::ostream &out, const std::chrono::time_zone *tz)
/*! \param out Output stream to add timestamp to.
    \param [in] tz Pointer to the time zone to convert system clock to. In case of **nullptr**, the timestamp will use system clock.
    \return \em out

    Write timestamp in format of "yyyy/mm/dd hh:mm:sss " to \em out.
*/
{
    typedef std::chrono::system_clock myclock;
    static char YMDHMS[30];
    static std::chrono::time_point<myclock,std::chrono::milliseconds> old_time;
    static const std::chrono::time_zone *old_tz = nullptr;
    const auto cur_time = time_point_cast<std::chrono::milliseconds>(myclock::now());
    if (cur_time != old_time || tz != old_tz)
    {
#ifdef STD_FORMAT_CHRONO_
        #define TIMESTAMP_FMT "{:%Y/%m/%d %H:%M:%S}"
        auto tm_str = tz? std::format(TIMESTAMP_FMT, tz->to_local(cur_time)): std::format(TIMESTAMP_FMT, cur_time);
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

std::ostream &logTrace(std::ostream &out, const std::chrono::time_zone *tz)
/*! \param out Output stream to add trace(line prefix) to.
    \param tz Pointer to the time zone to convert system clock to. In case of **nullptr**, the timestamp will use system clock.
    \return \em out

    Log timestamp & thread id
*/
{
    return timestamp(out,tz) <<" tid" <<TID_ <<' ';
}

} // namespace bux
