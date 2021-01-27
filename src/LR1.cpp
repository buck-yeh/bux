#include "LR1.h"
#include "LogStream.h"  // HRTN()
#include <fmt/core.h>   // fmt::format()
#include <limits>       // std::numeric_limits<>

namespace bux {
namespace LR1 {

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
            return fmt::format(FMT_STRING("bux::TOKENGEN_LB+{}"), token - TOKENGEN_LB);

        std::string out = fmt::format(FMT_STRING("0x{:x}"), token);
        if (isascii(token))
            out += fmt::format(FMT_STRING(" or \'{}\'"), asciiLiteral(char(token)));
        return out;
    }
}

C_Parser::C_Parser(const I_ParserPolicy &policy):
    m_Policy(policy),
    m_ErrState(std::numeric_limits<T_StateID>::max()),
    m_ShiftCountdown(0),
    m_Accepted(false)
{
}

void C_Parser::add(T_LexID token, unsigned line, unsigned col, I_LexAttr *unownedAttr)
{
    C_LexInfo   info;
    info.m_attr.assign(unownedAttr, true);
    info.m_pos.m_Source = m_CurSrc;
    info.m_pos.m_Line = line;
    info.m_pos.m_Col  = col;

    C_StateStackLR1 unreadStack;
    add(token, info, unreadStack);
}

void C_Parser::add(T_LexID token, C_LexInfo &info, C_StateStackLR1 &unreadStack)
{
    size_t actionid;
Again:
    if (m_Accepted)
        onError(info, "Already accepted");

    actionid = m_Policy.action(currentState(), token);
    switch (actionid)
    {
    case ACTION_SHIFT:
        shift(token, info);
        if (m_ShiftCountdown && !--m_ShiftCountdown)
            // Call and forget
        {
            std::function<void()> t;
            t.swap(m_OnPostShift);
            t();
        }
        break;
    case ACTION_ACCEPT:
        if (m_OnPostShift)
            // Call and forget
        {
            std::function<void()> t;
            t.swap(m_OnPostShift);
            m_ShiftCountdown = 0;
            t();
        }
        if (!reduceOn(m_Policy.getAcceptId(), info))
            onError(info, "Reduction error on acception");

        m_Accepted = true;
        break;
    case ACTION_ERROR: // Error
        if (m_Policy.changeToken(token, info.m_attr))
            goto Again;

        if (m_ErrState == std::numeric_limits<T_StateID>::max())
        {
            m_ErrState = currentState();
            m_ErrToken = token;
            m_ErrPos   = info.m_pos;
        }

        if (recover(token, info, unreadStack))
            // Recoverable
        {
            info.m_attr.assign(0, false);
            add(m_Policy.m_IdError, info, unreadStack);
        }
        else
            // Unrecoverable
        {
            auto out = fmt::format(FMT_STRING("Syntax error on state={} token={}"), m_ErrState, m_Policy.printToken(m_ErrToken));
            if (auto *attr =info.m_attr.get())
                out.append(" of attr type ").append(HRTN(*attr));
            else
                out += " with null attr";

            if (m_CurStack.empty())
                out += "\nEmpty stack";
            else
            {
                out += fmt::format(FMT_STRING("\nStack[{}] Dump:"), m_CurStack.size()-1);
                bool first = true;
                for (const auto &i: m_CurStack)
                {
                    if (first)
                        first = false;
                    else
                        out += fmt::format(FMT_STRING("\n({},{})\t{}\ts={}\tt={}"),
                                i.m_pos.m_Line, i.m_pos.m_Col, i.m_attr?HRTN(*i):"", i.m_StateID, m_Policy.printToken(i.m_TokenID));
                }
            }
            onError(m_ErrPos, out);
            if (token == TID_EOF)
                m_Accepted = true;
            else
            {
                panicRollback(unreadStack); // panic mode the cheapest
                m_ErrState = std::numeric_limits<T_StateID>::max();
            }
        }
        break;
    default:
        if (actionid >= ACTION_REDUCE_MIN)
        {
            const size_t prodId = actionid - ACTION_REDUCE_MIN;
            if (reduceOn(prodId, info.m_pos))
            {
                m_ErrState = std::numeric_limits<T_StateID>::max();
                goto Again;
            }
            onError(info, "Reduction error on production "+std::to_string(prodId));
        }
        onError(info, "Unknown action id "+std::to_string(actionid));
    } // switch (actionid)

    // Consume unread
    if (!unreadStack.empty())
    {
       token = unreadStack.top().m_TokenID;
       info = unreadStack.top();
       unreadStack.pop();
       goto Again;
    }
}

