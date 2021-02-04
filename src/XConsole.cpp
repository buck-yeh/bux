#include <iostream>         // std::cerr
#include <cstring>          // strchr()
#include "XConsole.h"
#ifdef _WIN32
    #include <conio.h>      // _kbhit(), _getch()
    #include <io.h>         // _access()
    #include <windows.h>    // GetStdHandle(), SetConsoleCursorPosition()
#elif defined(__unix__) || defined(__unix) || defined(__gnu_linux__)
    #include <termios.h>    // struct termios
    #include <unistd.h>     // access()
    #include <fcntl.h>      // fcntl()
#else
    #error "Platform other than Windows and Unix-like not supported yet"
#endif

namespace {

using namespace bux;

//
//      In-Module Types
//
#if defined(__unix__) || defined(__unix) || defined(__gnu_linux__)
class FC_GetChar
{
public:

    // Nonvirtuals
    FC_GetChar();
    ~FC_GetChar();
    int operator()();
    void bufferOff();
    void bufferOn();
    void echoOff();
    void echoOn();
    bool kbhit();

private:

    // Data
    struct termios      m_oldAttr, m_curAttr;
    bool                m_dirty;

    // Nonvirtuals
    void updateAttr();
};
#endif

//
//      In-Module Functions
//
#ifdef _MSC_VER
void clreol()
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    DWORD _;
    const HANDLE hConsole =GetStdHandle(STD_OUTPUT_HANDLE);
    if (INVALID_HANDLE_VALUE != hConsole &&
        GetConsoleScreenBufferInfo(hConsole, &info))
        FillConsoleOutputCharacter(hConsole, ' ', info.dwSize.X-info.dwCursorPosition.X,
            info.dwCursorPosition, &_);
}

void gotoxy(COORD pos)
{
    const HANDLE hConsole =GetStdHandle(STD_OUTPUT_HANDLE);
    if (INVALID_HANDLE_VALUE != hConsole)
        SetConsoleCursorPosition(hConsole, pos);
}

COORD cur_pos()
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    const HANDLE hConsole =GetStdHandle(STD_OUTPUT_HANDLE);
    if (INVALID_HANDLE_VALUE != hConsole &&
        GetConsoleScreenBufferInfo(hConsole, &info))
        return info.dwCursorPosition;

    static const COORD def ={0, 0};
    return def;
}
#endif

//
//      Implement Classes
//
#if defined(__unix__) || defined(__unix) || defined(__gnu_linux__)
FC_GetChar::FC_GetChar(): m_dirty(false)
{
    tcgetattr(0, &m_oldAttr);
    m_curAttr = m_oldAttr;
}

FC_GetChar::~FC_GetChar()
{
    if (m_curAttr.c_lflag != m_oldAttr.c_lflag)
        tcsetattr(0, TCSANOW, &m_oldAttr);
}

int FC_GetChar::operator()()
{
    updateAttr();
    return getchar();
}

void FC_GetChar::bufferOff()
{
    m_curAttr.c_lflag &= ~ICANON;
    m_dirty = true;
}

void FC_GetChar::echoOff()
{
    m_curAttr.c_lflag &= ~ECHO;
    m_dirty = true;
}

#if 0
void FC_GetChar::bufferOn()
{
    m_curAttr.c_lflag |= ICANON;
    m_dirty = true;
}

void FC_GetChar::echoOn()
{
    m_curAttr.c_lflag |= ECHO;
    m_dirty = true;
}

bool FC_GetChar::kbhit()
{
    updateAttr();
    const int oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    const int ch = getchar();
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF)
    {
        ungetc(ch, stdin);
        return true;
    }
    return false;
}
#endif

void FC_GetChar::updateAttr()
{
    if (m_dirty)
    {
        tcsetattr(0, TCSANOW, &m_curAttr);
        m_dirty = false;
    }
}
#endif

} // namespace

namespace bux {

//
//      Functions
//
void getKeyPunch(std::string_view msg, int &key, const char *keyFilter)
{
    std::cerr <<msg;
    int c;
#ifdef _WIN32
    do c =_getch();
#elif defined(__unix__) || defined(__unix) || defined(__gnu_linux__)
    FC_GetChar getc;
    getc.bufferOff();
    getc.echoOff();
    do  c = getc();
#endif
        while (keyFilter && !strchr(keyFilter, c));

    std::cerr <<char(c) <<'\n';
    key = c;
}

void pressAKey()
{
#ifdef _WIN32
    _getch();
#elif defined(__unix__) || defined(__unix) || defined(__gnu_linux__)
    FC_GetChar getc;
    getc.bufferOff();
    getc.echoOff();
    getc();
#endif
}

bool testWritability(const char *file)
{
#ifdef _WIN32
    if (!_access(file, 0))
#elif defined(__unix__) || defined(__unix) || defined(__gnu_linux__)
    if (!access(file, F_OK))
#endif
    {
        int c;
        getKeyPunch(std::string(file).append(" already exists. Overwrite it ?(y/n)"), c, "ynYN");
        if (c == 'n' || c == 'N')
            return false;
    }
    return true;
}

} // namespace bux
