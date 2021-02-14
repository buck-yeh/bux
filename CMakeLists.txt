cmake_minimum_required (VERSION 3.18)
cmake_policy(VERSION 3.18)
project(bux)

add_subdirectory (src)

if(${BUILD_TEST})
add_subdirectory (test)
endif()
