cmake_minimum_required (VERSION 3.18)
cmake_policy(VERSION 3.18)
project(build_tester)

enable_testing()

string(APPEND CMAKE_CXX_FLAGS " -Wall -Wextra -Wshadow -Wconversion -Wno-parentheses -Wno-unknown-pragmas -O2 -std=c++2a")

add_executable(test_expand_env test_expand_env.cpp)
target_link_libraries(test_expand_env bux fmt stdc++)

add_test(NAME test_home
         COMMAND test_expand_env home
         )
set_tests_properties(test_home PROPERTIES PASS_REGULAR_EXPRESSION "$ENV{HOME}")

add_test(NAME test_pass_str
         COMMAND test_expand_env in test_in.txt
         )
set_tests_properties(test_pass_str PROPERTIES PASS_REGULAR_EXPRESSION "XYZ-$ENV{USER}")

add_test(NAME test_pass_env
         COMMAND test_expand_env env "USER"
         )
set_tests_properties(test_pass_env PROPERTIES PASS_REGULAR_EXPRESSION "$ENV{USER}")
