#pragma once

#include "SyncLog.h"    // bux::I_SnapT<>
#include <chrono>       // std::chrono::*
#include <filesystem>   // std::filesystem::*
#include <fstream>      // std::ofstream
#include <stdexcept>    // std::runtime_error
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
    explicit C_PathFmtLogSnap(T_LocalZone tz = local_zone());
#if LOCALZONE_IS_TIMEZONE
    explicit C_PathFmtLogSnap(bool use_local_time): C_PathFmtLogSnap(use_local_time? local_zone(): T_LocalZone()) {}
#endif
    C_PathFmtLogSnap &configPath(const std::string &pathFmt);
    C_PathFmtLogSnap &configPath(uintmax_t fsize_in_bytes, const auto &fallbackPaths)
    /*! \param [in] fsize_in_bytes Max bytes per log file.
        \param [in] fallbackPaths Array of strings used in turn as `fmt` parameter of `std::vformat()` with one single extra argument to format, ie. timestamp for now.
        \return `*this`

        Each formatted path is the target file for the moment when something is to be logged on demand.
    */
        requires requires {
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
                throw std::runtime_error{"Null path format"};

            m_PathFmts.emplace_back(std::filesystem::absolute(_path).string());
        }
        m_FileSizeLimit = fsize_in_bytes;
        m_CurPathFmt = 0;
        return *this;
    }
    C_PathFmtLogSnap &enableAutoMkDir(bool yes = true);
    C_PathFmtLogSnap &setBinaryMode(bool enabled);

    // Implement I_SnapT<std::ostream*>
    std::ostream *snap() override;

private:

    // Data
    const T_LocalZone           m_tz;
    std::ofstream               m_Out;              // prior to m_Lock
    std::string                 m_CurrPath;         // Path of the currently openned file
    std::vector<std::string>    m_PathFmts;
    uintmax_t                   m_FileSizeLimit{};  // in bytes
    size_t                      m_CurPathFmt{};
    std::chrono::sys_seconds    m_OldTime;
    std::ios_base::openmode     m_OpenMode{std::ios_base::out};
    bool                        m_AutoMkDir{true};
};

} // namespace bux
