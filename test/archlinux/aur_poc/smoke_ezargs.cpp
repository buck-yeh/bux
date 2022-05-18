#include <bux/EZArgs.h>     // bux::C_EZArgs, bux::C_ErrorOrIndex
#include <iostream>         // std::cerr

int main(int argc, char *argv[])
{
    auto ret = bux::C_EZArgs{}.parse(argc, argv);
    if (ret)
        std::cout <<"Parsed ok\n";
    else
        std::cout <<"Parse returned:\n" <<ret.message();
}
