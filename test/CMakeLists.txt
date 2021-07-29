enable_testing()

add_executable(hrtn hrtn.cpp)
target_include_directories(hrtn PRIVATE ../include)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
target_link_libraries(hrtn PRIVATE bux)
else()
target_link_libraries(hrtn PRIVATE bux stdc++)
endif()

add_executable(smoke_coutlog smoke_coutlog.cpp)
target_include_directories(smoke_coutlog PRIVATE ../include)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
target_link_libraries(smoke_coutlog PRIVATE bux fmt::fmt)
else()
target_link_libraries(smoke_coutlog PRIVATE bux fmt stdc++)
endif()

add_executable(smoke_cerrlog smoke_cerrlog.cpp)
target_include_directories(smoke_cerrlog PRIVATE ../include)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
target_link_libraries(smoke_cerrlog PRIVATE bux fmt::fmt)
else()
target_link_libraries(smoke_cerrlog PRIVATE bux fmt stdc++)
endif()

add_executable(smoke_filelog smoke_filelog.cpp)
target_include_directories(smoke_filelog PRIVATE ../include)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
target_link_libraries(smoke_filelog PRIVATE bux fmt::fmt)
else()
target_link_libraries(smoke_filelog PRIVATE bux fmt stdc++)
endif()

add_executable(smoke_timelog smoke_timelog.cpp)
target_include_directories(smoke_timelog PRIVATE ../include)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
target_link_libraries(smoke_timelog PRIVATE bux fmt::fmt)
else()
target_link_libraries(smoke_timelog PRIVATE bux fmt stdc++)
endif()

add_executable(smoke_paralog smoke_paralog.cpp)
target_include_directories(smoke_paralog PRIVATE ../include)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
target_link_libraries(smoke_paralog PRIVATE bux fmt::fmt)
else()
target_link_libraries(smoke_paralog PRIVATE bux fmt stdc++ pthread curses)
endif()

add_executable(test_expand_env test_expand_env.cpp)
#target_compile_options(test_expand_env PUBLIC -D_GLIBCXX_USE_CXX11_ABI=1)
target_include_directories(test_expand_env PRIVATE ../include)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
target_link_libraries(test_expand_env PRIVATE bux fmt::fmt)
else()
target_link_libraries(test_expand_env PRIVATE bux fmt stdc++)
endif()

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
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
target_link_libraries(test_PO PRIVATE bux fmt::fmt)
else()
target_link_libraries(test_PO PRIVATE bux fmt stdc++ m)
endif()
catch_discover_tests(test_PO)
add_test(NAME test_PO_All COMMAND test_PO)

add_executable(test_atomix test_atomix.cpp)
target_include_directories(test_atomix PRIVATE ../include)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
target_link_libraries(test_atomix PRIVATE bux fmt::fmt)
else()
target_link_libraries(test_atomix PRIVATE bux fmt stdc++ m)
endif()
target_link_libraries(test_atomix PRIVATE bux stdc++ m)
add_test(NAME test_atomix_All COMMAND test_atomix)
