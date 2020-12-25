#ifndef bux_GLR_H_
#define bux_GLR_H_

#include "ParserBase.h" // bux::T_StateID, bux::F_GetProducedT<>, LexBase.h
#include <memory>       // std::shared_ptr<>
#include <optional>     // std::optional<>
#include <vector>       // std::vector<>

namespace bux {
namespace GLR {

//
//      Constants
//
enum
{
    ACTION_SHIFT,
    ACTION_ACCEPT,
    ACTION_REDUCE_MIN   // Ensure it's the last ACTION_-constant
};

//
//      Types
//
typedef unsigned T_ActionId;
typedef std::shared_ptr<const I_LexAttr> C_LexPtr;
typedef C_LexInfoT<const I_LexAttr,std::shared_ptr> C_LexInfo;
typedef F_GetProducedT<const I_LexAttr,std::shared_ptr> F_GetProduced;

class C_Parser;
typedef std::function<void(C_Parser &parser)> F_OncePostShift;
typedef std::function<void(C_Parser&, const F_GetProduced&, C_LexPtr&, F_OncePostShift&)> FH_Reduce;

struct I_ParserPolicy
{
    // Types
    struct C_ReduceInfo
    {
        size_t              m_PopLength;
        T_LexID             m_ResultID;
        FH_Reduce           m_Reduce;
    };

    // Virtuals
    virtual std::vector<T_ActionId> action(T_StateID state, T_LexID token) const =0;
    virtual bool changeToken(T_LexID &token, C_LexPtr &attr) const;
    virtual size_t getAcceptId() const =0;
    virtual bool getTokenName(T_LexID token, std::string &name) const;
    virtual T_StateID nextState(T_StateID state, T_LexID lex) const =0;
    virtual void getReduceInfo(size_t prodId, C_ReduceInfo &info) const =0;
    virtual void onError(void *userData, const C_SourcePos &pos, const std::string &message) const = 0;

    // Nonvirtuals
    std::string printToken(T_LexID token) const;
};

class C_Parser: public I_Parser
{
public:

    // Data
    const I_ParserPolicy    &m_policy;

    // Nonvirtuals
    C_Parser(const I_ParserPolicy &policy);
    void eachAccepted(std::function<void(C_LexPtr &)> apply);
    void onError(const C_SourcePos &pos, const std::string &message) const;
    void userData(void *p) { m_userData = p; }
    void *userData() const { return m_userData; }

    // Implement I_Parser
    void add(T_LexID token, unsigned line, unsigned col, I_LexAttr *unownedAttr) override;
    std::string_view setSource(std::string_view src) override;

private:

    // Types
    struct C_StateLR1;
    typedef std::shared_ptr<C_StateLR1> C_StateLR1Ptr;

    struct C_StateLR1: C_LexInfo
    {
        // Data
        C_StateLR1Ptr       m_prev;
        T_StateID           m_StateID;
        T_LexID             m_TokenID;

        // Nonvirtuals
        C_StateLR1() = default;
        C_StateLR1(C_StateLR1 &another);
        void operator=(C_StateLR1 &another);
    };
    typedef std::vector<C_StateLR1Ptr> C_StateLR1Ptrs;
    typedef std::pair<C_StateLR1Ptr,F_OncePostShift> T_Reduced;

    // Data
    void                    *m_userData{};
    C_StateLR1Ptrs          m_curTops, m_accepted;
    std::string_view        m_srcPath;
    bool                    m_added{};

    // Nonvirtuals
    C_Parser(C_Parser &root, const C_StateLR1Ptr &nestedTop);
    std::optional<T_Reduced> reduceOn(size_t id, C_StateLR1Ptr iTop, const C_SourcePos &pos);
    static T_StateID state(C_StateLR1Ptr &p);
};

} // namespace GLR
} // namespace bux

#endif // bux_GLR_H_
