#pragma once

#include "XException.h" // RUNTIME_ERROR()
#include <algorithm>    // std::sort()
#include <deque>        // std::deque<>
#include <functional>   // std::function<>
#include <map>          // std::map<>
#include <optional>     // std::optional<>
#include <ranges>       // std::ranges::forward_range<>, std::ranges::views::empty<>
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
    std::string             m_message;
    std::optional<size_t>   m_optIndex;

    // Nonvirtuals
    C_ErrorOrIndex(const std::string &error, auto flagErrInd): m_message(error), m_optIndex(flagErrInd) {}
    C_ErrorOrIndex(const std::string &help): m_message(help) {}
    C_ErrorOrIndex(auto flagStartInd): m_optIndex(flagStartInd) {}
    operator bool() const { return m_message.empty(); }
    auto index() const { return m_optIndex.value(); }
    std::string message() const;
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
    explicit C_EZArgs(const std::string &description = {}): m_desc(description) {}
    C_EZArgs &add_flag(std::string_view name, char short_name,
                       std::string_view description,
                       std::function<void()> trigger,
                       std::function<void(std::string_view)> parse = {});
    C_EZArgs &add_flag(std::string_view name, char short_name,
                       std::string_view description,
                       std::function<void(std::string_view)> parse){
        return add_flag(name, short_name, description, {}, parse);
    }
    C_EZArgs &add_flag(std::string_view name,
                       std::string_view description,
                       std::function<void()> trigger,
                       std::function<void(std::string_view)> parse = {}) {
        return add_flag(name, char(), description, trigger, parse);
    }
    C_EZArgs &add_flag(std::string_view name,
                       std::string_view description,
                       std::function<void(std::string_view)> parse){
        return add_flag(name, char(), description, {}, parse);
    }
    C_EZArgs &add_flag(char short_name,
                       std::string_view description,
                       std::function<void()> trigger,
                       std::function<void(std::string_view)> parse = {}) {
        return add_flag({}, short_name, description, trigger, parse);
    }
    C_EZArgs &add_flag(char short_name,
                       std::string_view description,
                       std::function<void(std::string_view)> parse) {
        return add_flag({}, short_name, description, {}, parse);
    }
    C_EZArgs &add_subcommand(const std::string &name, std::function<void()> onParsed = {}, const std::string &description = {});
    void details(std::string_view s) { m_details = s; }
    [[nodiscard]]C_ErrorOrIndex parse(std::integral auto argc, const char *const argv[]) const;

    C_EZArgs &position_args(const std::ranges::forward_range auto &arg_names,
        const std::ranges::forward_range auto &count_optionals, bool unlimited = false);
    C_EZArgs &position_args(const std::ranges::forward_range auto &arg_names, bool unlimited = false)
        { return position_args(arg_names, std::ranges::views::empty<size_t>, unlimited); }
    auto parsed_position_argc() const { return m_totoalPositionArgs; }

private:

    // Types
    struct C_FlagDef
    {
        std::string             m_name, m_descOneLiner;
        char                    m_shortName{};
        std::function<void()>   m_trigger;
        std::function<void(std::string_view)> m_parse;
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
    C_ErrorOrIndex help_full(const char *const argv[]) const;
    std::string help_tip(const std::string &error, const char *const argv[]) const;
};

//
//      Implement Member Templates
//
C_EZArgs &C_EZArgs::position_args(const std::ranges::forward_range auto &arg_names, const std::ranges::forward_range auto &count_optionals, bool unlimited)
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

