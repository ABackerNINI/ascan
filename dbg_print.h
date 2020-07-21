// #pragma once

#ifndef _DEBUG_PRINT_H_
#define _DEBUG_PRINT_H_

enum DBG_LEVEL {
    DBG_LVL_ERROR = 0,
    DBG_LVL_WARNING = 1,
    DBG_LVL_INFO = 2,
    DBG_LVL_DEBUG = 3,
    DBG_LVL_MSGDUMP = 4,
    DBG_LVL_EXCESSIVE = 5
};

#ifndef DEBUG_LEVEL
// Debug level during compiling, level greater than this will not be compiled
// into code.
#define DEBUG_LEVEL 5
#endif

#define ENABLE_RUNTIME_DEBUG_LEVEL 0
#define ENABLE_DEBUG_PRINT 1

// TODO: console color redirection to file

/* Print file&func&line info when call print_'dbg' functions. */
#define PRINT_FILE_FUNC_LINE 0
/* Force print file&func&line info when call print_'dbg'_ex functions. */
#define FORCE_PRINT_FILE_FUNC_LINE 0

/*===========================================================================*/

#define _DEBUG_PRINT(...) fprintf(stderr, __VA_ARGS__);
#define _PRINT_FILE_FUNC_LINE0 \
    _DEBUG_PRINT("@file:%s, func:%s, line:%d\n", __FILE__, __func__, __LINE__);

#if (PRINT_FILE_FUNC_LINE)
#define _PRINT_FILE_FUNC_LINE _PRINT_FILE_FUNC_LINE0
#else
#define _PRINT_FILE_FUNC_LINE
#endif

#if (FORCE_PRINT_FILE_FUNC_LINE)
#define _FORCE_PRINT_FILE_FUNC_LINE _PRINT_FILE_FUNC_LINE0
#else
#define _FORCE_PRINT_FILE_FUNC_LINE
#endif

/*===========================================================================*/

#ifndef _LINUX_CONSOLE_COLOR_
#define _LINUX_CONSOLE_COLOR_

/* Console color definations */

#define CC_RESET_ALL_ATTRIBUTES 0
#define CC_BRIGHT 1
#define CC_DIM 2
#define CC_UNDERSCORE 4
#define CC_BLINK 5
#define CC_REVERSE 7
#define CC_HIDDEN 8

/*  Foreground Colors */

#define CC_FG_BLACK 30
#define CC_FG_RED 31
#define CC_FG_GREEN 32
#define CC_FG_YELLOW 33
#define CC_FG_BLUE 34
#define CC_FG_MAGENTA 35
#define CC_FG_CYAN 36
#define CC_FG_WHITE 37

/*  Background Colors */

#define CC_BG_BLACK 40
#define CC_BG_RED 41
#define CC_BG_GREEN 42
#define CC_BG_YELLOW 43
#define CC_BG_BLUE 44
#define CC_BG_MAGENTA 45
#define CC_BG_CYAN 46
#define CC_BG_WHITE 47

/* Console color control */

#define _CC_EXPAND(clr) #clr /* expand the macro or will compile error */
#define CC_BEGIN(clr) "\033[" _CC_EXPAND(clr) "m"
#define CC_BEGIN2(clr1, clr2) "\033[" _CC_EXPAND(clr1) ";" _CC_EXPAND(clr2) "m"
#define CC_BEGIN3(clr1, clr2, clr3) \
    "\033[" _CC_EXPAND(clr1) ";" _CC_EXPAND(clr2) ";" _CC_EXPAND(clr3) "m"
#define CC_END "\033[0m"

#define CC(clr, str) CC_BEGIN(clr) str CC_END
#define CC2(clr1, clr2, str) CC_BEGIN(clr1) CC_BEGIN(clr2) str CC_END
#define CC3(clr1, clr2, clr3, str) \
    CC_BEGIN(clr1) CC_BEGIN(clr2) CC_BEGIN(clr3) str CC_END

#endif  // _LINUX_CONSOLE_COLOR_

/*===========================================================================*/

#if (ENABLE_RUNTIME_DEBUG_LEVEL)

extern int debug_level;  // Runtime debug level, you should define it yourself.

/*---------------------------------------------------------------------------*/

