#ifndef bux_ParaUtil_H_
#define bux_ParaUtil_H_

#include <iterator>     // std::iterator<>, std::random_access_iterator_tag
#include <type_traits>  // std::make_signed_t<>

namespace bux {

//
//      Types
//
template<typename T_Num, typename T_Dist = std::make_signed_t<T_Num>>
class C_NumIter: public std::iterator<std::random_access_iterator_tag, T_Num, T_Dist>
{
public:

    // Nonvirtuals
    constexpr C_NumIter() = default;
    constexpr explicit C_NumIter(T_Num ord): m_ord(ord) {}
    T_Num operator*() const { return m_ord; }
    T_Num operator[](T_Dist d) const { return m_ord + d; }
    constexpr bool operator<(const C_NumIter  &lhs) const { return m_ord < lhs.m_ord; }
    constexpr bool operator==(const C_NumIter &lhs) const { return m_ord == lhs.m_ord; }
    C_NumIter &operator++() { ++m_ord; return *this; }
    auto operator+(T_Dist d) const { return C_NumIter{m_ord + d}; }
    T_Dist operator-(const C_NumIter &lhs) const { return m_ord - lhs.m_ord; }

private:

    // Number
    T_Num       m_ord;
};

} //namespace bux

#endif // bux_ParaUtil_H_
