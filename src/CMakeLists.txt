if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
set(XCONSOLE_CPP )
else()
set(XCONSOLE_CPP XConsole.cpp)
endif()
message(DEBUG "XCONSOLE_CPP = ${XCONSOLE_CPP}")

add_library(bux STATIC
        AtomiX.cpp
        EZArgs.cpp EZScape.cpp
        FA.cpp FileAsMem.cpp FileLog.cpp
        LexBase.cpp LogStream.cpp LR1.cpp GLR.cpp ParserBase.cpp Logger.cpp ParaLog.cpp
        ScannerBase.cpp Serialize.cpp StrUtil.cpp SyncLog.cpp
        UnicodeCvt.cpp
        ${XCONSOLE_CPP} XException.cpp
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
