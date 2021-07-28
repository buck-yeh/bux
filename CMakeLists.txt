cmake_minimum_required (VERSION 3.18)
cmake_policy(VERSION 3.18)
project(bux)

set(GNU_LIKE_CXX_FLAGS " -Wall -Wextra -Wshadow -Wconversion -Wno-parentheses -Wno-unknown-pragmas -g3 -Og -std=c++2a")
#string(APPEND CMAKE_CXX_FLAGS " -g3")

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message("Clang")
    string(APPEND CMAKE_CXX_FLAGS "${GNU_LIKE_CXX_FLAGS} -Wno-potentially-evaluated-expression")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message("GNU")
    string(APPEND CMAKE_CXX_FLAGS "${GNU_LIKE_CXX_FLAGS}")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message("MSVC")
    string(APPEND CMAKE_CXX_FLAGS " /Zc:__cplusplus /std:c++latest /MP")
else()
    message("Unknown compiler")
endif()

find_package(fmt CONFIG REQUIRED)

add_subdirectory (src)

if(${BUILD_TEST})
add_subdirectory (test)
endif()
