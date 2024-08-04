cmake_minimum_required (VERSION 3.18)
cmake_policy(VERSION 3.18)
project(bux)

set(GNU_LIKE_CXX_FLAGS " -Wall -Wextra -Wshadow -Wconversion -Wno-parentheses -g3 -Og -std=c++23")
#string(APPEND CMAKE_CXX_FLAGS " -g3")

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT MSVC)
    message("Clang")
    string(APPEND CMAKE_CXX_FLAGS "${GNU_LIKE_CXX_FLAGS} -Wno-potentially-evaluated-expression")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message("GNU")
    string(APPEND CMAKE_CXX_FLAGS "${GNU_LIKE_CXX_FLAGS}")
elseif(MSVC)
    message("MSVC")
    string(APPEND CMAKE_CXX_FLAGS " /Zc:__cplusplus /std:c++latest /MP")
else()
    message("Unknown compiler")
endif()

add_subdirectory (src)

if(${BUILD_TEST})
message("Build tests too")
add_subdirectory (test)
endif()

install(TARGETS bux
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/bux DESTINATION include) # in ${CMAKE_INSTALL_PREFIX}/include/bux