T_StateID C_Parser::currentState() const
{
    if (m_CurStack.empty())
        return 0; // 0 is reserved as the ID of [ S' -> .S ]

    return m_CurStack.top().m_StateID;
}

void C_Parser::onError(const C_SourcePos &pos, const std::string &message)
{
    m_Policy.onError(*this, pos, message);
}

void C_Parser::panicRollback(C_StateStackLR1 &unreadStack)
{
    size_t finalSize;
    for (finalSize = 0; finalSize < m_CurStack.size(); ++finalSize)
    {
        if (m_CurStack[finalSize].m_TokenID == m_Policy.m_IdError)
            break;
    }

    while (!unreadStack.empty())
    {
        m_CurStack.push(unreadStack.top());
        unreadStack.pop();
    }
    while (m_CurStack.size() > finalSize)
    {
        C_StateLR1 &top = m_CurStack.top();
        if (top.m_TokenID != m_Policy.m_IdError)
            unreadStack.push(top);

        m_CurStack.pop();
    }
}

bool C_Parser::recover(T_LexID token, C_LexInfo &info, C_StateStackLR1 &unreadStack)
{
    bool isLast = true;
    for (auto i = m_CurStack.size(); i; --i, isLast = false)
    {
        if (!isLast && m_CurStack[i].m_TokenID == m_Policy.m_IdError)
            // No crossing old m_Policy.m_IdError
            break;

        if (m_Policy.action(m_CurStack[i-1].m_StateID, m_Policy.m_IdError) != ACTION_ERROR)
        {
            if (!isLast && info.m_pos == m_CurStack[i].m_pos)
                // Total length of nonempty ungotten tokens is zero.
                continue;

            for (size_t j = i; j; --j, isLast = false)
            {
                if (m_CurStack[j-1].m_pos < (isLast?info.m_pos:m_CurStack[j].m_pos))
                    break;
                if (m_CurStack[j-1].m_TokenID == m_Policy.m_IdError)
                    return false;
            }

            auto &t = unreadStack.push();
            t.m_TokenID = token;
            static_cast<C_LexInfo &>(t) = info;
            while (m_CurStack.size() > i)
            {
                unreadStack.push(m_CurStack.top());
                m_CurStack.pop();
            }
            info.m_pos = unreadStack.top().m_pos;
            return true;
        }
    }
    return false;
}

bool C_Parser::reduceOn(size_t id, const C_SourcePos &pos)
{
    I_ParserPolicy::C_ReduceInfo ri;
    m_Policy.getReduceInfo(id, ri);
    if (m_CurStack.size() >= ri.m_PopLength)
    {
        const auto base = m_CurStack.end() - ri.m_PopLength;
        C_LexInfo li;
        if (ri.m_Reduce)
            ri.m_Reduce(*this,
                        [=,this,size=ri.m_PopLength](size_t n)->C_LexInfo& {
                            if (n >= size)
                                onError(pos, "Production["+std::to_string(id)+"] out of range "+std::to_string(n)+'/'+std::to_string(size));

                            return base[n];
                        }, li.m_attr);
        if (ri.m_PopLength)
        {
            li.m_pos = base->m_pos;
            m_CurStack.pop(ri.m_PopLength);
        }
        else
            li.m_pos = pos;

        shift(ri.m_ResultID, li);
        return true;
    }
    return false;
}

void C_Parser::reservePostShift(std::function<void()> calledOnce, unsigned shifts)
{
    m_ShiftCountdown = shifts;
    m_OnPostShift = calledOnce;
}

std::string_view C_Parser::setSource(std::string_view src)
{
    const auto ret = m_CurSrc;
    m_CurSrc = src;
    return ret;
}

void C_Parser::shift(T_LexID token, C_LexInfo &info)
{
    const auto state = currentState();
    auto &t = m_CurStack.push();
    t.m_StateID = token != ROOT_NID? m_Policy.nextState(state, token): state;
    t.m_TokenID = token;
    static_cast<C_LexInfo&>(t) = info;
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
    m_StateID = another.m_StateID;
    m_TokenID = another.m_TokenID;
}

} // namespace LR1
} // namespace bux
