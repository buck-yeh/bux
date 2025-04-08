#include "EZArgs.h"
#include <cstring>      // strchr()
#include <filesystem>   // std::filesystem::path
#include <format>       // std::format()

namespace bux {

//
//      Implement Classes
//
std::string C_ErrorOrIndex::message() const
{
    return m_optIndex? std::format("argv[{}]: {}",*m_optIndex,m_message): m_message;
}

C_EZArgs::C_FlagDef &C_EZArgs::create_flag_def(std::string_view name, char short_name, std::string_view description)
/*! \param [in] name Long flag name, with or without -- prefix
    \param [in] short_name Short flag name as a single letter
    \param [in] description Decribe the flag
    \exception std::runtime_error if \c name is prefixed with a single '-' letter
    \return The created flag struct
*/
{
    auto &dst = m_flags.emplace_back();
    if (!name.empty() && name[0] == '-')
        if (name[1] == '-')
            dst.m_name = name.substr(2);
        else
        {
            m_flags.pop_back();
            throw std::runtime_error{"Invalid flag name: " + (std::string)name};
        }
    else
        dst.m_name = name;

    dst.m_descOneLiner  = description;
    dst.m_shortName     = short_name;
    if (dst.m_name == "help")
        m_helpShielded = true;
    if (dst.m_shortName == 'h')
        m_hShielded = true;

    return dst;
}

std::string C_EZArgs::retro_path(const char *const argv[]) const
{
    std::vector<const char*> polish_args;
    for (auto cur = this; cur; cur = cur->m_owner)
        polish_args.emplace_back(*(argv--));

    std::string ret;
    for (auto i = polish_args.rbegin(); i != polish_args.rend(); ++i)
    {
        if (ret.empty())
            ret = std::filesystem::path{*i}.filename().string();
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

    help += '\n';

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
        return std::format("{}\nType \"{} {}\" to read the help", error, retro_path(argv), helpFlag);

    return error;
}

const C_EZArgs::C_FlagDef* C_EZArgs::find_shortname_def(char sname) const
{
    for (auto cur = this; cur; cur = cur->m_owner)
        for (auto& def : cur->m_flags)
        {
            if (def.m_shortName == sname)
                // Matched
                return &def;
        }
    return nullptr;
}

const C_EZArgs::C_FlagDef* C_EZArgs::find_longname_def(std::string_view name) const
{
    for (auto cur = this; cur; cur = cur->m_owner)
        for (auto& def : cur->m_flags)
        {
            if (def.m_name == name)
                // Matched
                return &def;
        }
    return nullptr;
}

bool C_EZArgs::is_valid_flag(const char *const *argv_rest, int argc_rest) const
{
    const auto arg = *argv_rest;
    if (*arg != '-')
        return false;

    if (arg[1] == '-')
        // Match full flag name
    {
        const auto flag = arg + 2;
        const auto eqsign = strchr(flag, '=');
        const auto flag_name = eqsign ? std::string_view(flag, size_t(eqsign - flag)) : std::string_view(flag);
        return nullptr != find_longname_def(flag_name);
    }

    // Match every short flag
    for (auto p = arg; *++p;)
        if (auto const def = find_shortname_def(*p))
        {
            if (p[1])
            {
                if (!def->m_trigger)
                    return false;
            }
            else
            {
                if (def->m_trigger)
                    return true;

                // (def->m_parse != nullptr) implied
                if (argc_rest < 2 || is_valid_flag(argv_rest+1, argc_rest-1))
                    return false;
            }
        }
        else
            return false;

    return true;
}

} //namespace bux
