#pragma once

#ifdef __BORLANDC__
#   pragma warn -8058
#   pragma warn -8027
#endif

#include "XException.h" // RUNTIME_ERROR()
#include <string>       // std::string
#include <string_view>  // std::string_view
#include <type_traits>  // std::is_base_of_v<>, std::is_arithmetic_v<>

namespace bux {

//
//      Constants
//
enum: uint32_t
{
    MIN_TOKEN_ID            = 0x70000000,

    // Must-have non-lexical symbols
    TID_EOF                 = MIN_TOKEN_ID,
    ROOT_NID,                               // NID for the starting symbol <@>

    // Base of parser-generated identifiers
    TOKENGEN_LB
};

//
//      Types
//
using T_LexID = uint32_t;

struct C_SourcePos
{
    // Data
    std::string_view        m_Source;
    unsigned                m_Line;
    unsigned                m_Col;

    // Nonvirtuals
    C_SourcePos() = default;
    C_SourcePos(std::string_view source, unsigned line, unsigned col) noexcept;
    C_SourcePos(const C_SourcePos &a) = default;
    C_SourcePos &operator=(const C_SourcePos &a) = default;
};

template<class T, template<class> class C_Ptr>
struct C_LexInfoT
{
    // Data
    C_Ptr<T>                m_attr;
    C_SourcePos             m_pos;

    // Nonvirtuals
    C_LexInfoT() = default;
    C_LexInfoT(C_LexInfoT &another): m_attr(another.m_attr), m_pos(another.m_pos) {}
    C_LexInfoT &operator=(C_LexInfoT &another) { m_attr = another.m_attr; m_pos = another.m_pos; return *this; }
    //--------------------------------------------------------
    operator               bool() const { return bool{m_attr}; }
    operator          C_Ptr<T>&()       { return m_attr; }
    operator const C_SourcePos&() const { return m_pos; }
    template<class T2>
    operator                T2*() const { return dynamic_cast<T2*>(get()); }
    //--------------------------------------------------------
    T &operator *() const { return *m_attr; }
    T *get() const { return m_attr.get(); }
};

template<class T, template<class> class C_Ptr>
struct C_RetLvalT
{
    // Data
    C_Ptr<T> &m_lval;

    // Nonvirtuals
    C_RetLvalT(C_Ptr<T> &lval): m_lval(lval) {}
    template<class T2>
    void operator =(C_LexInfoT<T2,C_Ptr> &rval) const { m_lval = rval.m_attr; }
    template<class T2>
    void operator =(C_Ptr<T2> &rval) const { m_lval = rval; }
    void operator =(T *ptr) const { m_lval.reset(ptr); }
    T &operator *() const { return *m_lval; }
};

struct I_LexAttr
/*! Convenient base of all token types, terminals or non-terminals, for a specific parser framework.
*/
{
    virtual ~I_LexAttr() = default;
};

struct I_Parser
{
    // Pure virtuals
    virtual ~I_Parser() = default;
    virtual void add(T_LexID token, unsigned line, unsigned col, I_LexAttr *unownedAttr) = 0;
    virtual std::string_view setSource(std::string_view src) = 0;
};

template<class F_Pred>
class C_Screener: public I_Parser
{
public:

    // Nonvirtuals
    C_Screener(I_Parser &parser, F_Pred &&ignore): m_parser(parser), m_ignore(std::move(ignore)) {}

    // Implement I_Parser
    void add(T_LexID token, unsigned line, unsigned col, I_LexAttr *unownedAttr) override
    {
        if (m_ignore(token))
            // Skip this one
            delete unownedAttr;
        else
            m_parser.add(token, line, col, unownedAttr);
    }
    std::string_view setSource(std::string_view src) override
    {
        return m_parser.setSource(src);
    }

private:

    // Data
    I_Parser        &m_parser;
    const F_Pred    m_ignore;
};

template<T_LexID IGNORED>
struct C_ScreenerNo: C_Screener<bool(*)(T_LexID)>
{
    constexpr explicit C_ScreenerNo(I_Parser &parser): C_Screener<bool(*)(T_LexID)>(parser, *[](T_LexID token){ return token == IGNORED; }) {}
};

template<class T_Data> requires (!std::is_base_of_v<I_LexAttr,T_Data>)
struct C_LexDataT: I_LexAttr
/*! \brief Render any copyable "type" token attribute on the fly
*/
{
    using value_type = std::remove_cv_t<std::remove_reference_t<T_Data>>;

    // Data
    value_type  m_data;

    // Ctor
    template<class...T_Args>
    explicit constexpr C_LexDataT(T_Args&&...args): m_data(std::forward<T_Args>(args)...) {}
};

class C_IntegerLex: public I_LexAttr
{
public:

    // Nonvirtuals
    C_IntegerLex(std::string_view numstr, int _radix) noexcept;
    void negate();
    void prependPlus();
    auto radix() const noexcept { return m_radix; }
    auto &str() const noexcept  { return m_numStr; }
    template<class T> requires std::is_arithmetic_v<T>
    auto value() const
    {
        const auto t = value_();
        const auto ret = static_cast<T>(t);
        if (ret != t)
            RUNTIME_ERROR("Cast overflow {} != {}", ret, t);

        return ret;
    }

private:

    // Data
    std::string     m_numStr;
    const int       m_radix;

    // Nonvirtuals
    long long value_() const;
};

//
//      Externals
//
bool operator==(const C_SourcePos &a, const C_SourcePos &b) noexcept;
    ///< Equivalence relation
bool operator<(const C_SourcePos &a, const C_SourcePos &b) noexcept;
    ///< Partial relation

std::string asciiLiteral(uint32_t utf32);
std::string asciiLiteral(std::string_view utf8);

void addAsHex(std::string &dst, uint32_t utf32);

//
//      Function Templates
//
template<class T>
auto createLex(const T &t)
{
    return new C_LexDataT<T>(t);
}

template<class T>
auto createLex(T &t)
{
    return new C_LexDataT<T>(std::move(t));
}

template<class T, class...T_Args>
auto createLex(T_Args &&...args)
{
    return new C_LexDataT<T>(std::forward<T_Args>(args)...);
}

} //namespace bux
