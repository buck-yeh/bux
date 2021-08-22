- Supplemental static library of whatever is seen required in sense of general purpose but is not directly supported from [Modern C++](https://www.modernescpp.com/index.php/what-is-modern-c). Or whatever is deemed reusable from my side projects. 

- The library uses [fmt library](https://github.com/fmtlib/fmt) heavily as long as [C++20 \<format\>](https://en.cppreference.com/w/cpp/utility/format) is not there yet.

- 💡 There is also Doxygen-generated API reference [here](https://buck-yeh.github.io/bux/html/index.html) but Doxygen has been known for being insensitive to Modern C++ for so many years. Keywords like **concept**, any of attributes, ... etc can be misinterpreted or simply dropped. Viewer's discretion is advised.

# Table of Contents
   * [Installation &amp; Usage](#installation--usage)
      * [in ArchLinux](#in-archlinux)
      * [from github in any of Linux distros](#from-github-in-any-of-linux-distros)
   * [Header Intros](#header-intros)
      * [Containers](#containers)
      * [Input/Output](#inputoutput)
      * [Logger](#logger)
      * [Parser/scanner related](#parserscanner-related)
      * [System](#system)
      * [Thread Safety](#thread-safety)
      * [Misc.](#misc)

*(Created by [gh-md-toc](https://github.com/ekalinin/github-markdown-toc))*

# Installation & Usage
## in [ArchLinux](https://archlinux.org/)
1. Make sure you have installed [`yay`](https://aur.archlinux.org/packages/yay/) or any other [pacman wrapper](https://wiki.archlinux.org/index.php/AUR_helpers)
2. ~~~bash
   yay -Ss bux
   ~~~
3. To define target executable `foo` using `bux` in `CMakeLists.txt`
   ~~~cmake
   add_executable(foo foo.cpp)
   target_compile_options(foo PRIVATE -std=c++2a)
   target_link_libraries(foo bux fmt)
   ~~~
4. Include the header files by prefixing header name with `bux/`, for example:
   ~~~c++
   #include <bux/StrUtil.h>
   ~~~
   *p.s.* Header files are in `/usr/include/bux` and compiler is expected to search `/usr/include` by default.
5. If directly using `gcc` or `clang` is intended, the required compiler flags are `-std=c++2a -lbux`
## from github in any of [Linux distros](https://distrowatch.com/)
1. Make sure you have installed `cmake` `make` `gcc` `git` `fmt`
2. ~~~bash
   git clone -b main --single-branch https://github.com/buck-yeh/bux.git .
   cmake .
   make -j
   BUX_DIR="/full/path/to/current/dir"
   ~~~
   *p.s.* You can install a tagged version by replacing `main` with [tag name](https://github.com/buck-yeh/bux/tags).
3. To define target executable `foo` using `bux` in `CMakeLists.txt`
   ~~~cmake
   add_executable(foo foo.cpp)
   target_compile_options(foo PRIVATE -std=c++2a)
   target_include_directories(foo PRIVATE "$env{BUX_DIR}/include") 
   target_link_directories(foo PRIVATE "$env{BUX_DIR}/src") 
   target_link_libraries(foo bux fmt)
   ~~~
4. Include the header files by prefixing header name with `bux/`, for example:
   ~~~c++
   #include <bux/StrUtil.h>
   ~~~
5. If directly using command `gcc` or `clang` is intended, the required compiler flags are `-std=c++2a -I$BUX_DIR/include -L$BUX_DIR/src -lbux`
6. Subdirectory `test/` is excluded by default. To build with it, reconfigure `cmake` with:
   ~~~bash
   rm CMakeCache.txt
   cmake . -DBUILD_TEST=1
   make
   ~~~
   And test all of them:
   ~~~bash
   cd test
   ctest .
   ~~~

# Header Intros
## Containers
* [Intervals.h](include/bux/Intervals.h) - `std::C_Intervals<T>` defines its own arithmetics but is currently ever used by [`scannergen`](https://github.com/buck-yeh/parsergen/tree/main/ScannerGen)
* [PartialOrdering.h](include/bux/PartialOrdering.h) - Define [partial ordering](https://en.wikipedia.org/wiki/Partially_ordered_set) as container in order to generate a compatible [linear ordering](https://en.wikipedia.org/wiki/Total_order). 
* [XQue.h](include/bux/XQue.h) - An efficient generic queue. 
* [Xtack.h](include/bux/Xtack.h) - Generic stack types. 

## Input/Output
* [EZArgs.h](include/bux/EZArgs.h) - Inspired by Python [argparse.ArgumentParser](https://docs.python.org/3/library/argparse.html#argumentparser-objects) with interfaces making sense to Modern C++
* [LogStream.h](include/bux/LogStream.h) - Marginal utilities for `std::ostream`, used by loggers & exception messages.
* [MemIn.h](include/bux/MemIn.h) - Drop-in replacement of C++98-deprecated [`std::istrstream`](https://en.cppreference.com/w/cpp/io/istrstream).
* [MemOut.h](include/bux/MemOut.h) - Drop-in replacement of C++98-deprecated [`std::ostrstream`](https://en.cppreference.com/w/cpp/io/ostrstream). *(Not used recently)*
* [Serialize.h](include/bux/Serialize.h) - Simple functions to define serialization/deserialization in a symmetric way.
* [StrUtil.h](include/bux/StrUtil.h) - String utilities mostly related to input (parsing).
* [UnicodeCvt.h](include/bux/UnicodeCvt.h) - Encode text stream to unicodes (`utf8`/`utf16`/`utf32`)

## Logger
* [FileLog.h](include/bux/FileLog.h) - Snap object of file log which can be configured to automatically change the output path, *iow* to output to different files, according to the current timestamp. The object is a plugin to [ParaLog.h](include/bux/ParaLog.h) and [SyncLog.h](include/bux/SyncLog.h)
* [Logger.h](include/bux/Logger.h) - Log macros for various needs with *singleton* `bux::logger()` in mind.
* [LogLevel.h](include/bux/LogLevel.h) - LL_FATAL, LL_ERROR, LL_WARNING, LL_INFO, LL_VERBOSE
* [ParaLog.h](include/bux/ParaLog.h) - Logger facade to reroute log lines to child loggers 
* [SyncLog.h](include/bux/SyncLog.h) - Basic classes to give variety of *thread-safe* loggers.

## Parser/scanner related
* [FA.h](include/bux/FA.h) - Supports to finite automaton, *aka* finite state machine, *aka* regular expression, emphasizing on minimizing [NFA](https://en.wikipedia.org/wiki/Nondeterministic_finite_automaton) into [DFA](https://en.wikipedia.org/wiki/Deterministic_finite_automaton).
* [GLR.h](include/bux/GLR.h) - Implementation of [**G**eneralized **LR** parser](https://en.wikipedia.org/wiki/GLR_parser)
* [ImplGLR.h](include/bux/ImplGLR.h) - Stuffs constantly needed by [`parsergen`](https://github.com/buck-yeh/parsergen/tree/main/ParserGen)-generated *.cpp files for syntaxes classified as GLR.
* [ImplLR1.h](include/bux/ImplLR1.h) - Stuffs constantly needed by [`parsergen`](https://github.com/buck-yeh/parsergen/tree/main/ParserGen)-generated *.cpp files for syntaxes classified as LR1.
* [ImplScanner.h](include/bux/ImplScanner.h) - Generic implementation of scanner, *aka* [lexical analyzer](https://en.wikipedia.org/wiki/Lexical_analysis), mainly used by [`scannergen`](https://github.com/buck-yeh/parsergen/tree/main/ScannerGen)
* [LexBase.h](include/bux/LexBase.h) - Basic supports to create [lexical tokens](https://en.wikipedia.org/wiki/Lexical_analysis#Token) and parsers. 
* [LR1.h](include/bux/LR1.h) - Implementation of [**LR1** parser](https://en.wikipedia.org/wiki/Canonical_LR_parser)
* [ParserBase.h](include/bux/ParserBase.h) - Common supports to all parsers.
* [Range2Type.h](include/bux/Range2Type.h) - `bux::fittestType()` called by [`parsergen`](https://github.com/buck-yeh/parsergen/tree/main/ParserGen) & [`scannergen`](https://github.com/buck-yeh/parsergen/tree/main/ScannerGen).
* [ScannerBase.h](include/bux/ScannerBase.h) - Generic supports to all scanners.

## System
* [FsUtil.h](include/bux/FsUtil.h) - Utilities solely related to [\<filesystem\>](https://en.cppreference.com/w/cpp/header/filesystem)
* [XConsole.h](include/bux/XConsole.h) - Cross-platform console functions.
* [XPlatform.h](include/bux/XPlatform.h) - Most possibly macros & typedefs defined in per-platform fashions.

## Thread Safety
* [AtomiX.h](include/bux/AtomiX.h) - Spin lock on [`std::atomic_flag`](https://en.cppreference.com/w/cpp/atomic/atomic_flag) & a mapping cache type using it.

## Misc.
* [ParaUtil.h](include/bux/ParaUtil.h) - Iterator type for sheer numbers, expected to be used when calling parallel algorithms, *e.g.* `std::for_each(std::execution::par, ...)` without data allocation.
* [SafeArith.h](include/bux/SafeArith.h) - Supports to safe arithmetics. *(Not used recently)*
* [XAutoPtr.h](include/bux/XAutoPtr.h) - Safe [`std::auto_ptr`](https://en.cppreference.com/w/cpp/memory/auto_ptr) dated back to pre-C++11 years. *It ain't broke ...*
* [XException.h](include/bux/XException.h) - Macros to throw `std::runtime_error`, `std::logic_error`, as well as [other exceptions](https://en.cppreference.com/w/cpp/header/stdexcept), with location & message.
