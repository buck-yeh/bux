#ifndef bux_StrUtil_H_
#define bux_StrUtil_H_

#include <functional>   // std::function<>
#include <string>       // std::string
#include <string_view>  // std::string_view

namespace bux {

//
//      Types
//
typedef std::function<void(std::string_view)> FH_ApplyData;

struct FC_ParseNone
{
    // Data
    const FH_ApplyData  m_Apply;

    // Nonvirtuals
    FC_ParseNone(const FH_ApplyData &apply): m_Apply(apply) {}
    void operator()(const char *data, size_t n);
};

class FC_BufferedParse
{
public:

    // Nonvirtuals
    void operator()(const char *data, size_t n);

private:

    // Data
    std::string             m_YetToParse;

    // Pure virtuals
    virtual size_t parse(const char *data, size_t n) = 0;
};

class FC_ParseLine: public FC_BufferedParse
{
public:

    // Data
    const FH_ApplyData      m_Apply;
    const char              m_Delim;

    // Nonvirtuals
    template<typename APPLY>
    FC_ParseLine(APPLY &&apply, char delimeter ='\n'): m_Apply(std::forward<APPLY>(apply)), m_Delim(delimeter) {}

protected:

    // Implement FC_BufferedParse
    size_t parse(const char *data, size_t n) override;
};

class FC_ParseCRLF: public FC_BufferedParse
{
public:

    // Data
    const FH_ApplyData m_Apply;

    // Nonvirtuals
    template<typename APPLY>
    FC_ParseCRLF(APPLY &&apply): m_Apply(std::forward<APPLY>(apply)) {}

protected:

    // Implement FC_BufferedParse
    size_t parse(const char *data, size_t n) override;
};

//
//      Externs
//
const char *ord_suffix(size_t i);
std::string expand_env(const char *s);

} // namespace bux

#endif  // bux_StrUtil_H_
