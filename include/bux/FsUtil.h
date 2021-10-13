#pragma once

#include <filesystem>   // std::filesystem::path
#include <ranges>       // std::ranges::forward_range<>

namespace bux {

//
//      Function Templates
//
std::filesystem::path search_dirs(const std::filesystem::path &in_path, const std::ranges::forward_range auto &dirs)
{
    if (!std::filesystem::exists(in_path) && !in_path.has_root_path())
        for (auto &i: dirs)
        {
            const auto concat = i / in_path;
            if (std::filesystem::exists(concat))
                return concat;
        }
    return in_path;
}

} // namespace bux
