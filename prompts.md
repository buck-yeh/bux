<!-- TOC -->

- [/08/23 Claude Code Opus 5 / Max](#0823-claude-code-opus-5--max)
- [/08/16 Claude Code Opus 5 / Max](#0816-claude-code-opus-5--max)

<!-- /TOC -->

## 2026/08/23 Claude Code Opus 5 / Max

Summarize usage of `include/logger.h` in `doc/tldr-logger.h`

## 2026/08/16 Claude Code Opus 5 / Max

`test/test_lexbase.cpp` id complained by CXX
~~~
/home/buck/github/bux/test/test_strutil.cpp: In function ‘void CATCH2_INTERNAL_TEST_0()’:
/home/buck/github/bux/test/test_strutil.cpp:11:15: error: no matching function for call to ‘split(const char [1], const char [1], CATCH2_INTERNAL_TEST_0()::<lambda(auto:14)>)’
   11 |     bux::split("", "", [&](auto token){ tokens.emplace_back(token); });
      |     ~~~~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  • there is 1 candidate
In file included from /home/buck/github/bux/test/test_strutil.cpp:5:
    • candidate 1: ‘template<class T, class auto:6, class auto:7>  requires (regular_invocable<auto:6, std::basic_string_view<T, std::char_traits<_CharT> > >) && (regular_invocable<auto:7, std::basic_string_view<T, std::char_traits<_CharT> > >) void bux::split(std::basic_string_view<T>, std::basic_string_view<T>, auto:6&&, auto:7&&)’
      /home/buck/github/bux/test/../include/bux/StrUtil.h:22:6:
         22 | void split(std::basic_string_view<T> token_list, std::basic_string_view<T> splitters,
            |      ^~~~~
      • template argument deduction/substitution failed:
        •   mismatched types ‘std::basic_string_view<T>’ and ‘const char*’
          /home/buck/github/bux/test/test_strutil.cpp:11:15:
             11 |     bux::split("", "", [&](auto token){ tokens.emplace_back(token); });
                |     ~~~~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/home/buck/github/bux/test/test_strutil.cpp:14:15: error: no matching function for call to ‘split(const char [1], const char [1], CATCH2_INTERNAL_TEST_0()::<lambda(auto:15)>, CATCH2_INTERNAL_TEST_0()::<lambda(auto:16)>)’
   14 |     bux::split("", "",
      |     ~~~~~~~~~~^~~~~~~~
   15 |         [&](auto token){ tokens.emplace_back(token); },
      |         ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   16 |         [&](auto delim){ delims.emplace_back(delim); });
      |         ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  • there is 1 candidate
    • candidate 1: ‘template<class T, class auto:6, class auto:7>  requires (regular_invocable<auto:6, std::basic_string_view<T, std::char_traits<_CharT> > >) && (regular_invocable<auto:7, std::basic_string_view<T, std::char_traits<_CharT> > >) void bux::split(std::basic_string_view<T>, std::basic_string_view<T>, auto:6&&, auto:7&&)’
      /home/buck/github/bux/test/../include/bux/StrUtil.h:22:6:
         22 | void split(std::basic_string_view<T> token_list, std::basic_string_view<T> splitters,
            |      ^~~~~
      • template argument deduction/substitution failed:
        •   mismatched types ‘std::basic_string_view<T>’ and ‘const char*’
          /home/buck/github/bux/test/test_strutil.cpp:14:15:
             14 |     bux::split("", "",
                |     ~~~~~~~~~~^~~~~~~~
             15 |         [&](auto token){ tokens.emplace_back(token); },
                |         ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
             16 |         [&](auto delim){ delims.emplace_back(delim); });
                |         ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~

How to fix it?