C_ErrorOrIndex C_EZArgs::parse(std::integral auto argc, const char *const argv[]) const
{
    decltype(argc) ind = 1;
    switch (m_up2u.index())
    {
    case 0: // trivial case
        break;
    case 1: // with positional arguments
        for (; ind < argc && argv[ind][0] != '-'; ++ind);
        m_totoalPositionArgs = ind;
        if (ind < argc && (argv[ind][1] == 'h' || !strcmp(argv[ind], "--help")))
            return help_full(argv);
        else
        {
            auto &lo = std::get<1>(m_up2u);
            decltype(argc) lower_bound = 0;
            for (auto i: lo.m_posCounts)
            {
                if (lower_bound < ind && ind <= static_cast<decltype(argc)>(i))
                {
                    std::string message;
                    if (!lower_bound)
                    {
                        const bool plural = i > 1;
                        message = "Positional argument";
                        if (plural)
                            message += 's';
                        for (size_t j = 0; j < i; ++j)
                            message.append(" <").append(lo.m_posArgs[j]) += '>';

                        message += plural? " are required":  " is required";
                    }
                    else
                        message = std::to_string(ind-1).append(ind > 2? " positonal arguments are":  " positonal argument is").append(" not allowed");

                    return {help_tip(message,argv), ind};
                }
                lower_bound = static_cast<decltype(argc)>(i + 1);
            }
            if (!lo.m_unlimited && ind > static_cast<decltype(argc)>(lo.m_posArgs.size() + 1))
            {
                std::string message = "Too many positional arguments:";
                for (auto &i: lo.m_posArgs)
                    message.append(" <").append(i) += '>';
                return {help_tip(message,argv), ind};
            }
        }
        break;
    case 2: // into subcommands
        if (ind >= argc || !strcmp(argv[ind],"-h") || !strcmp(argv[ind],"--help"))
            return help_full(argv);
        else
        {
            auto &map = std::get<2>(m_up2u);
            auto i = map.find(argv[ind]);
            if (i == map.end())
                return {help_tip(std::string("Unknown subcommand: ")+argv[ind], argv), ind};

            auto ret = i->second.parse(argc-ind, argv+ind);
            if (ret)
            {
                m_totoalPositionArgs = i->second.m_totoalPositionArgs + ind;
                return ret.m_optIndex.value() + ind;
            }
            return ret;
        }
    }
    const auto flagStartInd = ind;

    // Parse un-ordered flags
    for (; ind < argc; ++ind)
    {
        auto arg = argv[ind];
        if (arg[0] != '-')
            // Not flag initial
            return {help_tip(std::string("Unexpected argument: ")+arg, argv), ind};

        if (arg[1] == '-')
            // Match full flag name
        {
            const auto flag = arg + 2;
            const auto eqsign = strchr(flag, '=');
            const auto flag_name = eqsign? std::string_view(flag, size_t(eqsign-flag)): std::string_view(flag);
            for (auto cur = this; cur; cur = cur->m_owner)
                for (auto &def: cur->m_flags)
                    if (def.m_name == flag_name)
                        // Matched
                    {
                        if (eqsign)
                            // (=)-connected flag value
                        {
                            if (def.m_parse)
                            {
                                def.m_parse(eqsign+1);
                                goto NextInd;
                            }
                            return {help_tip(std::string("Value parser absent: ")+arg, argv), ind};
                        }
                        else if (ind+1 < argc && argv[ind+1][0] != '-')
                            // Flag with value
                        {
                            if (def.m_parse)
                            {
                                def.m_parse(argv[++ind]);
                                goto NextInd;
                            }
                            return {help_tip(std::string("Value parser absent: ")+arg, argv), ind};
                        }
                        else
                            // Flag without value
                        {
                            if (def.m_trigger)
                            {
                                def.m_trigger();
                                goto NextInd;
                            }
                            return {help_tip(std::string(def.m_parse?"Missing flag value: ":"Triggerless flag: ")+arg, argv), ind};
                        }
                    }
            if ("help" == flag_name)
                return help_full(argv);
            else
                return {help_tip(std::string("Unknown flag: --")+=flag_name, argv), ind};
        } // if (arg[1] == '-')
        else
            // Match continous short flag names
            while (char sname = *++arg)
            {
                for (auto cur = this; cur; cur = cur->m_owner)
                    for (auto &def: cur->m_flags)
                        if (def.m_shortName == sname)
                        {
                            if (!arg[1] && ind+1 < argc && argv[ind+1][0] != '-')
                                // Flag with value
                            {
                                if (def.m_parse)
                                {
                                    def.m_parse(argv[++ind]);
                                    goto NextInd;
                                }
                                return {help_tip(std::string("Value parser absent: -")+sname, argv), ind};
                            }
                            else
                                // Flag without value
                            {
                                if (def.m_trigger)
                                {
                                    def.m_trigger();
                                    goto NextInd;
                                }
                                return {help_tip(std::string(def.m_parse?"Missing flag value: -":"Triggerless flag: -")+sname, argv), ind};
                            }
                        }
                if ('h' == sname)
                    return help_full(argv);
                else
                    return {help_tip(std::string("Unknown flag: -")+=sname, argv), ind};
            } // while (char sname = *++arg)
    NextInd:;
    } // for (; ind < argc; ++ind)

    if (m_onParsed)
        m_onParsed();

    return flagStartInd;
}

} //namespace bux
