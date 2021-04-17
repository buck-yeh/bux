#pragma once

#include "LexBase.h"    // bux::T_LexID, bux::I_LexAttr, bux::C_IntegerLex, bux::TID_EOF
#include "UnicodeCvt.h" // bux::C_UnicodeIn
#ifdef _WIN32
    #include <ctype.h>  // __isascii()
#else
    #include <wchar.h>  // wcwidth()
#endif

namespace bux {

//
//      Types
//
template<class T_Char>
struct I_Scanner
{
    // Pure virtuals
    virtual ~I_Scanner() = default;
    virtual void add(unsigned col, T_Char c) = 0;
    virtual void setLine(unsigned line) = 0;
    virtual void setSource(std::string_view src) = 0;
};

struct C_ActionRet
{
    T_LexID                 m_id;
    I_LexAttr               *m_pAttr;   ///< newed

    constexpr C_ActionRet(T_LexID id, I_LexAttr *unownedAttr = nullptr): m_id(id), m_pAttr(unownedAttr)
        {}
    C_ActionRet(): m_pAttr(nullptr)
        {}
};

template<class T_LexCh>
struct C_LexTraits
{
    static void appendUTF8(std::string &u8s, const T_LexCh &ch);
    static unsigned columnsInDisplay(const T_LexCh &ch);
    static T_LexID id(const T_LexCh &ch);
    static bool read(C_UnicodeIn &uin, T_LexCh &ch);
    static void setId(T_LexCh &ch, T_LexID id);
};

struct C_LexUTF32 { uint32_t m_U32; };

template<>
struct C_LexTraits<C_LexUTF32>
{
    static void appendUTF8(std::string &u8, C_LexUTF32 src)
    {
        u8 += to_utf8(src.m_U32);
    }
    static unsigned columnsInDisplay(C_LexUTF32 ch) noexcept
    {
#ifdef _WIN32
        return __isascii(int(ch.m_U32)) ?1U :2U;
#else
        return (unsigned)wcwidth(wchar_t(ch.m_U32));
#endif
    }
    static constexpr auto id(C_LexUTF32 ch) noexcept
    {
        return ch.m_U32;
    }
    static bool read(C_UnicodeIn &uin, C_LexUTF32 &ch)
    {
        return uin.get(ch.m_U32) > 0;
    }
    static void setId(C_LexUTF32 &ch, T_LexID id) noexcept
    {
        ch.m_U32 = id;
    }
};

//
//      Externals
//
[[nodiscard]]
std::string escseq2str(std::string);
[[nodiscard]]
bool isIdentifier(std::string_view s) noexcept;
[[nodiscard]]
size_t parseEscapeChar(std::string_view s, uint32_t &c, size_t pos =0);
[[nodiscard]]
size_t skipIdentifier(std::string_view s, size_t pos) noexcept;

//
//      Function Templates
//
template<class T_LexCh>
[[nodiscard]] auto toString(const T_LexCh *c, size_t start, size_t end) noexcept(noexcept(
    C_LexTraits<T_LexCh>::appendUTF8(std::declval<std::string&>(), T_LexCh())))
{
    std::string buf;
    for (size_t i = start; i < end; C_LexTraits<T_LexCh>::appendUTF8(buf, c[i++]));
    return buf;
}

template<T_LexID _ID, class T_LexCh>
[[nodiscard]] auto createCharLiteral(const T_LexCh *c, size_t n)
{
    uint32_t key;
    const auto len = parseEscapeChar(toString(c,1,n-1), key);
    if (len + 2 != n)
        RUNTIME_ERROR("parseEscapeChar() returns {} != {}", len, n -2);

    return C_ActionRet{_ID, createLex(key)};
}

template<T_LexID _ID, class T_LexCh>
[[nodiscard]] auto createDecNum(const T_LexCh *c, size_t n)
{
    return C_ActionRet{_ID, new C_IntegerLex(toString(c,0,n), 10)};
}

template<T_LexID _ID, class T_LexCh>
[[nodiscard]] auto createHexNum(const T_LexCh *c, size_t n)
{
    return C_ActionRet{_ID, new C_IntegerLex(toString(c,0,n), 16)};
}

template<T_LexID _ID, class T_LexCh>
[[nodiscard]] C_ActionRet createNothing(const T_LexCh *, size_t)
{
    return _ID;
}

template<T_LexID _ID, class T_LexCh>
[[nodiscard]] auto createOctNum(const T_LexCh *c, size_t n)
{
    return C_ActionRet{_ID, new C_IntegerLex(toString(c,1,n), 8)};
}

template<T_LexID _ID, class T_LexCh, size_t TRIMLEFT = 0, size_t TRIMRIGHT = 0>
[[nodiscard]] auto createPlainString(const T_LexCh *c, size_t n)
{
    return C_ActionRet{_ID, createLex(toString(c, TRIMLEFT, n-TRIMRIGHT))};
}

template<T_LexID _ID, class T_LexCh, size_t TRIMLEFT = 0, size_t TRIMRIGHT = 0>
[[nodiscard]] auto createEscapeString(const T_LexCh *c, size_t n)
{
    return C_ActionRet{_ID, createLex(escseq2str(toString(c, TRIMLEFT, n-TRIMRIGHT)))};
}

template<class T_Char>
void scanFile(std::string_view filename, std::istream &in, I_Scanner<T_Char> &scanner, T_LexID endToken = TID_EOF)
{
    C_UnicodeIn     src(in);
    unsigned        line = 1, col = 1;
    T_Char          c;

    scanner.setSource(filename);
    scanner.setLine(line);

    typedef C_LexTraits<T_Char> C_Traits;

    while (C_Traits::read(src, c))
    {
        scanner.add(col, c);
        switch (C_Traits::id(c))
        {
        case '\n':  // New line
            scanner.setLine(++line);
            col = 1;
            break;
        case '\t':  // TAB
            col += 4 - (col - 1) % 4;
            break;
        default:
            col += C_Traits::columnsInDisplay(c);
        }
    }
    C_Traits::setId(c, endToken);
    scanner.add(col, c);
}

} //namespace bux
