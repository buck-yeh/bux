enable_testing()

add_executable(test_expand_env test_expand_env.cpp)
target_include_directories(test_expand_env PRIVATE ../include)
target_link_libraries(test_expand_env Bux)

message("HOME=$ENV{HOME}")
message("USER=$ENV{USER}")
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
