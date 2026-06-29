#pragma once

#include <functional>   // std::function<>
#include <stdexcept>    // std::exception
#include <string>       // std::string
#include <string_view>  // std::string_view
#include <typeinfo>     // return type of typeid()

namespace bux {

//
//      Externs
//
const char *ord_suffix(size_t i);
std::string expand_env(const char *s);

std::string _HRTN(const char *originalName);
std::string OXCPT(const std::exception &e);

template<typename T>
void split(std::basic_string_view<T> token_list, std::basic_string_view<T> splitters,
    const std::function<void(std::basic_string_view<T>)>& apply_token,
    const std::function<void(std::basic_string_view<T>)>& apply_delim = {})
{
    for (size_t start_off = 0; start_off < token_list.size();)
    {
        const auto first_not_of_off = token_list.find_first_not_of(splitters, start_off);
        if (first_not_of_off == std::string::npos)
            break;

        if (apply_delim && start_off < first_not_of_off)
            apply_delim(token_list.substr(start_off, first_not_of_off - start_off));

        start_off = first_not_of_off;
        const auto end_off = token_list.find_first_of(splitters, start_off);
        if (end_off == std::string::npos)
        {
            if (apply_token && start_off < token_list.size())
                apply_token(token_list.substr(start_off));

            break;
        }
        if (apply_token)
            apply_token(token_list.substr(start_off, end_off - start_off));

        start_off = end_off;
    }
}

} // namespace bux

/*! HRTN stands for Human Readable Type Name
*/
#define HRTN(t) bux::_HRTN(typeid(t).name())
using bux::OXCPT;
