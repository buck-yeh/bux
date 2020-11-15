#include "FileLog.h"
#include "XException.h" // RUNTIME_ERROR()
#include <filesystem>   // std::filesystem::*

namespace fs = std::filesystem;

namespace bux {

//
//      Class Implementation
//
C_FileLog::C_FileLog(const std::string &_pathFmt): m_Lock(m_Out)
/*! \param [in] _pathFmt \ref InprocSyncReportPathFormat "Path formatting string".
     The default string "\%Y\%m\%d.log" is for daily report.

    What this constructor does is simply call configPath(_pathFmt) and
    enableAutoMkDir(true).

    Example:
    \code
    //
    // Programming IVR global log files
    //

    C_FileLog   BillingLog(".\\billinglog\\%Y%m%d.bil");            // daily
    C_FileLog   RatingLog(".\\billinglog\\%Y%m\\%d_outbound.log");  // daily
    C_FileLog   ErrorLog(".\\log\\%Y%m%p_err.log");     // half-daily
    C_FileLog   DbLog(".\\log\\%Y%m%d%H_db.log");       // hourly
    C_FileLog   ActivityLog(".\\log\\%Y%mcall.log");    // monthly
    \endcode
*/
{
    configPath(_pathFmt);
}

void C_FileLog::configPath(const std::string &_pathFmt)
/*! \anchor InprocSyncReportPathFormat
    \param [in] _pathFmt The third argument of ANSI C time function strftime(),
     see MSDN Library for help. Everytime when virtual method getResource() gets
     called, the current log path is regenerated according to this format
     string and the current timestamp to compare with the previous one. Were
     they different, the internal file stream is reopenned on the new path.
     Here is the format spec for your convience:
     \htmlonly
    <table width="90%" border="1" height="1803">
      <tr>
        <td colspan="2" height="1583"><br>
          The format argument consists of one or more
          codes; as in printf(), the formatting codes are preceded by a percent
          sign (<b>%</b>). Characters that do not begin with <b>%</b> are copied
          unchangedly. The LC_TIME category of the current locale affects the
          output formatting of strftime. (For more information on LC_TIME, see
          setlocale().) The formatting codes for strftime() are listed below:
          <p><b>%a</b></p>
          <p>Abbreviated weekday name</p>
          <p><b>%A</b></p>
          <p>Full weekday name</p>
          <p><b>%b</b></p>
          <p>Abbreviated month name</p>
          <p><b>%B</b></p>
          <p>Full month name</p>
          <p><b>%c</b></p>
          <p>Date and time representation appropriate for locale</p>
          <p><b>%d</b></p>
          <p>Day of month as decimal number (01 - 31)</p>
          <p><b>%H</b></p>
          <p>Hour in 24-hour format (00 - 23)</p>
          <p><b>%I</b></p>
          <p>Hour in 12-hour format (01 - 12)</p>
          <p><b>%j</b></p>
          <p>Day of year as decimal number (001 - 366)</p>
          <p><b>%m</b></p>
          <p>Month as decimal number (01 - 12)</p>
          <p><b>%M</b></p>
          <p>Minute as decimal number (00 - 59)</p>
          <p><b>%p</b></p>
          <p>Current locale's A.M./P.M. indicator for 12-hour clock</p>
          <p><b>%S</b></p>
          <p>Second as decimal number (00 - 59)</p>
          <p><b>%U</b></p>
          <p>Week of year as decimal number, with Sunday as first day of week
            (00 - 53)</p>
          <p><b>%w</b></p>
          <p>Weekday as decimal number (0 - 6; Sunday is 0)</p>
          <p><b>%W</b></p>
          <p>Week of year as decimal number, with Monday as first day of week
            (00 - 53)</p>
          <p><b>%x</b></p>
          <p>Date representation for current locale</p>
          <p><b>%X</b></p>
          <p>Time representation for current locale</p>
          <p><b>%y</b></p>
          <p>Year without century, as decimal number (00 - 99)</p>
          <p><b>%Y</b></p>
          <p>Year with century, as decimal number</p>
          <p><b>%z, %Z</b></p>
          <p>Time-zone name or abbreviation; no characters if time zone is unknown</p>
          <p><b>%%</b></p>
          <p>Percent sign</p>
          <p>As in the printf function, the <b>#</b> flag may prefix any formatting
            code. In that case, the meaning of the format code is changed as follows.<br>
          </p>
        </td>
      </tr>
      <tr>
        <td width="39%" height="4">
          <CENTER>Format Code</CENTER>
        </td>
        <td width="61%" height="4">
          <CENTER>Meaning</CENTER>
        </td>
      </tr>
      <tr>
        <td width="39%" height="19">
          <CENTER><b>%#a</b>, <b>%#A</b>, <b>%#b</b>, <b>%#B</b>,
            <b>%#p</b>, <b>%#X</b>, <b>%#z</b>, <b>%#Z</b>, <b>%#%</b></CENTER>
        </td>
        <td width="61%" height="19">
          <b>#</b> flag is ignored.
        </td>
      </tr>
      <tr>
        <td width="39%" height="16">
          <CENTER><b>%#c</b></CENTER>
        </td>
        <td width="61%" height="16">
          Long date and time representation, appropriate for
          current locale. For example: "Tuesday, March 14, 1995, 12:41:29".
        </td>
      </tr>
      <tr>
        <td width="39%" height="2">
          <CENTER><b>%#x</b></CENTER>
        </td>
        <td width="61%" height="2">
          Long date representation, appropriate to current locale.
            For example: "Tuesday, March 14, 1995".
        </td>
      </tr>
      <tr>
        <td width="39%" height="2">
          <CENTER><b>%#d</b>, <b>%#H</b>, <b>%#I</b>, <b>%#j</b>,
            <b>%#m</b>, <b>%#M</b>, <b>%#S</b>, <b>%#U</b>, <b>%#w</b>, <b>%#W</b>,
            <b>%#y</b>, <b>%#Y</b></CENTER>
        </td>
        <td width="61%" height="2">
          Remove leading zeros (if any).
        </td>
      </tr>
    </table>
    \endhtmlonly

    Example: see \ref C_FileLog::C_FileLog "construct".

    \pre _pathFmt is not empty.
*/
{
    if (_pathFmt.empty())
        RUNTIME_ERROR("Null path format")

    std::scoped_lock _(m_Lock.m_LockOut);
    m_PathFmt = fs::absolute(_pathFmt).string();
}

void C_FileLog::enableAutoMkDir(bool yes)
/*! \param [in] yes Whether or not to create subdirectory for the openning log path.
     The default is true.

    If enabled, getResource() always creates the subdir before openning the log
    path no matter how deep the subdir is.

    Since constructor always enables it, you hradly need to call this method
    explicitly.
*/
{
    m_AutoMkDir = yes;
}

std::ostream &C_FileLog::getResource()
{
    std::ostream &ret = m_Lock.getResource();
    if (&ret == &m_Out)
    {
        std::string         s;
        const auto          t = C_MyClock::now();
        if (t != m_OldTime)
        {
            m_OldTime = t;
            const auto ct = time_t(std::chrono::duration_cast<std::chrono::seconds>(t.time_since_epoch()).count());
            auto bytes = m_PathFmt.size() * 2;
            const auto buf = std::make_unique<char[]>(bytes);
            bytes = strftime(buf.get(), bytes, m_PathFmt.c_str(), localtime(&ct));
            s.assign(buf.get(), bytes);
        }
        if (!s.empty() && m_CurrPath != s)
            // Different paths -- Repoen it
        {
            if (m_AutoMkDir)
                (void)create_directories(fs::path(s).parent_path());

            m_Out.clear();
            if (m_CurrPath.empty())
                // First open
                m_Out.open(s, m_OpenMode|std::ios::ate|std::ios::in);
            else
                // Close the opened file
                m_Out.close();

            if (!m_Out.is_open())
                // Mostly serve for the latter case
            {
                m_Out.clear();
                m_Out.open(s, m_OpenMode);
            }
            if (!m_Out.is_open())
                RUNTIME_ERROR(s)

            m_CurrPath = s;
        }
    }
    return ret;
}

void C_FileLog::releaseResource(std::ostream &out)
{
    m_Lock.releaseResource(out);
}

void C_FileLog::setBinaryMode(bool enabled)
{
    std::scoped_lock _(m_Lock.m_LockOut);
    m_OpenMode =std::ios_base::out;
    if (enabled)
        m_OpenMode |=std::ios_base::binary;

    m_CurrPath.clear(); // trigger reopen
}

} // namespace bux
