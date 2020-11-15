#include "Serialize.h"
#include <functional>   // std::hash<>
#include <iterator>     // std::istreambuf_iterator
#include <istream>      // std::istream
#include <ostream>      // std::ostream

namespace bux {

//
//      Implement Functions
//
size_t read_size(const std::string &src, size_t &off) noexcept
{
    size_t n;
    read(src, off, n);
    return n;
}

std::tuple<std::string,size_t,bool> load_hashed_str(std::istream &in)
{
    size_t last_hash;
    in.read(reinterpret_cast<char*>(&last_hash), sizeof last_hash);
    const std::string src{std::istreambuf_iterator<char>{in}, {}};
    return {src, last_hash, std::hash<std::string>{}(src) == last_hash};
}

size_t save_hashed_str(std::ostream &out, const std::string &s)
{
    size_t n = std::hash<std::string>{}(s);
    out.write(reinterpret_cast<char*>(&n), sizeof n) <<s;
    return n;
}

} // namespace bux
