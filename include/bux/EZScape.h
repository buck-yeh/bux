#pragma once

#include <string>       // std::string
#include <string_view>  // std::string_view

namespace bux {

//
//      Externals
//
std::string easy_escape(std::string_view src);
std::string easy_unescape(std::string_view src);

} //namespace bux
