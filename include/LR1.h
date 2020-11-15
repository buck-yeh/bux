#ifndef LR1H
#define LR1H

#include "ParserBase.h" // bux::T_StateID, bux::F_GetProducedT<>, LexBase.h
#include "XAutoPtr.h"   // bux::C_AutoNode<>
#include "Xtack.h"      // bux::C_ResourceStack<>

namespace bux {
namespace LR1 {

//
//      Constants
//
enum
{
    ACTION_SHIFT,
    ACTION_ACCEPT,
    ACTION_ERROR,
    ACTION_REDUCE_MIN   // Ensure it's the last ACTION_-constant
};

//
//      Types
//
typedef C_AutoNode<I_LexAttr> C_LexPtr;
typedef F_GetProducedT<I_LexAttr,C_AutoNode> F_GetProduced;

class C_Parser;

struct I_ParserPolicy
{
    // Types
    typedef std::function<void(C_Parser &parser, const F_GetProduced &args, C_LexPtr &ret)> FH_Reduce;

    struct C_ReduceInfo
    {
        size_t              m_PopLength;
        T_LexID             m_ResultID;
        FH_Reduce           m_Reduce;
    };

    // Data
    const T_LexID m_IdError;

    // Ctor/Dtor
    constexpr I_ParserPolicy(T_LexID idError): m_IdError(idError) {}
    virtual ~I_ParserPolicy() =default;

    // (Almost) Pure virtuals
    virtual size_t action(T_StateID state, T_LexID token) const =0;
        ///< Return action kind
    virtual bool changeToken(T_LexID &token, C_LexPtr &attr) const;
        ///< A chance to save action error
    virtual size_t getAcceptId() const =0;
        ///< Return id which will be passed as the 1st arg of getReduceInfo()
    virtual bool getTokenName(T_LexID token, std::string &name) const;
        ///< Return true if token has a name
    virtual T_StateID nextState(T_StateID state, T_LexID lex) const =0;
        ///< Goto table
    virtual void getReduceInfo(size_t id, C_ReduceInfo &info) const =0;
        ///< Get reduction if available
    virtual void onError(C_Parser &parser, const C_SourcePos &pos, const std::string &message) const =0;
        ///< Report error (to log or to throw)

    // Nonvirtuals
    std::string printToken(T_LexID token) const;
};

typedef C_LexInfoT<I_LexAttr,C_AutoNode> C_LexInfo;

class C_Parser: public I_Parser
{
public:

    // Data
    const I_ParserPolicy    &m_Policy;

    // Ctor/Dtor
    C_Parser(const I_ParserPolicy &policy);

    // Nonvirtuals
    bool accepted() const { return m_Accepted; }
    auto &getFinalLex() { return m_CurStack.top(); }
    void onError(const C_SourcePos &pos, const std::string &message);
    void reservePostShift(std::function<void()> calledOnce, unsigned shifts);

    // Implement I_Parser
    void add(T_LexID token, unsigned line, unsigned col, I_LexAttr *unownedAttr) override;
    std::string_view setSource(std::string_view src) override;

private:

    // Types
    struct C_StateLR1: C_LexInfo
    {
        // Data
        T_StateID           m_StateID;
        T_LexID             m_TokenID;

        // Nonvirtuals
        C_StateLR1() = default;
        C_StateLR1(C_StateLR1 &another);
        void operator=(C_StateLR1 &another);
    };
    typedef C_ResourceStack<C_StateLR1> C_StateStackLR1;

    // Data
    C_StateStackLR1         m_CurStack;
    std::string_view        m_CurSrc;
    T_StateID               m_ErrState;
    T_LexID                 m_ErrToken;
    C_SourcePos             m_ErrPos;
    std::function<void()>   m_OnPostShift;
    unsigned                m_ShiftCountdown;
    bool                    m_Accepted;

    // Nonvirtuals
    void add(T_LexID token, C_LexInfo &info, C_StateStackLR1 &unreadStack);
        // Called by public add() or itself
    T_StateID currentState() const;
        //
    void panicRollback(C_StateStackLR1 &unreadStack);
        //
    bool recover(T_LexID token, C_LexInfo &info, C_StateStackLR1 &unreadStack);
        // Adjust the current state so that parsing can continue.
        // Return true on success
    bool reduceOn(size_t id, const C_SourcePos &pos);
        //
    void shift(T_LexID lex, C_LexInfo &info);
        // Shift next state thru given token
};

template<class T_Data>
struct C_NewLex: C_NewNode<C_LexDataT<T_Data>>
{
    // Nonvirtuals
    C_NewLex() = default;
    explicit C_NewLex(C_LexInfo &i): C_NewNode<C_LexDataT<T_Data>>(C_Void{})
    {
        if (!i)
            this->assign(new C_LexDataT<T_Data>, true);
        else if (!this->takeOver(i.m_attr))
            RUNTIME_ERROR(typeid(*i).name())
    }
    template<class...T_Args>
    explicit C_NewLex(T_Args&&...args):
        C_NewNode<C_LexDataT<T_Data>>(std::forward<T_Args>(args)...) {}
};

} // namespace LR1
} // namespace bux

#endif // LR1H
