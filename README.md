- `bux` is not Buck's. It could be box!

- Supplemental static library of whatever are seen required in sense of general purpose but not directly supported from [Modern C++](https://www.modernescpp.com/index.php/what-is-modern-c). Or whatever is deemed reusable from my side projects. 

- Doxygen-generated API reference is [here](https://buck-yeh.github.io/bux/html/index.html). 💡 Doxygen has been known for being insensitive to Modern C++ for so many years. Keywords like any of attributes, ... etc can be misinterpreted or simply dropped. Viewer's discretion is advised.

---
**Table of contents**
<!-- TOC -->

- [In ArchLinux](#in-archlinux)
- [Build from github in MacOS or any of Linux distros](#build-from-github-in-macos-or-any-of-linux-distros)
- [From vcpkg in Windows](#from-vcpkg-in-windows)
- [Header Intros](#header-intros)
    - [If you target iOS 16.0 or earlier](#if-you-target-ios-160-or-earlier)
    - [Containers](#containers)
    - [Input/Output](#inputoutput)
    - [Logger](#logger)
    - [Parser/scanner related](#parserscanner-related)
    - [System](#system)
    - [Thread Safety](#thread-safety)
    - [Misc.](#misc)

<!-- /TOC -->

## In [ArchLinux](https://archlinux.org/)

1. Make sure you have installed [`yay`](https://aur.archlinux.org/packages/yay/) or any other [pacman wrapper](https://wiki.archlinux.org/index.php/AUR_helpers)

2. ~~~bash
   yay -S bux
   ~~~

3. To define target executable `foo` using `bux` in `CMakeLists.txt`

   ~~~cmake
   add_executable(foo foo.cpp)
   target_compile_options(foo PRIVATE -std=c++2a)
   target_link_libraries(foo bux)
   ~~~

4. Include the header files by prefixing header name with `bux/`, for example:

   ~~~c++
   #include <bux/Logger.h>
   ~~~

   *p.s.* Header files are in `/usr/include/bux` and compiler is expected to search `/usr/include` by default.
5. If directly using `gcc` or `clang` is intended, the required compiler flags are `-std=c++23 -lbux`

## Build from github in MacOS or any of [Linux distros](https://distrowatch.com/)

1. Make sure you have installed the following tools or the likes:
   * `cmake`(3.18 or newer)
   * `make`
   * `gcc`(13 or newer) or `clang`(usually thru installing xcode on MacOS)
   * `git`

2. ~~~bash
   git clone https://github.com/buck-yeh/bux.git
   cd bux
   cmake .
   make -j
   BUX_DIR="/full/path/to/current/dir"
   ~~~

   *p.s.* You can install a tagged version by replacing `main` with [tag name](https://github.com/buck-yeh/bux/tags).
3. To define target executable `foo` using `bux` in `CMakeLists.txt`

   ~~~cmake
   add_executable(foo foo.cpp)
   target_compile_features(foo PRIVATE cxx_std_23)
   target_include_directories(foo PRIVATE "$env{BUX_DIR}/include") 
   target_link_directories(foo PRIVATE "$env{BUX_DIR}/src") 
   target_link_libraries(foo bux)
   ~~~

4. Include the header files by prefixing header names with `bux/`, for example:

   ~~~c++
   #include <bux/Logger.h>
   ~~~

5. If directly using command `gcc` or `clang` is intended, the required compiler flags are `-std=c++23 -I$BUX_DIR/include -L$BUX_DIR/src -lbux`
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

## From vcpkg in Windows

1. ~~~PowerShell
   PS F:\vcpkg> .\vcpkg.exe search bux
   buck-yeh-bux             1.11.0           A supplemental C++ library with functionalities not directly supported fro...
   buck-yeh-bux-sqlite      1.0.5            Modern C++ wrapper classes and utilities of the original sqlite3 API
   The result may be outdated. Run `git pull` to get the latest results.
   If your port is not listed, please open an issue at and/or consider making a pull request.  -  https://github.com/Microsoft/vcpkg/issues
   PS F:\vcpkg>
   ~~~
2. Install `buck-yeh-bux` with triplets you needed. For example:

   ~~~PowerShell
   PS F:\vcpkg> .\vcpkg.exe install buck-yeh-bux:x64-windows-static
   ~~~

3. Include the header files by prefixing header name with `bux/`, for example:

   ~~~c++
   #include <bux/Logger.h>
   ~~~

## Header Intros

### If you target iOS 16.0 or earlier
💡 The following headers and other headers using them should be avoided to use because they call [std::format()](https://en.cppreference.com/w/cpp/utility/format/format) directly or indirectly, which in turn calls the floating point version of [std::to_chars()](https://en.cppreference.com/w/cpp/utility/to_chars), which wasn't ready in AppleClang at the advent C++17 for quite a few years.

- [EZScape.h](include/bux/EZScape.h)
- [ImplScanner.h](include/bux/ImplScanner.h)
- [Logger.h](include/bux/Logger.h)
- [XException.h](include/bux/XException.h)

### Containers

- [Intervals.h](include/bux/Intervals.h) - `std::C_Intervals<T>` defines its own arithmetics but is currently ever used by [`scannergen`](https://github.com/buck-yeh/parsergen/tree/main/ScannerGen)
- [PartialOrdering.h](include/bux/PartialOrdering.h) - Define [partial ordering](https://en.wikipedia.org/wiki/Partially_ordered_set) as container in order to generate a compatible [linear ordering](https://en.wikipedia.org/wiki/Total_order).
- [XQue.h](include/bux/XQue.h) - An efficient generic queue.
- [Xtack.h](include/bux/Xtack.h) - Generic stack types.

### Input/Output

- [EZArgs.h](include/bux/EZArgs.h) - Inspired by Python [argparse.ArgumentParser](https://docs.python.org/3/library/argparse.html#argumentparser-objects) with interfaces making sense to Modern C++
- [LogStream.h](include/bux/LogStream.h) - Fundation functions for `std::ostream`, used indirectly by logger macros such as [`LOG()`](https://buck-yeh.github.io/bux/html/Logger_8h.html#ac1de67d40c06ffbf5dbe628a2f25e928), ...
- [MemIn.h](include/bux/MemIn.h) - Drop-in replacement of C++98-deprecated [`std::istrstream`](https://en.cppreference.com/w/cpp/io/istrstream).
- [MemOut.h](include/bux/MemOut.h) - Drop-in replacement of C++98-deprecated [`std::ostrstream`](https://en.cppreference.com/w/cpp/io/ostrstream). *(Not used recently)*
- [Serialize.h](include/bux/Serialize.h) - Simple functions to define serialization/deserialization in a symmetric way.
- [StrUtil.h](include/bux/StrUtil.h) - String utilities.
- [UnicodeCvt.h](include/bux/UnicodeCvt.h) - Encode text stream to unicodes (`utf8`/`utf16`/`utf32`)

### Logger

- [FileLog.h](include/bux/FileLog.h) - [`bux::C_PathFmtLogSnap`](https://buck-yeh.github.io/bux/html/classbux_1_1C__PathFmtLogSnap.html) can be configured to automatically change the output path, *IOW* to output to different files, according to the current timestamp. The object is a plugin to `bux::C_ReenterableOstreamSnap` and `bux::C_ParaLog`
- [Logger.h](include/bux/Logger.h) - Log macros for various needs with *singleton* `bux::logger()` in mind.
- [LogLevel.h](include/bux/LogLevel.h) - LL_FATAL, LL_ERROR, LL_WARNING, LL_INFO, LL_DEBUG, LL_VERBOSE
- [ParaLog.h](include/bux/ParaLog.h) - [`bux::C_ParaLog`](https://buck-yeh.github.io/bux/html/classbux_1_1C__ParaLog.html) is a logger facade to reroute log lines to multiple child loggers 
- [SyncLog.h](include/bux/SyncLog.h) - Basic classes to give variety of *thread-safe* loggers.

### Parser/scanner related

- [FA.h](include/bux/FA.h) - Supports to finite automaton, *aka* finite state machine, *aka* regular expression, emphasizing on minimizing [NFA](https://en.wikipedia.org/wiki/Nondeterministic_finite_automaton) into [DFA](https://en.wikipedia.org/wiki/Deterministic_finite_automaton).
- [GLR.h](include/bux/GLR.h) - Implementation of [**G**eneralized **LR** parser](https://en.wikipedia.org/wiki/GLR_parser)
- [ImplGLR.h](include/bux/ImplGLR.h) - Stuffs constantly needed by [`parsergen`](https://github.com/buck-yeh/parsergen/tree/main/ParserGen)-generated *.cpp files for syntaxes classified as GLR.
- [ImplLR1.h](include/bux/ImplLR1.h) - Stuffs constantly needed by [`parsergen`](https://github.com/buck-yeh/parsergen/tree/main/ParserGen)-generated *.cpp files for syntaxes classified as LR1.
- [ImplScanner.h](include/bux/ImplScanner.h) - Generic implementation of scanner, *aka* [lexical analyzer](https://en.wikipedia.org/wiki/Lexical_analysis), mainly used by [`scannergen`](https://github.com/buck-yeh/parsergen/tree/main/ScannerGen)
- [LexBase.h](include/bux/LexBase.h) - Basic supports to create [lexical tokens](https://en.wikipedia.org/wiki/Lexical_analysis#Token) and parsers. 
- [LR1.h](include/bux/LR1.h) - Implementation of [**LR1** parser](https://en.wikipedia.org/wiki/Canonical_LR_parser)
- [ParserBase.h](include/bux/ParserBase.h) - Common supports to all parsers.
- [Range2Type.h](include/bux/Range2Type.h) - `bux::fittestType()` called by [`parsergen`](https://github.com/buck-yeh/parsergen/tree/main/ParserGen) & [`scannergen`](https://github.com/buck-yeh/parsergen/tree/main/ScannerGen).
- [ScannerBase.h](include/bux/ScannerBase.h) - Generic supports to all scanners.

### System

- [FsUtil.h](include/bux/FsUtil.h) - Utilities solely related to [\<filesystem\>](https://en.cppreference.com/w/cpp/header/filesystem)
- [XConsole.h](include/bux/XConsole.h) - Cross-platform console functions.
- [XPlatform.h](include/bux/XPlatform.h) - Conditionally defined macros, types, functions for as many platforms as possible.

### Thread Safety

- [AtomiX.h](include/bux/AtomiX.h) - Spin lock on [`std::atomic_flag`](https://en.cppreference.com/w/cpp/atomic/atomic_flag) & a mapping cache type using it.

### Misc.

- [EZScape.h](include/bux/EZScape.h) - Replacement of [`curl_easy_escape()`](https://curl.se/libcurl/c/curl_easy_escape.html) & [`curl_easy_unescape()`](https://curl.se/libcurl/c/curl_easy_unescape.html) in `libcurl`.
- [SafeArith.h](include/bux/SafeArith.h) - Supports to safe arithmetics. *(Not used recently)*
- [XAutoPtr.h](include/bux/XAutoPtr.h) - Safe [`std::auto_ptr`](https://en.cppreference.com/w/cpp/memory/auto_ptr) dated back to pre-C++11 years. *It ain't broke ...*
- [XException.h](include/bux/XException.h) - Macros to throw `std::runtime_error`, `std::logic_error`, as well as [other exceptions](https://en.cppreference.com/w/cpp/header/stdexcept), with location & formatted message.
