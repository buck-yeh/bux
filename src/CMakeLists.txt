set(USE_TOCHARS_CPP EZArgs.cpp EZScape.cpp FileLog.cpp LR1.cpp GLR.cpp ParserBase.cpp LogStream.cpp Logger.cpp XException.cpp )
if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
unset(XCONSOLE_CPP)
if (DEFINED CMAKE_OSX_DEPLOYMENT_TARGET AND CMAKE_OSX_DEPLOYMENT_TARGET VERSION_GREATER 0.0)
    if ((CMAKE_SYSTEM_NAME STREQUAL "iOS" AND CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS 16.5 ) OR
        (CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS 13.4 ))
        unset(USE_TOCHARS_CPP)
    endif()
endif()
else()
set(XCONSOLE_CPP XConsole.cpp)
endif()

message(NOTICE "XCONSOLE_CPP = ${XCONSOLE_CPP}")
message(NOTICE "USE_TOCHARS_CPP = ${USE_TOCHARS_CPP}")

add_library(bux STATIC
        AtomiX.cpp
        FA.cpp FileAsMem.cpp
        LexBase.cpp ParaLog.cpp
        ScannerBase.cpp Serialize.cpp StrUtil.cpp SyncLog.cpp
        UnicodeCvt.cpp
        ${USE_TOCHARS_CPP}
        ${XCONSOLE_CPP}
        )
target_include_directories(bux PRIVATE ../include/bux)
target_compile_features(bux PRIVATE cxx_std_23)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Android")
target_compile_options(bux PUBLIC -fPIC)
endif()
#target_compile_options(bux PRIVATE -g -O0)
if(Iconv_FOUND)
target_link_libraries(bux Iconv::Iconv)
endif()
