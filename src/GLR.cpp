#include "GLR.h"
#include "LogStream.h"  // HRTN()
#include <fmt/core.h>   // fmt::format()

namespace bux {
namespace GLR {

//
//      Implement Classes
//
bool I_ParserPolicy::changeToken(T_LexID &, C_LexPtr &) const
{
    return false;
}

bool I_ParserPolicy::getTokenName(T_LexID, std::string &) const
{
    return false;
}

std::string I_ParserPolicy::printToken(T_LexID token) const
{
    std::string name;
    if (getTokenName(token, name))
        return name;

    switch (token)
    {
    case TID_EOF:
        return "bux::TID_EOF";
    case ROOT_NID:
        return "<@> aka bux::ROOT_NID";
    default:
        if (token >= TOKENGEN_LB)
            return fmt::format("bux::TOKENGEN_LB+{}", token - TOKENGEN_LB);

        auto out = fmt::format("0x{:x}", token);
        if (isascii(int(token)))
            out.append(" or \'").append(asciiLiteral(token)) += '\'';

        return out;
    }
}

C_Parser::C_Parser(const I_ParserPolicy &policy): m_policy(policy)
{
}

C_Parser::C_Parser(C_Parser &root, const C_StateLR1Ptr &nestedTop):
    m_policy(root.m_policy),
    m_userData(root.m_userData)
{
    m_curTops.emplace_back(nestedTop);
}

void C_Parser::add(T_LexID token, unsigned line, unsigned col, I_LexAttr *unownedAttr)
{
    C_LexInfo   info;
    info.m_attr.reset(unownedAttr);
    info.m_pos.m_Source = m_srcPath;
    info.m_pos.m_Line = line;
    info.m_pos.m_Col  = col;

    if (!m_accepted.empty())
        onError(info, "Already accepted");

    std::vector<std::pair<C_StateLR1Ptr,F_OncePostShift>> reduced;
    C_StateLR1Ptrs  nextTops;
Again:
    C_StateLR1Ptrs  curTops{m_curTops};
    while (!reduced.empty() || !curTops.empty() || !m_added)
    {
        m_added = true;

        C_StateLR1Ptr iTop;
        F_OncePostShift iPostShift;
        if (!reduced.empty())
        {
            std::tie(iTop,iPostShift) = reduced.back();
            reduced.pop_back();
        }
        else if (!curTops.empty())
        {
            iTop = curTops.back();
            curTops.pop_back();
        }
        const auto fromState = state(iTop);
        for (auto j: m_policy.action(fromState, token))
            switch (j)
            {
            case ACTION_SHIFT:
                {
                    C_StateLR1Ptr t{new C_StateLR1};
                    t->m_prev = iTop;
                    t->m_StateID = m_policy.nextState(fromState, token);
                    t->m_TokenID = token;
                    static_cast<C_LexInfo&>(*t) = info;
                    if (!iPostShift)
                        nextTops.emplace_back(t);
                    else
                    {
                        C_Parser nestedGLR{*this, t};
                        iPostShift(nestedGLR);
                        nextTops.insert(nextTops.end(), nestedGLR.m_curTops.begin(), nestedGLR.m_curTops.end());
                        m_accepted.insert(m_accepted.end(), nestedGLR.m_accepted.begin(), nestedGLR.m_accepted.end());
                    }
                }
                break;
            case ACTION_ACCEPT:
                if (!iPostShift)
                {
                    if (auto ret = reduceOn(m_policy.getAcceptId(), iTop, info))
                        m_accepted.push_back(ret->first); // ret->second not checked
                    else
                        onError(info, "Reduction error on acception");
                }
                else
                {
                    C_Parser nestedGLR{*this, iTop};
                    iPostShift(nestedGLR);
                    for (auto &k: nestedGLR.m_curTops)
                        m_accepted.push_back(k);

                    if (!nestedGLR.m_accepted.empty())
                        onError(info, "Doubly accepted");
                }
                break;
            default:
                if (j >= ACTION_REDUCE_MIN)
                {
                    const size_t prodId = j - ACTION_REDUCE_MIN;
                    if (auto ret = reduceOn(prodId, iTop, info))
                    {
                        if (ret->second)
                            reduced.emplace_back(*ret);
                        else
                            curTops.emplace_back(ret->first);
                    }
                    else
                        onError(info, "Reduction error on production "+std::to_string(prodId));
                }
                else
                    onError(info, "Unknown action id "+std::to_string(j));
            }
    }
    if (nextTops.empty() && m_accepted.empty())
        // No action - Try recovery coups or claim error
    {
        if (m_policy.changeToken(token, info.m_attr))
            goto Again;

        auto out = "Syntax error on token=" + m_policy.printToken(token);
        if (auto *attr = info.m_attr.get())
            out.append(" of attr type ").append(HRTN(*attr));
        else
            out += " with null attr";

        if (const auto n = m_curTops.size())
        {
            out += "\nStack dump";
            if (n > 1)
                out += fmt::format(" on {} paths", n);

            out += ':';
            size_t ind{};
            for (auto i: m_curTops)
            {
                out += fmt::format("\nPath[{}]:", ind++);
                while (i)
                {
                    out += fmt::format("\n({},{})\t", i->m_pos.m_Line, i->m_pos.m_Col);
                    if (i->m_attr)
                        out += HRTN(*i->m_attr);

                    out += fmt::format("\ts={}\tt={}", i->m_StateID, m_policy.printToken(i->m_TokenID));
                    i = i->m_prev;
                }
            }
        }
        onError(info, out);
        return;
    }
    m_curTops.swap(nextTops);
}

void C_Parser::eachAccepted(std::function<void(C_LexPtr &)> apply)
{
    for (auto &i: m_accepted)
        apply(i->m_attr);
}

void C_Parser::onError(const C_SourcePos &pos, std::string_view message)
{
    m_policy.onError(*this, pos, message);
}

auto C_Parser::reduceOn(size_t id, C_StateLR1Ptr iTop, const C_SourcePos &pos) -> std::optional<T_Reduced>
{
    I_ParserPolicy::C_ReduceInfo ri;
    m_policy.getReduceInfo(id, ri);
    C_StateLR1Ptrs arr;
    const size_t size = ri.m_PopLength;
    while (arr.size() < size)
    {
        if (!iTop)
            return {};

        arr.emplace_back(iTop);
        iTop = iTop->m_prev;
    }

    C_StateLR1Ptr r{new C_StateLR1};
    F_OncePostShift ps;
    if (ri.m_Reduce)
        ri.m_Reduce(*this, [&](size_t n)->C_LexInfo& {
            if (n >= size)
                onError(pos, "Production["+std::to_string(id)+"] out of range "+std::to_string(n)+'/'+std::to_string(size));

            return *arr.rbegin()[int(n)];
        }, r->m_attr, ps);

    r->m_pos = size? arr.back()->m_pos: pos;
    r->m_prev = iTop;
    const auto curStateId = state(iTop);
    r->m_StateID = ri.m_ResultID != ROOT_NID? m_policy.nextState(curStateId, ri.m_ResultID): curStateId;
    r->m_TokenID = ri.m_ResultID;
    return std::optional<T_Reduced>{std::in_place, r, ps};
}

std::string_view C_Parser::setSource(std::string_view src)
{
    const auto ret = m_srcPath;
    m_srcPath = src;
    return ret;
}

T_StateID C_Parser::state(C_StateLR1Ptr &p)
{
    return p? p->m_StateID: decltype(p->m_StateID){};
}

C_Parser::C_StateLR1::C_StateLR1(C_StateLR1 &another):
    C_LexInfo(another),
    m_StateID(another.m_StateID),
    m_TokenID(another.m_TokenID)
{
}

void C_Parser::C_StateLR1::operator=(C_StateLR1 &another)
{
    C_LexInfo::operator=(another);
    m_StateID =another.m_StateID;
    m_TokenID =another.m_TokenID;
}

} // namespace GLR
} // namespace bux
