#pragma once

#include <concepts>     // std::convertible_to<>
#include <filesystem>   // std::filesystem::path

namespace bux {

//
//      Concepts
//
template<typename T>
concept ConstPathList = requires(T t){
     ++std::cbegin(t);
     { *std::cbegin(t) } -> std::convertible_to<const std::filesystem::path&>;
     std::cend(t);
};

//
//      Function Templates
//
std::filesystem::path search_dirs(const std::filesystem::path &in_path, const ConstPathList auto &dirs)
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
