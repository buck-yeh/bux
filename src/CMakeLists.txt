string(APPEND CMAKE_CXX_FLAGS " -Wall -Wextra -Wshadow -Wconversion -Wno-parentheses -Wno-unknown-pragmas -g3 -O3 -std=c++2a")
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        message("Clang")
        string(APPEND CMAKE_CXX_FLAGS " -Wno-potentially-evaluated-expression")
else()
        message("Not Clang")
endif()

add_library(bux STATIC
        AtomiX.cpp
        EZArgs.cpp
        FA.cpp FileLog.cpp
        LexBase.cpp Logger.cpp LogStream.cpp LR1.cpp GLR.cpp ParserBase.cpp
        ScannerBase.cpp Serialize.cpp StrUtil.cpp SyncStream.cpp
        UnicodeCvt.cpp
        XConsole.cpp
        XException.cpp
        )
target_include_directories(bux PRIVATE ../include/bux)
