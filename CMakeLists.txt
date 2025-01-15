cmake_minimum_required (VERSION 3.18)
cmake_policy(VERSION 3.18)
project(bux)

set(GNU_LIKE_CXX_FLAGS " -Wall -Wextra -Wshadow -Wconversion -Wno-parentheses -std=c++23")
#string(APPEND GNU_LIKE_CXX_FLAGS " -ggdb")
string(APPEND GNU_LIKE_CXX_FLAGS " -O2")

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT MSVC)
    message(DEBUG "Clang")
    string(APPEND CMAKE_CXX_FLAGS "${GNU_LIKE_CXX_FLAGS} -Wno-potentially-evaluated-expression")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(DEBUG "GNU")
    string(APPEND CMAKE_CXX_FLAGS "${GNU_LIKE_CXX_FLAGS}")
elseif(MSVC)
    message(DEBUG "MSVC")
    string(APPEND CMAKE_CXX_FLAGS " /Zc:__cplusplus /std:c++latest /MP")
else()
    message(DEBUG "Unknown compiler")
endif()

add_subdirectory (src)

if(${BUILD_TEST})
message(DEBUG "Build tests too")
add_subdirectory (test)
endif()

install(TARGETS bux
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/bux DESTINATION include) # in ${CMAKE_INSTALL_PREFIX}/include/bux
