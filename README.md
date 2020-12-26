# Brief
Supplemental static library whatever is required in sense of general purpose but is not directly supported from Modern C++. Or whatever reusable originated from my side projects. 

The libraty is `<sstream>`-free and uses [fmt](https://github.com/fmtlib/fmt) library heavily as long as [C++20 \<format\>](https://en.cppreference.com/w/cpp/utility/format) is not there yet.

# Installation & Usage
## In [ArchLinux](https://archlinux.org/)
1. Make sure you have installed [`yay`](https://aur.archlinux.org/packages/yay/) or any other [pacman wrapper](https://wiki.archlinux.org/index.php/AUR_helpers)
2. ~~~bash
   yay -Ss bux
   ~~~
3. To define target executable `foo` using `bux` in `CMakeLists.txt`
   ~~~cmake
   add_executable(foo foo.cpp)
   target_compile_options(foo PRIVATE -std=c++2a)
   target_link_libraries(foo bux fmt)
   ~~~
4. Include `bux` header files by prefixing header name with `bux/', for example:
   ~~~c++
   #include <bux/StrUtil.h>
   ~~~
   *p.s.* Header files are in `/usr/include/bux` and compiler is expected to search `/usr/include` by default.
5. If directly using `gcc/clang` is intended, the only required flag is `-lbux`
## Install from `github` in any of [Linux distros](https://distrowatch.com/)
1. Make sure you have installed `cmake` `make` `gcc` `git` `fmt`
2. ~~~bash
   git clone -b main --single-branch https://github.com/buck-yeh/bux.git .
   cmake .
   make -j
   BUX_DIR="/full/path/to/current/dir"
   ~~~
   *p.s.* You can install any tagged version by replacing `main` with the desired [tag name](https://github.com/buck-yeh/bux/tags).
3. To define target executable `foo` using `bux` in `CMakeLists.txt`
   ~~~cmake
   add_executable(foo foo.cpp)
   target_compile_options(foo PRIVATE -std=c++2a)
   target_include_directories(foo PRIVATE "$env{BUX_DIR}/include") 
   target_link_directories(foo PRIVATE "$env{BUX_DIR}/src") 
   target_link_libraries(foo bux fmt)
   ~~~
4. Include `bux` header files by prefixing header name with `bux/', for example:
   ~~~c++
   #include <bux/StrUtil.h>
   ~~~
5. If directly using `gcc/clang` is intended, the required flags are `-I$BUX_DIR/include -L$BUX_DIR/src -lbux`

# Header Intors
### Containers
### Logging
### Parser/scanner related
### Thread Safety
[AtomiX.h](include/bux/AtomiX.h)

