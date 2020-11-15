#ifndef SerializeH
#define SerializeH

#include <array>        // std::array<>
#include <cstring>      // memcpy()
#include <iosfwd>       // Forwarded std::istream, std::ostream
#include <string>       // std::string
#include <tuple>        // std::tuple<>
#include <type_traits>  // std::is_standard_layout_v<>

namespace bux {

//
//      Functions
//
template<class T>
void append(const T &src, std::string &dst)
{
    static_assert(std::is_standard_layout_v<T>);
    dst.append(reinterpret_cast<const char*>(&src), sizeof src);
}

template<class T>
void append_size_of(const T &src, std::string &dst)
{
    append(std::size(src), dst);
}

template<class T>
void append(const T *src, size_t argN, std::string &dst)
{
    static_assert(std::is_standard_layout_v<T>);
    static_assert(sizeof(T[10]) == sizeof(T) * 10);
    dst.append(reinterpret_cast<const char*>(src), sizeof(*src)*argN);
}

template<class T>
void read(const std::string &src, size_t &off, T &data) noexcept
{
    static_assert(std::is_standard_layout_v<T>);
    data = *reinterpret_cast<const T*>(src.data() + off);
    off += sizeof data;
}

size_t read_size(const std::string &src, size_t &off) noexcept;

template<class T>
void read(const std::string &src, size_t &off, T *data, size_t count)
{
    static_assert(std::is_standard_layout_v<T>);
    const auto bytes = sizeof(T) * count;
    memcpy(data, src.data()+off, bytes);
    off += bytes;
}

template<size_t N>
auto to_str(const std::array<char,N> &arr)
{
    return std::string{arr.data(), arr.size()};
}

template<size_t N>
std::array<char,N> read_charr(const std::string &src, size_t &off) noexcept
{
    std::array<char,N> ret;
    memcpy(ret.data(), src.data()+off, N);
    off += N;
    return ret;
}

template<size_t N, template<typename> class C>
void append(const C<std::array<char,N>> &src, std::string &dst)
{
    append_size_of(src, dst);
    if constexpr (N <= 8)
    {
        for (auto i: src)
            dst += to_str(i);
    }
    else
    {
        for (auto &i: src)
            dst += to_str(i);
    }
}

template<size_t N, template<typename> class C>
void read(const std::string &src, size_t &off, C<std::array<char,N>> &data)
{
    for (size_t i = 0, n = read_size(src,off); i < n; ++i)
        data.emplace_back(read_charr<N>(src, off));
}

std::tuple<std::string,size_t,bool> load_hashed_str(std::istream &in);
size_t save_hashed_str(std::ostream &out, const std::string &s);

} // namespace bux

#endif // SerializeH
