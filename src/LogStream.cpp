#include "LogStream.h"
//---------------------------------------------------------------
#include <ostream>          // std::ostream
#include <chrono>           // std::chrono::system_clock
#include <cinttypes>        // imaxdiv()

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

namespace bux {

std::ostream &timestamp(std::ostream &out)
/*! \param out Output stream to add timestamp to.
    \return \em out

    Write timestamp of the form "yyyy/mm/dd hh:mm:sss " to \em out.
*/
{
    typedef std::chrono::system_clock myclock;

    static char YMDHMS[30];
    static myclock::time_point old_time;

    const auto cur_time = myclock::now();
    if (cur_time != old_time)
    {
        auto d = imaxdiv(std::chrono::duration_cast<std::chrono::milliseconds>(cur_time.time_since_epoch()).count(), 1000);
        const time_t t = time_t(d.quot);
        char *const p = YMDHMS + strftime(YMDHMS, sizeof YMDHMS, "%Y/%m/%d %H:%M:%S.", localtime(&t));
        for (int i = 3; i--;)
        {
            p[i] = char('0' + d.rem % 10);
            d.rem /= 10;
        }
        p[3] = 0;
        old_time = cur_time;
    }
    return out <<YMDHMS;
}

std::ostream &logTrace(std::ostream &out)
/*! \param out Output stream to add trace(line prefix) to.
    \return \em out

    Log timestamp & thread id
*/
{
    return timestamp(out) <<" tid" <<TID_ <<' ';
}

} // namespace bux
