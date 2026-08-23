/*! \page tldr_logger Summary to Logger.h

[TOC]

Header `bux/Logger.h` wraps the process-wide logger behind a handful of macros.
Whatever the sink is, usage always boils down to the same two steps:

1. **Define the sink once** &mdash; exactly one `DEF_LOGGER_`\em XXX line, at namespace
   scope, in exactly one translation unit of the program. It expands to the definition of
   bux::user::logger(), which the library-side bux::logger() calls.
2. **Log from anywhere else** &mdash; any thread, any translation unit which includes
   `<bux/Logger.h>`. No logger object is ever passed around.

~~~cpp
#include <bux/Logger.h>     // DEF_LOGGER_COUT(), LOG(), FUNLOGX1()
#include <iostream>         // std::cout

DEF_LOGGER_COUT()           // (1) once per program

void foo(int i)
{                           // (2) anywhere, any thread
    FUNLOGX1(i);
    LOG(LL_INFO, "i*2 = {}", i*2);
}
~~~

\section tldr_logger_sink 1. Pick a sink

| Macro *(square brackets mark optional arguments)* | Sink | Include before it |
|---------------------------------------------------|------|-------------------|
| `DEF_LOGGER_COUT([max_ll])`                | `std::cout`                                    | `<iostream>` |
| `DEF_LOGGER_CERR([max_ll])`                | `std::cerr`                                    | `<iostream>` |
| `DEF_LOGGER_OSTREAM(out[, max_ll])`        | any `std::ostream` lvalue outliving the logger | &mdash; |
| `DEF_LOGGER_FILE(path[, max_ll])`          | one `std::ofstream` opened at \c path          | `<fstream>` |
| `DEF_LOGGER_FILES(pathfmt[, max_ll])`      | file whose name is re-formatted from the current timestamp | `<bux/FileLog.h>` |
| `DEF_FALLBACK_LOGGER_FILES(fsize, paths)`  | same, plus fallback to a finer path format once \c fsize bytes are exceeded | `<bux/FileLog.h>` |
| `DEF_PARA_LOGGER`                          | bux::C_ParaLog facade fanning out to child loggers | `<bux/ParaLog.h>` |

\c max_ll is the *most verbose* level the sink accepts, defaulting to \c LL_VERBOSE
(*i.e.* accept everything). See \ref tldr_logger_levels.

\note `DEF_LOGGER_COUT()` and `DEF_LOGGER_CERR()` are the same macro with a different
      stream, so using **both** in one program is an ODR violation, as is any two
      `DEF_LOGGER_`\em XXX lines.

\subsection tldr_logger_sink_files Timestamped file names

\c pathfmt is a `std::format` string taking the current timestamp as its single
argument, so a new file is opened whenever the formatted name changes:

~~~cpp
#include <bux/Logger.h>     // DEF_LOGGER_FILES()
#include <bux/FileLog.h>    // bux::C_PathFmtLogSnap

DEF_LOGGER_FILES("timelog/{:%y%m%d_%H%M}.log")  // one file per minute
~~~

Missing directories are created on demand by bux::C_PathFmtLogSnap &mdash; unlike
`DEF_LOGGER_FILE()`, whose plain `std::ofstream` requires the folder to already exist.

`DEF_FALLBACK_LOGGER_FILES()` additionally caps each file by size: once the current file
exceeds \c fsize bytes, the next path format in \c paths takes over. Keep \c paths in
a namespace-scope `constinit` object, because it is consumed by a global initializer:

~~~cpp
#include <bux/Logger.h>     // DEF_FALLBACK_LOGGER_FILES()
#include <bux/FileLog.h>    // bux::C_PathFmtLogSnap

constinit const std::array fallbacks{
    "timelog/{:%y%m%d-%H}.log",
    "timelog/{:%y%m%d-%H-%M}.log",
    "timelog/{:%y%m%d-%H-%M-%S}.log"
};
DEF_FALLBACK_LOGGER_FILES(65536, fallbacks)
~~~

\subsection tldr_logger_sink_para Several sinks at once

`DEF_PARA_LOGGER` defines `bux::user::g_paraLog`, a bux::C_ParaLog to be populated at
run time &mdash; each child keeps its own \c max_ll, and bux::C_ParaLog::partitionBy()
routes lines to children by content:

~~~cpp
#include <bux/Logger.h>     // DEF_PARA_LOGGER
#include <bux/ParaLog.h>    // bux::C_ParaLog
#include <bux/FileLog.h>    // bux::C_PathFmtLogSnap

DEF_PARA_LOGGER

int main()
{
    using bux::user::g_paraLog;
    g_paraLog.addChild(std::cout, LL_WARNING);                  // console: warnings and above
    g_paraLog.addChildT<bux::C_PathFmtLogSnap>([](auto &logger) // rotated files: everything
    {
        logger.configPath("logs/{:%Y-%m-%d}.log");
    });
    auto nodes = g_paraLog.partitionBy(std::initializer_list<FC_MatchStr>{"[foo]", "[bar]"});
    nodes[0].addChildT<bux::C_PathFmtLogSnap>(...);             // lines containing "[foo]"
    nodes[1].addChildT<bux::C_PathFmtLogSnap>(...);             // lines containing "[bar]"
    nodes.matchedNone().addChildT<bux::C_PathFmtLogSnap>(...);  // all the other lines
}
~~~

\section tldr_logger_macros 2. Log with the macros

| Macro | Emits |
|-------|-------|
| `LOG(ll, fmtStr[, args...])`             | one stamped line, dropped unless \c ll passes the level filter |
| `LOG1(ll, str)`                          | `LOG(ll, "{}", str)`, for text which is not a literal |
| `LOG_RAW(fmtStr[, args...])`             | the formatted text alone &mdash; no stamp, no level filtering |
| `FUNLOG`                                 | `{` on the spot and `}` at end of the enclosing block, tagged with the current function name |
| `FUNLOGX(fmtStr[, args...])`             | same, with the formatted text as the "arguments" of the scope |
| `FUNLOGX1(x1)` &hellip; `FUNLOGX9(x1, ..., x9)` | same, with the values rendered as a comma-separated list |
| `SCOPELOG(scope)`                        | as `FUNLOG`, but named \c scope instead of the current function |
| `SCOPELOGX(scope, fmtStr[, args...])`    | as `FUNLOGX()`, but named \c scope |
| `SCOPELOGX1(scope, x1)` &hellip; `SCOPELOGX9(scope, x1, ..., x9)` | as `FUNLOGX1()`&hellip;`FUNLOGX9()`, but named \c scope |

The `FUNLOG`/`SCOPELOG` family declares an unnamed bux::C_EntryLog on the stack, so the
closing line is written by its destructor &mdash; through every `return`, `break`, and
`throw` &mdash; and nested scopes indent the lines in between.

\section tldr_logger_levels Log levels

Declared in `bux/LogLevel.h` and exported to the global namespace, so no `bux::` prefix
is needed:

| Level | Letter | Meaning |
|-------|--------|---------|
| `LL_FATAL`   | `F` | The program should shut down right after reporting this. |
| `LL_ERROR`   | `E` | Error the program can keep running after. |
| `LL_WARNING` | `W` | Worth warning about, nothing sabotaged yet. |
| `LL_INFO`    | `I` | Status worth mentioning, normal or not. |
| `LL_DEBUG`   | `D` | Debug-only, easily suppressed for releases. |
| `LL_VERBOSE` | `V` | More detail than some would care for. |

A line survives when its level is *not more verbose* than the sink's \c max_ll, *i.e.*
`DEF_LOGGER_COUT(LL_WARNING)` keeps `LL_FATAL`, `LL_ERROR`, `LL_WARNING` and drops the
rest. Filtering costs no formatting: the arguments of `LOG()` are never evaluated once
the level is rejected.

\section tldr_logger_format Anatomy of a line

Output of `test/smoke_coutlog.cpp`, abridged:

~~~
2026/08/23 18:17:34.509 tid173271 V:********** LOGS BEGUN **********
2026/08/23 18:17:34.509 tid173271 F:Hello fatal
2026/08/23 18:17:34.509 tid173271 V:@1@int main()(Outer) {
2026/08/23 18:17:34.509 tid173271 W:|Hello warning
2026/08/23 18:17:34.509 tid173271 V:|@2@int main() {
2026/08/23 18:17:34.509 tid173271 I:||Hello info
2026/08/23 18:17:34.509 tid173271 V:|@2@}
2026/08/23 18:17:34.509 tid173271 V:@1@}
~~~

- `2026/08/23 18:17:34.509` &mdash; timestamp down to the millisecond, in local time
  unless told otherwise by \ref tldr_logger_switches "LOGGER_USE_LOCAL_TIME_".
- `tid173271` &mdash; id of the calling thread.
- `V:` &mdash; the level letter of the table above.
- `||` &mdash; one `|` per enclosing `FUNLOG`/`SCOPELOG` scope, counted per thread.
- `@2@`&hellip;`{` / `@2@}` &mdash; matching entry/exit pair of one scope; the serial
  number pairs them up even when other threads interleave. A scope left by an exception
  closes with `@2@} due to 1 uncaught exception` instead.
- `********** LOGS BEGUN **********` is written once, on the first call to bux::logger().

\section tldr_logger_switches Compile-time switches

Both are consulted by `bux/Logger.h` at include time, so `#define` them **before**
including it &mdash; ideally on the compiler command line, so all translation units agree.

- `TURN_OFF_LOGGER_` &mdash; every macro of \ref tldr_logger_macros "the table above"
  expands to nothing and no arguments are evaluated. Note that bux::user::logger() is
  then never defined either, so code calling bux::logger() directly has to be guarded:
  ~~~cpp
  #ifndef TURN_OFF_LOGGER_
      if (bux::C_UseLog u{bux::logger()})
          *u <<"raw text\n";
  #endif
  ~~~
- `LOGGER_USE_LOCAL_TIME_` &mdash; the time zone of every timestamp, defaulting to
  `true`. Valid values:
  ~~~cpp
  #define LOGGER_USE_LOCAL_TIME_ true                                             // local time
  #define LOGGER_USE_LOCAL_TIME_ std::chrono::get_tzdb().current_zone()           // ditto
  #define LOGGER_USE_LOCAL_TIME_ std::chrono::get_tzdb().locate_zone("Asia/Taipei")
  #define LOGGER_USE_LOCAL_TIME_ false                                            // system clock
  #define LOGGER_USE_LOCAL_TIME_ nullptr                                          // ditto
  ~~~

\section tldr_logger_gotchas Gotchas

- **Format strings of `LOG()`/`LOG_RAW()` must be compile-time constants**, since they
  reach `std::format()` verbatim. Pass runtime text as an argument &mdash; `LOG1(ll, s)`
  is exactly that shorthand. The format string of `FUNLOGX()`/`SCOPELOGX()` instead goes
  through `std::vformat()`, so a mismatch throws `std::format_error` at run time rather
  than failing to compile.
- **Scope logs ignore the level filter.** `FUNLOG`, `SCOPELOG`, `LOG_RAW()` and manual
  bux::C_UseLog usage lock the sink without a level, so their lines are written even
  under `DEF_LOGGER_FILE("x.log", LL_INFO)`, despite being stamped `V`.
- **Nested logging is fine, and inner lines land first.** The sink is held by a
  `std::recursive_mutex` for the whole line, so logging from a function called inside a
  `LOG()` argument list neither deadlocks nor interleaves; the inner line simply
  completes first.
- Each line is flushed when it ends, so nothing is lost on a crash; conversely, a very
  chatty `LL_VERBOSE` sink is not free.

\section tldr_logger_beyond Below the macros

`LOG()` is a thin shorthand for locking the singleton and stamping a line, which is worth
doing by hand for output the macros do not shape &mdash; multi-line dumps, manipulators,
or anything streamed piecewise:

~~~cpp
if (bux::C_UseLog u{bux::logger()})         // no other thread writes until u dies
    *u <<std::boolalpha <<"raw, unstamped\n";

if (bux::C_UseLog u{bux::logger(), LL_INFO})  // ...unless LL_INFO is filtered out
    bux::stamp(u, LL_INFO) <<"stamped like LOG()\n";
~~~

A logger the `DEF_LOGGER_`\em XXX table does not cover is defined by writing
bux::user::logger() directly. `DEF_LOGGER_TAIL_()` closes the namespaces for you, which
is how `test/test_logger.cpp` swaps in a fresh sink per test case:

~~~cpp
namespace bux { namespace user {
std::unique_ptr<C_GizmoLogger> g_log;       // any bux::I_SyncLog will do
I_SyncLog &logger() {
DEF_LOGGER_TAIL_(*g_log)                    // expands to: return *g_log; }}}
~~~

\section tldr_logger_seealso See also

- `bux/LogLevel.h` &mdash; the `LL_`\em XXX enumerators.
- `bux/SyncLog.h` &mdash; bux::I_SyncLog, bux::C_SyncLogger, bux::C_UseLog and the
  reenterable building blocks the `DEF_LOGGER_`\em XXX macros assemble.
- `bux/FileLog.h` &mdash; bux::C_PathFmtLogSnap, behind `DEF_LOGGER_FILES()`.
- `bux/ParaLog.h` &mdash; bux::C_ParaLog, behind `DEF_PARA_LOGGER`.
- `bux/LogStream.h` &mdash; bux::timestamp() and bux::logTrace(), which compose the line
  prefix.
- Working examples: `test/smoke_coutlog.cpp`, `test/smoke_cerrlog.cpp`,
  `test/smoke_filelog.cpp`, `test/smoke_timelog.cpp`, `test/smoke_paralog.cpp`,
  `test/smoke_filtlog.cpp`, `test/test_logger.cpp`.
*/
