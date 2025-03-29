#pragma once
#ifdef _WIN32
#include <windows.h>
#endif
#include <filesystem>

namespace bux {

class C_FileAsMemory
{
public:

    // Nonvirtuals
    C_FileAsMemory(const std::filesystem::path& path);
    ~C_FileAsMemory();
    const char* data() const { return m_data; }

private:

    // Data
    char*       m_data{};
#ifdef _WIN32
    HANDLE      m_handle{};
#else
    size_t      m_bytes{};
    int         m_fd = -1;
#endif
};

} // namespace bux
