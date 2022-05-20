#pragma once

#include "XException.h" // RUNTIME_ERROR()
#include <algorithm>    // std::sort()
#include <concepts>     // std::integral<>, std::invocable<>
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
/*! -# \c C_EZArgs the argument parser can define either subcommands or positional arguments, but not both.
    -# Every subcommand is again an argument parser, and hence follows rule 1.
    -# Rules about flags:
       -# Flags (-x, --xxx) always come \b after all subcommands and positional arguments. <em>(This one may be loosen in the future)</em>
       -# Flag with value may be given as either <tt>--flag=value</tt> or <tt>--flag value</tt>
       -# Adding a flag with both \c trigger and \c parse callbacks means the flag has an optional value.
    -# The 9 <tt>add_flag()</tt> overloaded methods facilitate defining new flags without any \a optional arguments:
       -# It is ok to call without either \c name or \c short_name, but not both;
       -# It is ok to call without either \c trigger or \c parse, but not both;
       -# Always call with \c description
*/
{
public:

    // Ctors
    explicit C_EZArgs(const std::string &description = {}): m_desc(description) {}
    C_EZArgs(const C_EZArgs&) = delete;
    C_EZArgs &operator=(const C_EZArgs&) = delete;

    // Nonvirtuals - for syntax & help layout
    C_EZArgs &add_flag(std::string_view name, char short_name,
                       std::string_view description,
                       std::invocable<> auto &&trigger,
                       std::invocable<std::string_view> auto &&parse){
        auto &def = create_flag_def(name, short_name, description);
        def.m_trigger  = std::move(trigger);
        def.m_parse    = std::move(parse);
        return *this;
    }
    C_EZArgs &add_flag(std::string_view name, char short_name,
                       std::string_view description,
                       std::invocable<> auto &&trigger){
        create_flag_def(name, short_name, description).m_trigger = std::move(trigger);
        return *this;
    }
    C_EZArgs &add_flag(std::string_view name, char short_name,
                       std::string_view description,
                       std::invocable<std::string_view> auto &&parse){
        create_flag_def(name, short_name, description).m_parse = std::move(parse);
        return *this;
    }
    C_EZArgs &add_flag(std::string_view name,
                       std::string_view description,
                       std::invocable<> auto trigger,
                       std::invocable<std::string_view> auto &&parse) {
        auto &def = create_flag_def(name, char(), description);
        def.m_trigger  = std::move(trigger);
        def.m_parse    = std::move(parse);
        return *this;
    }
    C_EZArgs &add_flag(std::string_view name,
                       std::string_view description,
                       std::invocable<> auto &&trigger) {
        create_flag_def(name, char(), description).m_trigger = std::move(trigger);
        return *this;
    }
    C_EZArgs &add_flag(std::string_view name,
                       std::string_view description,
                       std::invocable<std::string_view> auto &&parse){
        create_flag_def(name, char(), description).m_parse = std::move(parse);
        return *this;
    }
    C_EZArgs &add_flag(char short_name,
                       std::string_view description,
                       std::invocable<> auto &&trigger,
                       std::invocable<std::string_view> auto &&parse) {
        auto &def = create_flag_def({}, short_name, description);
        def.m_trigger  = std::move(trigger);
        def.m_parse    = std::move(parse);
        return *this;
    }
    C_EZArgs &add_flag(char short_name,
                       std::string_view description,
                       std::invocable<> auto &&trigger) {
        create_flag_def({}, short_name, description).m_trigger = std::move(trigger);
        return *this;
    }
    C_EZArgs &add_flag(char short_name,
                       std::string_view description,
                       std::invocable<std::string_view> auto &&parse) {
        create_flag_def({}, short_name, description).m_parse = std::move(parse);
        return *this;
    }
    //
    C_EZArgs &add_subcommand(const std::string &name, std::invocable<> auto onParsed, const std::string &description = {});
    void details(std::string_view s) { m_details = s; }
    //
    C_EZArgs &position_args(const std::ranges::forward_range auto &arg_names,
        const std::ranges::forward_range auto &count_optionals, bool unlimited = false);
    C_EZArgs &position_args(const std::ranges::forward_range auto &arg_names, bool unlimited = false)
        { return position_args(arg_names, std::ranges::views::empty<size_t>, unlimited); }

    // Nonvirtuals - for parsing
    [[nodiscard]]C_ErrorOrIndex parse(std::integral auto argc, const char *const argv[]) const;
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

    using C_UP2U = std::variant<std::monostate,C_ArgLayout,std::map<std::string,C_EZArgs>>;
    enum
    {
        UP2U_NULL,
        UP2U_LAYOUT,
        UP2U_SUBCMD
    };

    // Data
    const std::string       m_desc;
    std::string             m_details;
    std::deque<C_FlagDef>   m_flags;
    C_UP2U                  m_up2u;
    C_EZArgs                *m_owner{};
    std::function<void()>   m_onParsed;
    size_t                  mutable m_totoalPositionArgs{};
    bool                    m_helpShielded{false};
    bool                    m_hShielded{false};

    // Compile-time assertions
    static_assert(std::variant_size_v<C_UP2U> == 3);
    static_assert(std::is_same_v<std::monostate,                    std::variant_alternative_t<UP2U_NULL,   C_UP2U>>);
    static_assert(std::is_same_v<C_ArgLayout,                       std::variant_alternative_t<UP2U_LAYOUT, C_UP2U>>);
    static_assert(std::is_same_v<std::map<std::string,C_EZArgs>,    std::variant_alternative_t<UP2U_SUBCMD, C_UP2U>>);

    // Nonvirtuals
    C_FlagDef &create_flag_def(std::string_view name, char short_name, std::string_view description);
    const C_FlagDef *find_shortname_def(char sname) const;
    const C_FlagDef* find_longname_def(std::string_view name) const;
    bool is_valid_flag(const char* arg) const;
    std::string help_flags() const;
    C_ErrorOrIndex help_full(const char *const argv[]) const;
    std::string help_tip(const std::string &error, const char *const argv[]) const;
    std::string retro_path(const char *const argv[]) const;
};