#define _PRINT_FUNC(clr, lvl, ...)                             \
    {                                                          \
        if (debug_level >= DBG_LVL_##lvl) {                    \
            _PRINT_FILE_FUNC_LINE;                             \
            _DEBUG_PRINT("[" CC_BEGIN(clr) #lvl CC_END "]: "); \
            _DEBUG_PRINT(__VA_ARGS__);                         \
        }                                                      \
    }
#define _PRINT_FUNC2(clr1, clr2, lvl, ...)                             \
    {                                                                  \
        if (debug_level >= DBG_LVL_##lvl) {                            \
            _PRINT_FILE_FUNC_LINE;                                     \
            _DEBUG_PRINT("[" CC_BEGIN2(clr1, clr2) #lvl CC_END "]: "); \
            _DEBUG_PRINT(__VA_ARGS__);                                 \
        }                                                              \
    }
#define _PRINT_FUNC_EX(lvl, ...)            \
    {                                       \
        if (debug_level >= DBG_LVL_##lvl) { \
            _FORCE_PRINT_FILE_FUNC_LINE;    \
            _DEBUG_PRINT(__VA_ARGS__);      \
        }                                   \
    }

#else  // ENABLE_RUNTIME_DEBUG_LEVEL

#define _PRINT_FUNC(clr, lvl, ...)                         \
    {                                                      \
        _PRINT_FILE_FUNC_LINE;                             \
        _DEBUG_PRINT("[" CC_BEGIN(clr) #lvl CC_END "]: "); \
        _DEBUG_PRINT(__VA_ARGS__);                         \
    }
#define _PRINT_FUNC2(clr1, clr2, lvl, ...)                         \
    {                                                              \
        _PRINT_FILE_FUNC_LINE;                                     \
        _DEBUG_PRINT("[" CC_BEGIN2(clr1, clr2) #lvl CC_END "]: "); \
        _DEBUG_PRINT(__VA_ARGS__);                                 \
    }
#define _PRINT_FUNC_EX(lvl, ...)     \
    {                                \
        _FORCE_PRINT_FILE_FUNC_LINE; \
        _DEBUG_PRINT(__VA_ARGS__);   \
    }

#endif  // ENABLE_RUNTIME_DEBUG_LEVEL

#define _STMT_FUNC(...) __VA_ARGS__

/*---------------------------------------------------------------------------*/

#if DEBUG_LEVEL >= 0
/* Print error msg. */
#define print_error(...) _PRINT_FUNC2(CC_FG_RED, CC_BRIGHT, ERROR, __VA_ARGS__)
/* Print extended error msg. */
#define print_error_ex(...) _PRINT_FUNC_EX(ERROR, __VA_ARGS__)
#define stmt_error(...) _STMT_FUNC(__VA_ARGS__)
#else
#define print_error(...)
#define print_error_ex(...)
#define stmt_error(...)
#endif

#if DEBUG_LEVEL >= 1
/* Print warning msg. */
#define print_warning(...) _PRINT_FUNC(CC_FG_YELLOW, WARNING, __VA_ARGS__)
/* Print extended warning msg. */
#define print_warning_ex(...) _PRINT_FUNC_EX(WARNING, __VA_ARGS__)
#define stmt_warning(...) _STMT_FUNC(__VA_ARGS__)
#else
#define print_warning(...)
#define print_warning_ex(...)
#define stmt_warning(...)
#endif

#if DEBUG_LEVEL >= 2
/* Print info msg. */
#define print_info(...) _PRINT_FUNC(CC_FG_GREEN, INFO, __VA_ARGS__)
/* Print extended info msg. */
#define print_info_ex(...) _PRINT_FUNC_EX(INFO, __VA_ARGS__)
#define stmt_info(...) _STMT_FUNC(__VA_ARGS__)
#else
#define print_info(...)
#define print_info_ex(...)
#define stmt_info(...)
#endif

#if DEBUG_LEVEL >= 3
/* Print debug msg. */
#define print_debug(...) _PRINT_FUNC(CC_FG_GREEN, DEBUG, __VA_ARGS__)
/* Print extended debug msg. */
#define print_debug_ex(...) _PRINT_FUNC_EX(DEBUG, __VA_ARGS__)
#define stmt_debug(...) _STMT_FUNC(__VA_ARGS__)
#else
#define print_debug(...)
#define print_debug_ex(...)
#define stmt_debug(...)
#endif

#if DEBUG_LEVEL >= 4
/* Print msgdump msg. */
#define print_msgdump(...) _PRINT_FUNC(CC_FG_GREEN, MSGDUMP, __VA_ARGS__)
/* Print extended msgdump msg. */
#define print_msgdump_ex(...) _PRINT_FUNC_EX(MSGDUMP, __VA_ARGS__)
#define stmt_msgdump(...) _STMT_FUNC(__VA_ARGS__)
#else
#define print_msgdump(...)
#define print_msgdump_ex(...)
#define stmt_msgdump(...)
#endif

#if DEBUG_LEVEL >= 5
/* Print excessive msg. */
#define print_excessive(...) _PRINT_FUNC(CC_FG_GREEN, EXCESSIVE, __VA_ARGS__)
/* Print extended excessive msg. */
#define print_excessive_ex(...) _PRINT_FUNC_EX(EXCESSIVE, __VA_ARGS__)
#define stmt_excessive(...) _STMT_FUNC(__VA_ARGS__)
#else
#define print_excessive(...)
#define print_excessive_ex(...)
#define stmt_excessive(...)
#endif

/*===========================================================================*/

#if (ENABLE_DEBUG_PRINT)

#ifdef DEBUG

#define dbg_print(...)     \
    _PRINT_FILE_FUNC_LINE; \
    printf(__VA_ARGS__);
#define dbg_stmt(...) __VA_ARGS__

#include <assert.h>
#define dbg_require(...) assert(__VA_ARGS__)
#define dbg_assert(...) assert(__VA_ARGS__)
#define dbg_ensure(...) assert(__VA_ARGS__)

#else  // DEBUG

#define dbg_print(...)
#define dbg_stmt(...)
#define dbg_require(...)
#define dbg_assert(...)
#define dbg_ensure(...)

#endif  // DEBUG

#endif  // ENABLE_DEBUG_PRINT

#endif  //_DEBUG_PRINT_H_
