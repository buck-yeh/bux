#include <bux/StrUtil.h>    // bux::expand_env()
#include <fstream>          // std::ifstream
#include <iostream>         // std::cout
#include <cstdlib>          // std::getenv()
#include <cstring>          // std::strcmp()

enum
{
    ERR_NONE = 0,
    ERR_ARGS,
};

int main(int argc, char *argv[])
{
    if (argc < 2)
        return ERR_ARGS;

    if (!strcmp(argv[1], "home"))
        std::cout <<bux::expand_env("~") <<'\n';
    else if (!strcmp(argv[1], "env"))
    {
        if (argc < 3)
            return ERR_ARGS;

        std::cout <<getenv(argv[2]) <<'\n';
    }
    else if (!strcmp(argv[1], "in"))
    {
        if (argc < 3)
            return ERR_ARGS;

        std::ifstream in{argv[2]};
        std::string line;
        if (!getline(in, line))
            return ERR_ARGS;

        std::cout <<bux::expand_env(line.c_str()) <<'\n';
    }
    else
        return ERR_ARGS;

    return 0;
}
