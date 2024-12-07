//#define TURN_OFF_LOGGER_
#include <bux/Logger.h>     // DEF_LOGGER_COUT()
#include <iostream>         // std::cout

DEF_LOGGER_COUT()
//DEF_LOGGER_COUT(LL_WARNING)

void fun9()
{
    FUNLOGX9('1','2','3','4','5','6','7','8','9');
    LOG1(LL_INFO, "End of indentation");
}

void fun8()
{
    FUNLOGX8('1','2','3','4','5','6','7','8');
    fun9();
}

void fun7()
{
    FUNLOGX7('1','2','3','4','5','6','7');
    fun8();
}

void fun6()
{
    FUNLOGX6('1','2','3','4','5','6');
    fun7();
}

void fun5()
{
    FUNLOGX5('1','2','3','4','5');
    fun6();
}

void fun4()
{
    FUNLOGX4('1','2','3','4');
    fun5();
}

void fun3()
{
    FUNLOGX3('1','2','3');
    fun4();
}

void fun2()
{
    FUNLOGX2('1','2');
    fun3();
}

void fun1()
{
    FUNLOGX1('1');
    fun2();
}

int main()
{
    LOG(LL_FATAL,   "Hello fatal");
    LOG(LL_ERROR,   "Hello error");
    FUNLOGX("Outer");
    LOG(LL_WARNING, "Hello warning");
    FUNLOG;
    LOG(LL_INFO,    "Hello info");
    LOG(LL_VERBOSE, "Hello verbose");
    //---------------------------------
    LOG1(LL_INFO, "{}");
    //---------------------------------
    SCOPELOGX1("Indent", 1);
    SCOPELOGX2("Indent", 1, 2);
    SCOPELOGX3("Indent", 1, 2, 3);
    SCOPELOGX4("Indent", 1, 2, 3, 4);
    SCOPELOGX5("Indent", 1, 2, 3, 4, 5);
    SCOPELOGX6("Indent", 1, 2, 3, 4, 5, 6);
    SCOPELOGX7("Indent", 1, 2, 3, 4, 5, 6, 7);
    SCOPELOGX8("Indent", 1, 2, 3, 4, 5, 6, 7, 8);
    SCOPELOGX9("Indent", 1, 2, 3, 4, 5, 6, 7, 8, 9);
    fun1();
}
