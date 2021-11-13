#include "EZArgs.h"
#include <cstring>      // strchr()
#include <filesystem>   // std::filesystem::path

namespace bux {

//
//      Implement Classes
//
std::string C_ErrorOrIndex::message() const
{
    return m_optIndex? fmt::format("argv[{}]: {}",*m_optIndex,m_message): m_message;
}

C_EZArgs &C_EZArgs::add_flag(std::string_view name, char short_name, std::string_view description,
    std::function<void()> trigger, std::function<void(std::string_view)> parse)
/*! \param [in] name Long flag name, with or without -- prefix
    \param [in] short_name Short flag name as a single letter
    \param [in] description Decribe the flag
    \param [in] trigger Called when the flag is fiven without value
    \param [in] parse Called when the flag is fiven with a value
    \exception std::runtime_error if both \c trigger and \c parse is null or \c name is prefixed with a single '-' letter
    \return <tt>*this</tt>

    Together with other 5 overload methods, <tt>add_flag()</tt> can be called conveniently
    1. without either \c name or \c short_name, but not both;
    2. without either \c trigger or \c parse, but not both;
    3. always with the only mandatory \c description
*/
{
    if (!trigger && !parse)
        RUNTIME_ERROR("Either trigger or parse handler must be provided");

    auto &dst = m_flags.emplace_back();
    if (!name.empty() && name[0] == '-')
        if (name[1] == '-')
            dst.m_name = name.substr(2);
        else
        {
            m_flags.pop_back();
            RUNTIME_ERROR("Invalid flag name: {}", name);
        }
    else
        dst.m_name = name;

    dst.m_descOneLiner  = description;
    dst.m_shortName     = short_name;
    dst.m_trigger       = trigger;
    dst.m_parse         = parse;
    if (dst.m_name == "help")
        m_helpShielded = true;
    if (dst.m_shortName == 'h')
        m_hShielded = true;

    return *this;
}

C_EZArgs &C_EZArgs::add_subcommand(const std::string &name, std::function<void()> onParsed, const std::string &description)
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

std::string C_EZArgs::retro_path(const char *const argv[]) const
{
    std::vector<const char*> polish_args;
    for (auto cur = this; cur; cur = cur->m_owner)
        polish_args.emplace_back(*(argv--));

    std::string ret;
    for (auto i = polish_args.crbegin(); i != polish_args.crend(); ++i)
    {
        if (ret.empty())
            ret.assign(
#ifdef _WIN32
                ".\\"
#else
                "./"
#endif
               ) += std::filesystem::path{*i}.filename();
        else
        {
            ret += ' ';
            ret += *i;
        }
    }
    return ret;
}

C_ErrorOrIndex C_EZArgs::help_full(const char *const argv[]) const
{
    // Synthesize USAGE
    std::string help = "USAGE: " + retro_path(argv);
    std::string validActions;
    switch (m_up2u.index())
    {
    case UP2U_NULL:
        break;
    case UP2U_LAYOUT:
        {
            auto &lo = std::get<UP2U_LAYOUT>(m_up2u);
            const auto minPosArgs = lo.m_posCounts.empty()? lo.m_posArgs.size(): lo.m_posCounts.front();
            for (size_t i = 0; i < minPosArgs; ++i)
                ((help += " <") += lo.m_posArgs[i]) += '>';

            std::string optionals;
            if (lo.m_unlimited)
                optionals = "...";

            size_t ub = lo.m_posArgs.size();
            for (auto i: lo.m_posCounts)
            {
                std::string t;
                for (auto j = i; j < ub; ++j)
                    t += (t.empty()? "[<": "> <") + lo.m_posArgs[j];

                if (!t.empty())
                {
                    if (optionals.empty())
                        optionals = t + ">]";
                    else
                        optionals = t + "> " + optionals + ']';
                }
                ub = i;
            }
            if (!optionals.empty())
                (help += ' ') += optionals;
        }
        break;
    case UP2U_SUBCMD:
        validActions += "VALID ACTIONS:\n";
        {
            auto &subcmds = std::get<UP2U_SUBCMD>(m_up2u);
            std::string actions;
            size_t positionalCount{};
            for (auto &i: subcmds)
            {
                actions += actions.empty()? '(': '|';
                actions += i.first;
                ((validActions += "  ") += i.first) += '\n';
                if (!i.second.m_desc.empty())
                    ((validActions += '\t') += i.second.m_desc) += '\n';
                if (i.second.m_up2u.index() != UP2U_NULL)
                    ++positionalCount;
            }
            (help += ' ') += actions;
            if (!positionalCount)
                help += ')';
            else if (positionalCount < subcmds.size())
                help += ") [...]";
            else
                help += ") ...";
        }
    }

    for (auto cur = this; cur; cur = cur->m_owner)
        for (auto &def: cur->m_flags)
        {
            help += " [-";
            if (def.m_shortName)
                help += def.m_shortName;
            else
                (help += '-') += def.m_name;

            if (def.m_parse)
                help += def.m_trigger? " [ARG]": " ARG";

            help += ']';
        }
    if (!m_hShielded)
        help += " [-h]";
    else if (!m_helpShielded)
        help += " [--help]";

    // Append DESCRIPTION if any
    if (!m_desc.empty())
    {
        help += "\n"
                "DESCRIPTION:\n"
                "  ";
        help += m_desc;
    }
    if (help.back() != '\n')
        help += '\n';

    // '\n'-ended guaranteed
    if (!validActions.empty())
        help += validActions;
    else
    {
        // Append flag helps if any
        std::string flags = help_flags();
        const char *help_flag{};
        switch ((m_hShielded?0:2) + (m_helpShielded?0:1))
        {
        case 1:
            help_flag = "  --help\n";
            break;
        case 2:
            help_flag = "  -h\n";
            break;
        case 3:
            help_flag = "  -h, --help\n";
            break;
        }
        if (help_flag)
            flags.append(help_flag).append("\tDisplay this help and exit\n");
        if (!flags.empty())
            help += "\n"
                    "VALID FLAGS:\n" + flags;

        flags.clear();
        for (auto cur = m_owner; cur; cur = cur->m_owner)
            flags += cur->help_flags();
        if (!flags.empty())
            help += "\n"
                    "INHERITED FLAGS:\n" + flags;
    }

    // '\n'-ended guaranteed
    if (!m_details.empty())
    {
        help += "\n"
                "DETAILED:\n"
                "  ";
        help += m_details;
        if (m_details.back() != '\n')
            help += '\n';
    }

    // '\n'-ended guaranteed
    return help;
}

std::string C_EZArgs::help_flags() const
{
    std::string help;
    for (auto &def: m_flags)
    {
        help += "  -";
        if (def.m_shortName)
        {
            help += def.m_shortName;
            if (!def.m_name.empty())
                (help += ", --") += def.m_name;
        }
        else
            (help += '-') += def.m_name;

        if (def.m_parse)
            help += def.m_trigger? " [ARG]": " ARG";

        help += '\n';
        if (!def.m_descOneLiner.empty())
            (help += '\t').append(def.m_descOneLiner) += '\n';
    }
    return help;
}

std::string C_EZArgs::help_tip(const std::string &error, const char *const argv[]) const
{
    const char *helpFlag{};
    if (!m_hShielded)
        helpFlag = "-h";
    else if (!m_helpShielded)
        helpFlag = "--help";

    if (helpFlag)
        return fmt::format("{}\nType \"{} {}\" to read the help", error, retro_path(argv), helpFlag);

    return error;
}

} //namespace bux
