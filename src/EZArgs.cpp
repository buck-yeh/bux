#include "EZArgs.h"
#include <cstring>      // strchr()

namespace bux {

//
//      Implement Classes
//
C_EZArgs &C_EZArgs::add_flag(
    std::string_view        name,
    char                    short_name,
    std::string_view        description,
    const std::function<void()> &trigger,
    const std::function<void(std::string_view)> &parse)
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

C_EZArgs &C_EZArgs::add_subcommand(std::string_view name, const std::function<void()> &onParsed, std::string_view description)
{
    switch (m_up2u.index())
    {
    case 0:
        m_up2u.emplace<std::map<std::string,C_EZArgs>>();
        break;
    case 2:
        break;
    case 1:
        RUNTIME_ERROR("Already set as positional arguments");
    }
    auto &ret = std::get<2>(m_up2u).try_emplace(std::string(name), C_EZArgs(description)).first->second;
    ret.m_helpShielded  = m_helpShielded;
    ret.m_hShielded     = m_hShielded;
    ret.m_owner         = this;
    ret.m_onParsed      = onParsed;
    return ret;
}

C_ErrorOrIndex C_EZArgs::parse(size_t argc, const char *argv[]) const
{
    size_t ind = 1;
    switch (m_up2u.index())
    {
    case 0: // trivial case
        break;
    case 1: // with positional arguments
        for (; ind < argc && argv[ind][0] != '-'; ++ind);
        m_totoalPositionArgs = ind;
        if (ind < argc && (argv[ind][1] == 'h' || !strcmp(argv[ind], "--help")))
            return {help_full(argv), ind};
        else
        {
            auto &lo = std::get<1>(m_up2u);
            size_t lower_bound = 0;
            for (auto i: lo.m_posCounts)
            {
                if (lower_bound < ind && ind <= i)
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
                lower_bound = i + 1;
            }
            if (!lo.m_unlimited && ind > lo.m_posArgs.size() + 1)
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
            return {help_full(argv), ind};
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
                return ret.m_index + ind;
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
                return {help_full(argv), ind};
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
                    return {help_full(argv), ind};
                else
                    return {help_tip(std::string("Unknown flag: -")+=sname, argv), ind};
            } // while (char sname = *++arg)
    NextInd:;
    } // for (; ind < argc; ++ind)

    if (m_onParsed)
        m_onParsed();

    return flagStartInd;
}

std::string C_EZArgs::help_full(const char *argv[]) const
{
    // Synthesize USAGE
    std::string help;
    for (auto cur = this; cur; cur = cur->m_owner)
    {
        const auto arg0 = *(argv--);
        if (help.empty())
            help = arg0;
        else
            help = std::string(arg0) + ' ' + help;
    }
    help = "USAGE: " + help;
    std::string validActions;
    switch (m_up2u.index())
    {
    case 0:
        break;
    case 1:
        {
            auto &lo = std::get<1>(m_up2u);
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
    case 2:
        validActions += "VALID ACTIONS:\n";
        {
            auto &map = std::get<2>(m_up2u);
            std::string actions;
            size_t positionalCount{};
            for (auto &i: map)
            {
                actions += actions.empty()? '(': '|';
                actions += i.first;
                ((validActions += "  ") += i.first) += '\n';
                if (!i.second.m_desc.empty())
                    ((validActions += '\t') += i.second.m_desc) += '\n';
                if (i.second.m_up2u.index())
                    ++positionalCount;
            }
            (help += ' ') += actions;
            if (!positionalCount)
                help += ')';
            else if (positionalCount < map.size())
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
        if (m_desc.back() != '\n')
            help += '\n';
    }

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

    if (!m_details.empty())
    {
        help += "\n"
                "DETAILED:\n"
                "  ";
        help += m_details;
        if (m_details.back() != '\n')
            help += '\n';
    }
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

std::string C_EZArgs::help_tip(const std::string &error, const char *argv[]) const
{
    const char *helpFlag{};
    if (!m_hShielded)
        helpFlag = "-h";
    else if (!m_helpShielded)
        helpFlag = "--help";

    if (helpFlag)
    {
        std::string tip;
        for (auto cur = this; cur; cur = cur->m_owner)
            tip = std::string(*(argv--)) + ' ' + tip;

        return error + "\nType \"" + tip + helpFlag + "\" to read the help";
    }
    return error;
}

} //namespace bux
