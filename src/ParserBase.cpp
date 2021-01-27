#include "ParserBase.h"
//----------------------------------------------------------
#include <fmt/core.h>   // fmt::format()

namespace bux {

//
//      Implement Classes
//
void C_ParserLogCount::log(E_LogLevel ll, const C_SourcePos &pos, std::string_view message)
{
    ++m_count.at(ll);
    static constinit char const *const LELEL_NAMES[]{"Fatal","Error","Warn","Info","Verbose"};
    println(fmt::format("{} {}: {}", toStr(pos), LELEL_NAMES[ll], message));
}

std::string C_ParserLogCount::toStr(const C_SourcePos &pos) const
{
    if (pos.m_Source.empty())
        return fmt::format("({},{})", pos.m_Line, pos.m_Col);
    else
        return fmt::format("{}@({},{})", pos.m_Source, pos.m_Line, pos.m_Col);
}

void C_ParserOStreamCount::println(const std::string &line)
{
    m_out <<line <<'\n';
}

} //namespace bux
