#ifndef ImplScannerH
#define ImplScannerH

#include "ScannerBase.h"    // bux::C_LexTraits<>, bux::I_Scanner<>, bux::C_ActionRet, ...
#include <vector>           // std::vector<>

namespace bux {

//
//      Types
//
template<class T_Input, class T_State, class T_Char, class C_Traits = C_LexTraits<T_Char>>
class C_ScannerImpl: public I_Scanner<T_Char>
{
public:

    // Types
    typedef C_ActionRet F_Action(const T_Char *c, size_t n);
    typedef bool F_IsFinal(const T_Char *c, size_t n);

    struct C_GotoPair
    {
        T_Input             m_inputLB;
        T_State             m_nextState;
    };

    struct C_StateRec
    {
        const C_GotoPair    *m_goto;
        F_Action            *m_action;
    };

    // Nonvirtuals
    C_ScannerImpl(I_Parser &parser);

    // Implement I_Scanner<T_Char>
    void add(unsigned col, T_Char c) override;
    void setLine(unsigned line) override;
    void setSource(std::string_view src) override;

protected:

    // Nonvirtuals
    void firstFits(const T_State *states, F_IsFinal *const *isFinal, size_t stateN)
        { m_1stFits = states; m_isFinal = isFinal; m_1stFitN = stateN; }
    void stateTables(const C_StateRec *stateRecs, const T_Input *gotoN)
        { m_stateRecs = stateRecs; m_gotoN = gotoN; }

private:

    // Types
    typedef std::vector<T_Char> C_ChStack;
    typedef std::vector<C_SourcePos> C_PosStack;

    // Data
    I_Parser                &m_Parser;
    std::string_view        m_OldSrc;
    std::string_view        m_CurSrc;
    unsigned                m_CurLine;
    //---- Read State Begins
    int                     m_LastSuccess;
    F_Action                *m_pAction; // valid if m_LastSuccess >= 0
    T_State                 m_CurState;
    C_ChStack               m_ReadCh,   m_UnreadCh;
    C_PosStack              m_ReadPos,  m_UnreadPos;
    //---- Read State Ends
    //---- Transit Table Begins
    const C_StateRec        *m_stateRecs    {nullptr};
    const T_Input           *m_gotoN        {nullptr};
    const T_State           *m_1stFits      {nullptr};
    F_IsFinal *const        *m_isFinal      {nullptr};
    size_t                  m_1stFitN       {0};
    //---- Transit Table Ends

