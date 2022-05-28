#include <bux/StrUtil.h>    // bux::_HRTN()
#include <iostream>         // std::cout

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i)
        std::cout <<argv[i] <<"\t-> " <<bux::_HRTN(argv[i]) <<'\n';

    std::cout <<"Total " <<(argc - 1) <<" conversion(s)\n";
}
