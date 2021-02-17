enable_testing()

string(APPEND CMAKE_CXX_FLAGS " -Wall -Wextra -Wshadow -Wconversion -Wno-parentheses -Wno-unknown-pragmas -g3 -O3 -std=c++2a")

add_executable(hrtn hrtn.cpp)
target_include_directories(hrtn PRIVATE ../include)
target_link_libraries(hrtn bux stdc++)

add_executable(test_expand_env test_expand_env.cpp)
#target_compile_options(test_expand_env PUBLIC -D_GLIBCXX_USE_CXX11_ABI=1)
target_include_directories(test_expand_env PRIVATE ../include)
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

find_package(Catch2 REQUIRED)
include(Catch)

add_executable(test_PO test_PO.cpp)
target_include_directories(test_PO PRIVATE ../include)
target_link_libraries(test_PO PRIVATE bux stdc++ m)
catch_discover_tests(test_PO)
#add_test(NAME test_PO_All COMMAND test_PO)

add_executable(test_atomix test_atomix.cpp)
target_include_directories(test_atomix PRIVATE ../include)
target_link_libraries(test_atomix PRIVATE bux stdc++ tbb m)
catch_discover_tests(test_atomix)
#add_test(NAME test_atomix_All COMMAND test_atomix)