    // Nonvirtuals
    void addToken(T_LexID token, C_SourcePos pos, I_LexAttr *unownedAttr);
    void resetReadState();
    void shrinkReadSize(size_t newSize);
};

//
//      Implement Class Templates
//
template<class T_Input, class T_State, class T_Char, class C_Traits>
C_ScannerImpl<T_Input,T_State,T_Char,C_Traits>::C_ScannerImpl(I_Parser &parser): m_Parser(parser)
{
    resetReadState();
}

template<class T_Input, class T_State, class T_Char, class C_Traits>
void C_ScannerImpl<T_Input,T_State,T_Char,C_Traits>::add(unsigned col, T_Char c)
{
    for (bool consumed = false;;)
    {
        // Read the next char
        if (m_UnreadCh.empty())
        {
            if (consumed)
                // The only stop condition
                break;

            m_ReadCh.emplace_back(c);
            m_ReadPos.emplace_back(m_CurSrc, m_CurLine, col);
            consumed =true;
        }
        else
        {
            m_ReadPos.emplace_back(m_UnreadPos.back());
            m_UnreadPos.pop_back();
            m_ReadCh.emplace_back(m_UnreadCh.back());
            m_UnreadCh.pop_back();
        }

        // Match the char against the transit table
        const T_LexID idTop = C_Traits::id(m_ReadCh.back());
        if (idTop < MIN_TOKEN_ID)
        {
            const auto gotos = m_stateRecs[m_CurState].m_goto;
            T_State nextState;
            bool found = false;
            for (int i = m_gotoN[m_CurState]; i > 0;)
            {
                const auto pt = gotos[--i];
                if (pt.m_inputLB <= idTop)
                {
                    nextState = pt.m_nextState;
                    if (std::numeric_limits<T_State>::max() != nextState)
                        found = true;
                    break;
                }
            }
            if (found)
                // Transition found
            {
                m_CurState = nextState;
                m_pAction = m_stateRecs[nextState].m_action;
                if (m_pAction)
                    // Is final
                {
                    for (size_t i = 0; i < m_1stFitN; ++i)
                        if (m_1stFits[i] == nextState &&
                            (!m_isFinal[i] || (*m_isFinal[i])(m_ReadCh.data(), m_ReadCh.size())))
                            // First fit - action right now
                        {
                            const C_ActionRet ret = (*m_pAction)(m_ReadCh.data(), m_ReadCh.size());
                            addToken(ret.m_id, m_ReadPos.front(), ret.m_pAttr);
                            return;
                        }
                    m_LastSuccess = int(m_ReadCh.size());
                }
                return;
            }
        } // if (id < MIN_TOKEN_ID)

        // No transition - Claim the new token
        const auto pos = m_ReadPos.front();
        T_LexID token;
        I_LexAttr *attr{};
        if (m_LastSuccess < 0)
            // No final state ever visited -- unread all but the first
        {
            shrinkReadSize(1);
            token = C_Traits::id(m_ReadCh.front());
        }
        else if (!m_pAction)
            // Bug ?
        {
            std::string buf;
            for (auto i: m_ReadCh)
                switch (auto id = C_Traits::id(i))
                {
                case TID_EOF:
                    buf += "EOF";
                    break;
                default:
                    buf += to_utf8(id);
                }
            RUNTIME_ERROR("Run out of scanner at " <<pos.m_Source <<'(' <<pos.m_Line <<',' <<pos.m_Col <<") |" <<buf <<'|');
        }
        else
            // Conclude on the latest visited final state
        {
            shrinkReadSize(size_t(m_LastSuccess));
            const C_ActionRet ret = (*m_pAction)(m_ReadCh.data(), m_ReadCh.size());
            token = ret.m_id;
            attr  = ret.m_pAttr;
        }

        // Clean up the read state (compared with the unread counterpart)
        addToken(token, pos, attr);
    }
}

template<class T_Input, class T_State, class T_Char, class C_Traits>
void C_ScannerImpl<T_Input,T_State,T_Char,C_Traits>::addToken(
    T_LexID                 token,
    C_SourcePos             pos,
    I_LexAttr               *attr   )
{
    resetReadState();

    // Add new token to parser
    if (m_OldSrc != pos.m_Source)
    {
        (void)m_Parser.setSource(pos.m_Source);
        m_OldSrc = pos.m_Source;
    }
    m_Parser.add(token, pos.m_Line, pos.m_Col, attr);
}

template<class T_Input, class T_State, class T_Char, class C_Traits>
void C_ScannerImpl<T_Input,T_State,T_Char,C_Traits>::resetReadState()
{
    m_LastSuccess = -1; // No success ever being made
    m_CurState = 0;     // presumably the starting state
    m_ReadCh.clear();
    m_ReadPos.clear();
}

template<class T_Input, class T_State, class T_Char, class C_Traits>
void C_ScannerImpl<T_Input,T_State,T_Char,C_Traits>::setLine(unsigned line)
{
    m_CurLine = line;
}

template<class T_Input, class T_State, class T_Char, class C_Traits>
void C_ScannerImpl<T_Input,T_State,T_Char,C_Traits>::setSource(std::string_view src)
{
    m_CurSrc = src;
}

template<class T_Input, class T_State, class T_Char, class C_Traits>
void C_ScannerImpl<T_Input,T_State,T_Char,C_Traits>::shrinkReadSize(size_t newSize)
{
    while (newSize < m_ReadCh.size())
    {
        m_UnreadCh.emplace_back(m_ReadCh.back());
        m_ReadCh.pop_back();
        m_UnreadPos.emplace_back(m_ReadPos.back());
        m_ReadPos.pop_back();
    }
}

} //namespace bux

#endif  // ImplScannerH
