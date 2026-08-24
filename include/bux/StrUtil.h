#pragma once

#include <concepts>     // std::regular_invocable<>
#include <functional>   // std::function<>
#include <stdexcept>    // std::exception
#include <string>       // std::string
#include <string_view>  // std::string_view
#include <type_traits>  // std::type_identity_t<>
#include <typeinfo>     // return type of typeid()
#include <utility>      // std::declval(), std::forward()

namespace bux {

//
//      Types
//
/*! Character type of any string-like type S, i.e. anything std::basic_string_view is constructible from.
*/
template<typename S>
using string_char_t = typename decltype(std::basic_string_view{std::declval<const S&>()})::value_type;

//
//      Externs
//
const char *ord_suffix(size_t i);
std::string expand_env(const char *s);

std::string _HRTN(const char *originalName);
std::string OXCPT(const std::exception &e);

/*! \brief Split \em token_list at any character in \em splitters, feeding each token to
    \em apply_token and each run of splitters between two tokens to \em apply_delim.

    The character type is deduced from \em token_list, which can be any string-like type,
    e.g. a string literal, a const T* or a std::basic_string<T>. \em splitters is then
    a non-deduced std::basic_string_view of that same character type.
*/
template<typename S, typename T = string_char_t<S>>
void split(const S& token_list, std::type_identity_t<std::basic_string_view<T>> splitters,
    std::regular_invocable<std::basic_string_view<T>> auto&& apply_token,
    std::regular_invocable<std::basic_string_view<T>> auto&& apply_delim)
{
    using view_t = std::basic_string_view<T>;
    const view_t src{token_list};
    for (size_t start_off = 0; start_off < src.size();)
    {
        const auto first_not_of_off = src.find_first_not_of(splitters, start_off);
        if (first_not_of_off == view_t::npos)
            break;

        if (start_off < first_not_of_off)
            apply_delim(src.substr(start_off, first_not_of_off - start_off));

        start_off = first_not_of_off;
        const auto end_off = src.find_first_of(splitters, start_off);
        if (end_off == view_t::npos)
        {
            if (start_off < src.size())
                apply_token(src.substr(start_off));

            break;
        }
        apply_token(src.substr(start_off, end_off - start_off));
        start_off = end_off;
    }
}

/*! \brief Same as the 4-argument overload, ignoring the delimiters between tokens.
*/
template<typename S, typename T = string_char_t<S>>
void split(const S& token_list, std::type_identity_t<std::basic_string_view<T>> splitters,
    std::regular_invocable<std::basic_string_view<T>> auto&& apply_token)
{
    split(token_list, splitters,
        std::forward<decltype(apply_token)>(apply_token), [](std::basic_string_view<T>){});
}

} // namespace bux

/*! HRTN stands for Human Readable Type Name
*/
#define HRTN(t) bux::_HRTN(typeid(t).name())
using bux::OXCPT;
