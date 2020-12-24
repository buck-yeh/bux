#ifndef FileLogH
#define FileLogH

#include "SyncStream.h" // bux::C_SyncOstream
#include <chrono>       // std::chrono::system_clock
#include <fstream>      // std::ofstream
#include <string>       // std::string

namespace bux {

//
//      Types
//
class C_FileLog: public I_SyncOstream
/*! Thread-safe file log which can be configured to automatically change the
    output path according to the current timestamp. Apply bux::C_UseOstream
    to block any other thread WITHIN process from using it.
*/
{
public:

    // Nonvirtuals
    explicit C_FileLog(const std::string &pathFmt = "%Y%m%d.log");
    void configPath(const std::string &pathFmt);
    void enableAutoMkDir(bool yes = true);
    void setBinaryMode(bool enabled);

protected:

    // Implement I_SyncOstream
    std::ostream &getResource() override;
    void releaseResource(std::ostream &out) override;

private:

    // Types
    typedef std::chrono::system_clock C_MyClock;

    // Data
    std::ofstream           m_Out;      // prior to m_Lock
    C_SyncOstream           mutable m_Lock;
    std::string             m_CurrPath; // Path of the currently openned file
    std::string             m_PathFmt;
    C_MyClock::time_point   m_OldTime;
    std::ios_base::openmode m_OpenMode{std::ios_base::out};
    bool                    m_AutoMkDir{true};
};

} // namespace bux

#endif // FileLogH
