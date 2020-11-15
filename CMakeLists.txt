cmake_minimum_required (VERSION 2.8.1)
cmake_policy(VERSION 2.8.1)

#set(CMAKE_CXX_COMPILER "clang++")

list(APPEND CMAKE_CXX_FLAGS "-Wall -Wextra -Wshadow -Wconversion -Wno-parentheses -Wno-unknown-pragmas -g3 -O3 -std=c++2a")

add_library(Bux STATIC
        AtomiX.cpp
        EZArgs.cpp
        FA.cpp FileLog.cpp
        LexBase.cpp Logger.cpp LogStream.cpp LR1.cpp GLR.cpp
        ScannerBase.cpp Serialize.cpp StrUtil.cpp SyncStream.cpp
        UnicodeCvt.cpp
        XConsole.cpp
        XException.cpp
        )
