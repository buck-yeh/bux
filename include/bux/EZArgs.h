#pragma once

#include "XException.h" // RUNTIME_ERROR()
#include <algorithm>    // std::sort()
#include <deque>        // std::deque<>
#include <functional>   // std::function<>
#include <map>          // std::map<>
#include <string_view>  // std::string_view
#include <variant>      // std::variant<>, std::monostate
#include <vector>       // std::vector<>

namespace bux {

//
//      Types
//
struct C_ErrorOrIndex
{
    // Data
    std::string         m_error;
    size_t              m_index;

    // Nonvirtuals
    C_ErrorOrIndex(const std::string &error, size_t flagErrInd): m_error(error), m_index(flagErrInd) {}
    C_ErrorOrIndex(size_t flagStartInd): m_index(flagStartInd) {}
    operator bool() const { return m_error.empty(); }
};

class C_EZArgs
/*  Argument parser with flag rules:
    1) Flags (-x, --xxx) always come after actions and positional arguments.
    2) Flag with value may be given as either "--flag=value" or "--flag value"
    3) Adding a flag with trigger & parse at the same time means the flag can be given either with or without value.
*/
{
public:

    // Nonvirtuals
    explicit C_EZArgs(std::string_view description = {}): m_desc(description) {}
    C_EZArgs &add_flag(std::string_view name, char short_name,
                       std::string_view description,
                       const std::function<void()> &trigger,
                       const std::function<void(std::string_view )> &parse ={});
    C_EZArgs &add_flag(std::string_view name,
                       std::string_view description,
                       const std::function<void()> &trigger,
                       const std::function<void(std::string_view )> &parse ={}){
        return add_flag(name, char(), description, trigger, parse);
    }
    C_EZArgs &add_flag(std::string_view name,
                       std::string_view description,
                       const std::function<void(std::string_view )> &parse){
        return add_flag(name, char(), description, {}, parse);
    }
    C_EZArgs &add_flag(std::string_view name, char short_name,
                       std::string_view description,
                       const std::function<void(std::string_view )> &parse){
        return add_flag(name, short_name, description, {}, parse);
    }
    C_EZArgs &add_flag(char short_name,
                       std::string_view description,
                       const std::function<void()> &trigger,
                       const std::function<void(std::string_view )> &parse ={}){
        return add_flag({}, short_name, description, trigger, parse);
    }
    C_EZArgs &add_flag(char short_name,
                       std::string_view description,
                       const std::function<void(std::string_view )> &parse){
        return add_flag({}, short_name, description, {}, parse);
    }
    C_EZArgs &add_subcommand(std::string_view name, const std::function<void()> &onParsed = {},
        std::string_view description = {});
    void details(std::string_view s) { m_details = s; }
    [[nodiscard]]C_ErrorOrIndex parse(size_t argc, const char *argv[]) const;
    [[nodiscard]]auto parse(int argc, const char *argv[]) const { return parse(size_t(argc), argv); }
    template<class S, class N = size_t>
    C_EZArgs &position_args(std::initializer_list<S> arg_names, std::initializer_list<N> count_optionals = {}, bool unlimited = false);
    template<class C1, class C2 = std::initializer_list<size_t>>
    C_EZArgs &position_args(const C1 &arg_names, const C2 &count_optionals = {}, bool unlimited = false);
    auto parsed_position_argc() const { return m_totoalPositionArgs; }

private:

    // Types
    struct C_FlagDef
    {
        std::string             m_name, m_descOneLiner;
        char                    m_shortName{};
        std::function<void()>   m_trigger;
        std::function<void(std::string_view )> m_parse;
    };

    struct C_ArgLayout
    {
        std::vector<std::string>    m_posArgs;
        std::vector<size_t>         m_posCounts;
        bool                        m_unlimited;
    };

    // Data
    const std::string       m_desc;
    std::string             m_details;
    std::deque<C_FlagDef>   m_flags;
    std::variant<std::monostate,C_ArgLayout,std::map<std::string,C_EZArgs>> m_up2u;
    C_EZArgs                *m_owner{};
    std::function<void()>   m_onParsed;
    size_t                  mutable m_totoalPositionArgs{};
    bool                    m_helpShielded{false};
    bool                    m_hShielded{false};

    // Nonvirtuals
    std::string help_flags() const;
    std::string help_full(const char *argv[]) const;
    std::string help_tip(const std::string &error, const char *argv[]) const;
};

//
//      Implememt Class Method Templates
//
template<class S, class N>
C_EZArgs &C_EZArgs::position_args(std::initializer_list<S> arg_names, std::initializer_list<N> count_optionals, bool unlimited)
{
    return position_args<std::initializer_list<S>, std::initializer_list<N>>(arg_names, count_optionals, unlimited);
}

template<class C1, class C2>
C_EZArgs &C_EZArgs::position_args(const C1 &arg_names, const C2 &count_optionals, bool unlimited)
{
    if (!std::empty(arg_names) || unlimited)
        // Non-trivial case
    {
        switch (m_up2u.index())
        {
        case 0: // turn case 1
            m_up2u.emplace<C_ArgLayout>();
            break;
        case 1:
            break;
        case 2:
            RUNTIME_ERROR("Already added subcommands");
        }
        auto &dst = std::get<1>(m_up2u);
        dst.m_posArgs.assign(std::begin(arg_names), std::end(arg_names));
        dst.m_unlimited = unlimited;

        // Assign & adjust m_posCounts
        dst.m_posCounts.assign(std::begin(count_optionals), std::end(count_optionals));
        sort(dst.m_posCounts.begin(), dst.m_posCounts.end());
        const auto n_args = dst.m_posArgs.size();
        while (!dst.m_posCounts.empty() && dst.m_posCounts.back() >= n_args)
            dst.m_posCounts.pop_back();

        dst.m_posCounts.emplace_back(n_args);
    }
    return *this;
}

} //namespace bux
