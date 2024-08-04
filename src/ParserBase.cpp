#include "ParserBase.h"
//----------------------------------------------------------
#include <format>       // std::format()
#include <ostream>      // std::ostream

namespace bux {

//
//      Implement Classes
//
void C_ParserLogCount::log(E_LogLevel ll, const C_SourcePos &pos, std::string_view message)
{
    ++m_count.at(ll);
    static constinit char const *const LELEL_NAMES[]{"Fatal","Error","Warn","Info","Verbose"};
    println(std::format("{} {}: {}", toStr(pos), LELEL_NAMES[ll], message));
}

std::string C_ParserLogCount::toStr(const C_SourcePos &pos) const
{
    if (pos.m_Source.empty())
        return std::format("({},{})", pos.m_Line, pos.m_Col);
    else
        return std::format("{}@({},{})", pos.m_Source, pos.m_Line, pos.m_Col);
}

void C_ParserOStreamCount::println(const std::string &line)
{
    m_out <<line <<'\n';
}

} //namespace bux
