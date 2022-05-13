#pragma once

#include "SyncLog.h"    // bux::I_SyncLog, bux::I_SnapT<>
#include "XException.h" // RUNTIME_ERROR()
#include <chrono>       // std::chrono::system_clock
#include <filesystem>   // std::filesystem::*
#include <fstream>      // std::ofstream
#include <string>       // std::string
#include <vector>       // std::vector<>

namespace bux {

//
//      Types
//
class C_PathFmtLogSnap: public I_SnapT<std::ostream*>
/*! Configurable to automatically change the output path according to the current timestamp.
*/
{
public:

    // Nonvirtuals
    explicit C_PathFmtLogSnap(const std::string &pathFmt = "%Y%m%d.log")
    /*! \param [in] pathFmt \ref InprocSyncReportPathFormat "Path formatting string".
         The default string "\%Y\%m\%d.log" is for daily report.

        This constructor simply calls configPath(_pathFmt)

        Example:
        \code
        //
        // Recommended log file formats according to timestamp
        //

        C_PathFmtLogSnap   BillingLog("./billinglog/%Y%m%d.bil");          // daily
        C_PathFmtLogSnap   RatingLog("./billinglog/%Y%m/%d_outbound.log"); // daily
        C_PathFmtLogSnap   ErrorLog("./log/%Y%m%p_err.log");     // half-daily
        C_PathFmtLogSnap   DbLog("./log/%Y%m%d%H_db.log");       // hourly
        C_PathFmtLogSnap   ActivityLog("./log/%Y%mcall.log");    // monthly
        \endcode
    */
    {
        configPath(pathFmt);
    }
    C_PathFmtLogSnap(uintmax_t fsize_in_bytes, const auto &fallbackPaths)
    /*! \param [in] fsize_in_bytes Size limit in bytes for each log file.
        \param [in] fallbackPaths Fallback path format list, ...

        This constructor simply calls configPath(fsize_in_bytes, fallbackPaths)

        Example:
        \code
        //
        // Demo fallback file formats from daily to per-minute everytime 16MB file size is reached.
        //

        C_PathFmtLogSnap   snap(16UL<<20, std::array{
                                            "logs/%y%m%d-0000.log",
                                            "logs/%y%m%d-%H00.log",
                                            "logs/%y%m%d-%H%M.log"});
        \endcode
    */
    {
        configPath(fsize_in_bytes, fallbackPaths);
    }
    void configPath(const std::string &pathFmt);
    void configPath(uintmax_t fsize_in_bytes, const auto &fallbackPaths) requires
        requires {
            std::begin(fallbackPaths);
            { *std::begin(fallbackPaths) }-> std::convertible_to<std::string>;
            std::end(fallbackPaths);
        }
    {
        m_PathFmts.clear();
        for (auto &i: fallbackPaths)
        {
            const std::string _path = i;
            if (_path.empty())
                RUNTIME_ERROR("Null path format");

            m_PathFmts.emplace_back(std::filesystem::absolute(_path).string());
        }
        m_FileSizeLimit = fsize_in_bytes;
        m_CurPathFmt = 0;
    }
    void enableAutoMkDir(bool yes = true);
    void setBinaryMode(bool enabled);

    // Implement I_SnapT<std::ostream*>
    std::ostream *snap() override;

private:

    // Types
    typedef std::chrono::system_clock C_MyClock;

    // Data
    std::ofstream               m_Out;              // prior to m_Lock
    std::string                 m_CurrPath;         // Path of the currently openned file
    std::vector<std::string>    m_PathFmts;
    uintmax_t                   m_FileSizeLimit{};  // in bytes
    size_t                      m_CurPathFmt{};
    C_MyClock::time_point       m_OldTime;
    std::ios_base::openmode     m_OpenMode{std::ios_base::out};
    bool                        m_AutoMkDir{true};
};

} // namespace bux