//
//      Implement Member Templates
//
C_EZArgs &C_EZArgs::add_subcommand(const std::string &name, std::invocable<> auto onParsed, const std::string &description)
/*! \param [in] name The verb
    \param [in] onParsed Called when the flag is fiven without value
    \param [in] description Decribe the subcommand
    \exception std::runtime_error if <tt>position_args()</tt> has been called.
    \return The newly constructed subcommand as a \c C_EZArgs instance
*/
{
    switch (m_up2u.index())
    {
    case UP2U_NULL:
        m_up2u.emplace<UP2U_SUBCMD>(); // become UP2U_SUBCMD
        break;
    case UP2U_SUBCMD:
        break;
    case UP2U_LAYOUT:
        RUNTIME_ERROR("Already set as positional arguments");
    }
    auto &ret = std::get<UP2U_SUBCMD>(m_up2u).try_emplace(name, description).first->second;
    ret.m_helpShielded  = m_helpShielded;
    ret.m_hShielded     = m_hShielded;
    ret.m_owner         = this;
    ret.m_onParsed      = onParsed;
    return ret;
}

C_EZArgs &C_EZArgs::position_args           (
    const std::ranges::forward_range auto   &arg_names,
    const std::ranges::forward_range auto   &count_optionals,
    bool                                    unlimited       )
/*! \param [in] arg_names Argument display names and the implied maximal count of positional arguments when \c unlimited is \b false.
    \param [in] count_optionals Valid argument counts, automatically extended with all intergers greater than <tt>max(count_optionals)</tt>
    when \c unlimited is \b true.
    \param [in] unlimited If count of positional arguments grater than <tt>max(count_optionals)</tt> is valid.
    \exception std::runtime_error if <tt>add_subcommand()</tt> has been called.
    \return <tt>*this</tt>

    Together with another overload, only \c arg_names is mandatory. When only \c arg_names is provided, exactly <tt>std::size(arg_names)</tt>
    postional arguments are expected.
*/
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
/*! \param [in] argc Typically the same \c argc from <tt>main(int argc, char *argv[])</tt>
    \param [in] argv Typically the same \c argv from <tt>main(int argc, char *argv[])</tt>
    \return On success, the returned value can be implicitly cast to \b true; otherwise, call <tt>message()</tt> method
            to get help or error message.
    \post Call parsed_position_argc() to get number of positional arguments including <tt>argv[0]</tt> and subcommand(s)
*/
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
            if (auto def = find_longname_def(flag_name))
            {
                if (eqsign)
                    // (=)-connected flag value
                {
                    if (def->m_parse)
                    {
                        def->m_parse(eqsign+1);
                        goto NextInd;
                    }
                    return {help_tip(std::string("Value parser absent: ")+arg, argv), ind};
                }
                else if (ind+1 < argc && !is_valid_flag(argv[ind+1]))
                    // Flag with value
                {
                    if (def->m_parse)
                    {
                        def->m_parse(argv[++ind]);
                        goto NextInd;
                    }
                    return {help_tip(std::string("Value parser absent: ")+arg, argv), ind};
                }
                else
                    // Flag without value
                {
                    if (def->m_trigger)
                    {
                        def->m_trigger();
                        goto NextInd;
                    }
                    return {help_tip(std::string(def->m_parse?"Missing flag value: ":"Triggerless flag: ")+arg, argv), ind};
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
                if (auto def = find_shortname_def(sname))
                {
                    if (!arg[1] && ind+1 < argc && !is_valid_flag(argv[ind+1]))
                        // Flag with value
                    {
                        if (def->m_parse)
                        {
                            def->m_parse(argv[++ind]);
                            goto NextInd;
                        }
                        return {help_tip(std::string("Value parser absent: -")+sname, argv), ind};
                    }
                    else
                        // Flag without value
                    {
                        if (def->m_trigger)
                        {
                            def->m_trigger();
                            goto NextInd;
                        }
                        return {help_tip(std::string(def->m_parse?"Missing flag value: -":"Triggerless flag: -")+sname, argv), ind};
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
