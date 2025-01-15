
add_library(bux STATIC
        AtomiX.cpp
        EZArgs.cpp EZScape.cpp
        FA.cpp FileLog.cpp
        LexBase.cpp LogStream.cpp LR1.cpp GLR.cpp ParserBase.cpp Logger.cpp ParaLog.cpp
        ScannerBase.cpp Serialize.cpp StrUtil.cpp SyncLog.cpp
        UnicodeCvt.cpp
        XConsole.cpp XException.cpp
        )
target_include_directories(bux PRIVATE ../include/bux)
if(Iconv_FOUND)
target_link_libraries(bux Iconv::Iconv)
endif()
