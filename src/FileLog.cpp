#include "FileLog.h"
#include <format>     // std::vformat()

namespace fs = std::filesystem;

namespace bux {

//
//      Class Implementation
//
C_PathFmtLogSnap::C_PathFmtLogSnap(T_LocalZone tz): m_tz(tz)
{
    configPath("{:%Y%m%d}.log");
}

C_PathFmtLogSnap &C_PathFmtLogSnap::configPath(const std::string &_pathFmt)
/*! \param [in] _pathFmt Used as `fmt` parameter of `std::vformat()` with one single extra argument to format, ie. timestamp for now.
    \return `*this`

    Each formatted path is the target file for the moment when something is to be logged on demand.
*/
{
    if (_pathFmt.empty())
        RUNTIME_ERROR("Null path format");

    m_PathFmts.clear();
    m_PathFmts.emplace_back(fs::absolute(_pathFmt).string());
    m_FileSizeLimit =
    m_CurPathFmt    = 0;
    return *this;
}

C_PathFmtLogSnap &C_PathFmtLogSnap::enableAutoMkDir(bool yes)
/*! \param [in] yes Whether or not to create subdirectory for the openning log path. The default is _true_.
    \return `*this`

    If enabled, missing subdirs are always created before opening the log file no matter how deep the subdir is.
*/
{
    m_AutoMkDir = yes;
    return *this;
}

C_PathFmtLogSnap &C_PathFmtLogSnap::setBinaryMode(bool enabled)
/*! \param [in] enabled Whether or not to open log files in binary mode. The default is _false_.
    \return `*this`
*/
{
    m_OpenMode = std::ios_base::out;
    if (enabled)
        m_OpenMode |= std::ios_base::binary;

    m_CurrPath.clear(); // trigger reopen
    return *this;
}

std::ostream *C_PathFmtLogSnap::snap()
{
    const auto  t = time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
    const auto  get_new_path = [t,this](auto indFmt) {
        std::string_view pathFmt = m_PathFmts.at(indFmt);
        if (m_tz)
        {
#if LOCALZONE_IS_TIMEZONE
            auto ltm = m_tz->to_local(t);
#else
            auto sys_t = std::chrono::system_clock::to_time_t(t);
            std::chrono::local_time<std::chrono::seconds> ltm(t.time_since_epoch() + std::chrono::seconds(localtime(&sys_t)->tm_gmtoff));
#endif
            return std::vformat(pathFmt, make_format_args(ltm));
        }
        return std::vformat(pathFmt, make_format_args(t));
    };
    std::string nextPath;
    if (m_OldTime == t)
        nextPath = m_CurrPath;
    else
    {
        m_OldTime = t;
        nextPath = get_new_path(m_CurPathFmt);
    }
OpenNewFile:
    if (nextPath.empty())
        return nullptr;

    if (m_CurrPath != nextPath)
        // Different paths -- Repoen it
    {
        if (m_CurrPath.empty())
            // First time
        {
            if (m_FileSizeLimit)
                while (m_CurPathFmt + 1 < m_PathFmts.size() &&
                       fs::is_regular_file(nextPath) &&
                       fs::file_size(nextPath) >= m_FileSizeLimit)
                    nextPath = get_new_path(++m_CurPathFmt);
        }
        else
            // Find the lowest fallback
            while (m_CurPathFmt)
            {
                auto s2 = get_new_path(m_CurPathFmt - 1);
                if (fs::exists(s2))
                    break;

                --m_CurPathFmt;
                nextPath = std::move(s2);
            }

        // At this stage, nextPath is finalized as the new log path for sure
        if (m_AutoMkDir)
            (void)create_directories(fs::path(nextPath).parent_path());

        m_Out.clear();
        if (m_CurrPath.empty())
            // First open
            m_Out.open(nextPath, m_OpenMode|std::ios::ate|std::ios::in);
        else
            // Close the opened file
            m_Out.close();

        if (!m_Out.is_open())
            // Mostly serve for the latter case
        {
            m_Out.clear();
            m_Out.open(nextPath, m_OpenMode);
        }
        if (!m_Out.is_open())
            RUNTIME_ERROR("{}", nextPath);

        m_CurrPath = nextPath;
    }
    else if (
        m_FileSizeLimit &&
        m_CurPathFmt + 1 < m_PathFmts.size() &&
        m_Out.tellp() >= std::streamoff(m_FileSizeLimit))
        // Size limit is reached and we can fallback to use the next path format
    {
        nextPath = get_new_path(++m_CurPathFmt);
        goto OpenNewFile;
    }
    return &m_Out;
}

} // namespace bux
