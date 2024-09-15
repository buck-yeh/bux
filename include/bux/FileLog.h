#pragma once

#include "SyncLog.h"    // bux::I_SnapT<>
#include "XException.h" // RUNTIME_ERROR()
#include <chrono>       // std::chrono::*
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
    explicit C_PathFmtLogSnap(const std::chrono::time_zone *tz);
    explicit C_PathFmtLogSnap(bool use_local_time = true): C_PathFmtLogSnap(use_local_time? std::chrono::get_tzdb().current_zone(): nullptr) {}
    C_PathFmtLogSnap &configPath(const std::string &pathFmt);
    C_PathFmtLogSnap &configPath(uintmax_t fsize_in_bytes, const auto &fallbackPaths) requires
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
        return *this;
    }
    C_PathFmtLogSnap &enableAutoMkDir(bool yes = true);
    C_PathFmtLogSnap &setBinaryMode(bool enabled);

    // Implement I_SnapT<std::ostream*>
    std::ostream *snap() override;

private:

    // Data
    const std::chrono::time_zone    *const m_tz;
    std::ofstream                   m_Out;              // prior to m_Lock
    std::string                     m_CurrPath;         // Path of the currently openned file
    std::vector<std::string>        m_PathFmts;
    uintmax_t                       m_FileSizeLimit{};  // in bytes
    size_t                          m_CurPathFmt{};
    std::chrono::sys_seconds        m_OldTime;
    std::ios_base::openmode         m_OpenMode{std::ios_base::out};
    bool                            m_AutoMkDir{true};
};

} // namespace bux
