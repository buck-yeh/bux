#include "FileAsMem.h"
#ifndef _WIN32
#include <sys/mman.h>           // mmap(), munmap()
#include <fcntl.h>              // open()
#include <unistd.h>             // lseek()
#endif
#include <stdexcept>            // std::runtime_error

namespace bux {

C_FileAsMemory::C_FileAsMemory(const std::filesystem::path& path)
{
#ifdef _WIN32
    const auto hFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
        throw std::runtime_error{"Failed to open \" + path.string() + \" for read-only"};

    m_handle = CreateFileMappingW(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
    CloseHandle(hFile);
    if (!m_handle)
        throw std::runtime_error{"Failed to create file mapping"};

    m_data = static_cast<char*>(MapViewOfFile(m_handle, FILE_MAP_READ, 0, 0, 0));
    if (!m_data)
        CloseHandle(m_handle);
#else
    m_fd = open(path.c_str(), O_RDONLY);
    if (m_fd == -1)
        throw std::runtime_error{"Failed to open \"" + path.string() + "\" for read-only"};

    m_bytes = (size_t)lseek(m_fd, 0, SEEK_END);
    auto data = mmap(nullptr, m_bytes, PROT_READ, MAP_PRIVATE, m_fd, 0);
    if (data != MAP_FAILED)
        m_data = static_cast<char*>(data);
    else
        close(m_fd);
#endif
}

C_FileAsMemory::~C_FileAsMemory()
{
    if (m_data)
    {
#ifdef _WIN32
        UnmapViewOfFile(m_data);
        CloseHandle(m_handle);
#else
        munmap(m_data, m_bytes);
        close(m_fd);
#endif
    }
}

} // namespace bux
