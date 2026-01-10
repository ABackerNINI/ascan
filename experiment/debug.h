/* #pragma once */

/* This is designed for gcc/clang on linux. */

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
/* Debug level during compiling, level greater than this will not be compiled
 * into code.*/
#define DEBUG_LEVEL 5
#endif

#ifndef ENABLE_RUNTIME_DEBUG_LEVEL
/* Debug level during runtime.
 *
 * You should declare global variable 'int debug_level;' in your .c/.cpp source
 * file and set 'debug_level' to DBG_LVL_'XXX', macros print_'xxx',
 * print_'xxx'_ex and stmt_'xxx' will be executed only if debug_level is not
 * less than the corresponding value.
 *
 * E.g. If you set debug_level to DBG_LVL_INFO, print_error/warning/info will be
 * executed but print_debug/msgdump/excessive will not.
 */
#define ENABLE_RUNTIME_DEBUG_LEVEL 1
#endif

#ifndef ENABLE_DEBUG_PRINT
/* Enable dbg_'xxx' macros. */
#define ENABLE_DEBUG_PRINT 1
#endif

#ifndef ENABLE_COLOR_PRINT
/* Enable console color print. This should be disabled on windows. */
#define ENABLE_COLOR_PRINT 1
#endif

#ifndef PRINT_FILE_FUNC_LINE
/* Print file & func & line info when call print_'xxx' macros. */
#define PRINT_FILE_FUNC_LINE 0
#endif

#ifndef PRINT_FILE_FUNC_LINE_EX
/* Force print file & func & line info when call print_'xxx'_ex macros. */
#define PRINT_FILE_FUNC_LINE_EX 0
#endif

#ifndef DBG_OUT
#define DBG_OUT stderr
#endif

/*===========================================================================*/

#ifndef _LINUX_CONSOLE_COLOR_
#define _LINUX_CONSOLE_COLOR_

#if (ENABLE_COLOR_PRINT)

/* Console color definations */

#define CC_RESET_ALL_ATTRIBUTES 0
#define CC_BRIGHT 1
#define CC_DIM 2
#define CC_UNDERSCORE 4
#define CC_BLINK 5
#define CC_REVERSE 7
#define CC_HIDDEN 8

/* Foreground Colors */

#define CC_FG_BLACK 30
#define CC_FG_RED 31
#define CC_FG_GREEN 32
#define CC_FG_YELLOW 33
#define CC_FG_BLUE 34
#define CC_FG_MAGENTA 35
#define CC_FG_CYAN 36
#define CC_FG_WHITE 37

/* Background Colors */

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
#define CC2(clr1, clr2, str) CC_BEGIN2(clr1, clr2) str CC_END
#define CC3(clr1, clr2, clr3, str) CC_BEGIN3(clr1, clr2, clr3) str CC_END

#else /* ENABLE_COLOR_PRINT */

#define CC_BEGIN(clr)
#define CC_BEGIN2(clr1, clr2)
#define CC_BEGIN3(clr1, clr2, clr3)
#define CC_END

#define CC(clr, str) str
#define CC2(clr1, clr2, str) str
#define CC3(clr1, clr2, clr3, str) str

#endif /* ENABLE_COLOR_PRINT */

#endif /* _LINUX_CONSOLE_COLOR_ */

/*===========================================================================*/

#define _DEBUG_PRINT(...) fprintf(DBG_OUT, __VA_ARGS__)
#define _PRINT_FILE_FUNC_LINE0 \
    _DEBUG_PRINT("@file:%s, func:%s, line:%d\n", __FILE__, __func__, __LINE__);

#if (PRINT_FILE_FUNC_LINE)
#define _PRINT_FILE_FUNC_LINE _PRINT_FILE_FUNC_LINE0
#else
#define _PRINT_FILE_FUNC_LINE
#endif

#if (PRINT_FILE_FUNC_LINE_EX)
#define _PRINT_FILE_FUNC_LINE_EX _PRINT_FILE_FUNC_LINE0
#else
#define _PRINT_FILE_FUNC_LINE_EX
#endif

/*---------------------------------------------------------------------------*/

#if (ENABLE_RUNTIME_DEBUG_LEVEL)

/* Runtime debug level, you should declare it yourself. */
extern int debug_level;

/*---------------------------------------------------------------------------*/

#include <stdio.h> /* fprintf */

#define _DEBUG_PRINT(...) fprintf(DBG_OUT, __VA_ARGS__)
#define __PRINT_FILE_FUNC_LINE \
    _DEBUG_PRINT("@file:%s, func:%s, line:%d\n", __FILE__, __func__, __LINE__);

#if (PRINT_FILE_FUNC_LINE)
#define _PRINT_FILE_FUNC_LINE __PRINT_FILE_FUNC_LINE
#else
#define _PRINT_FILE_FUNC_LINE
#endif

#if (PRINT_FILE_FUNC_LINE_EX)
#define _PRINT_FILE_FUNC_LINE_EX __PRINT_FILE_FUNC_LINE
#else
#define _PRINT_FILE_FUNC_LINE_EX
#endif

/*---------------------------------------------------------------------------*/

#if (ENABLE_COLOR_PRINT)

#include <unistd.h> /* isatty */

#define __PRINT_FUNC(_begin_clr_, dbg_lvl, lvl_str, ...)        \
    if (debug_level >= dbg_lvl) {                               \
        _PRINT_FILE_FUNC_LINE;                                  \
        if (isatty(fileno(DBG_OUT))) {                          \
            _DEBUG_PRINT("[" _begin_clr_ lvl_str CC_END "]: "); \
        } else {                                                \
            _DEBUG_PRINT("[" lvl_str "]: ");                    \
        }                                                       \
        _DEBUG_PRINT(__VA_ARGS__);                              \
    }

#else /* ENABLE_COLOR_PRINT: do not need to check isatty */

#define __PRINT_FUNC(_begin_clr_, dbg_lvl, lvl_str, ...) \
    if (debug_level >= dbg_lvl) {                        \
        _PRINT_FILE_FUNC_LINE;                           \
        _DEBUG_PRINT("[" lvl_str "]: ");                 \
        _DEBUG_PRINT(__VA_ARGS__);                       \
    }

#endif /* ENABLE_COLOR_PRINT */

#define _PRINT_FUNC_EX(lvl, ...)        \
    if (debug_level >= DBG_LVL_##lvl) { \
        _PRINT_FILE_FUNC_LINE_EX;       \
        _DEBUG_PRINT(__VA_ARGS__);      \
    }

#define _STMT_FUNC(lvl, ...)            \
    if (debug_level >= DBG_LVL_##lvl) { \
        __VA_ARGS__                     \
    }

#else /* ENABLE_RUNTIME_DEBUG_LEVEL */

#if (ENABLE_COLOR_PRINT)

#include <unistd.h> /* isatty */

#define __PRINT_FUNC(_begin_clr_, dbg_lvl, lvl_str, ...)        \
    do {                                                        \
        _PRINT_FILE_FUNC_LINE;                                  \
        if (isatty(fileno(DBG_OUT))) {                          \
            _DEBUG_PRINT("[" _begin_clr_ lvl_str CC_END "]: "); \
        } else {                                                \
            _DEBUG_PRINT("[" lvl_str "]: ");                    \
        }                                                       \
        _DEBUG_PRINT(__VA_ARGS__);                              \
    } while (0)

#else /* ENABLE_COLOR_PRINT: do not need to check isatty */

#define __PRINT_FUNC(_begin_clr_, dbg_lvl, lvl_str, ...) \
    do {                                                 \
        _PRINT_FILE_FUNC_LINE;                           \
        _DEBUG_PRINT("[" lvl_str "]: ");                 \
        _DEBUG_PRINT(__VA_ARGS__);                       \
    } while (0)

#endif /* ENABLE_COLOR_PRINT */

#define _PRINT_FUNC_EX(lvl, ...)   \
    do {                           \
        _PRINT_FILE_FUNC_LINE_EX;  \
        _DEBUG_PRINT(__VA_ARGS__); \
    } while (0)

#define _STMT_FUNC(lvl, ...) \
    do {                     \
        __VA_ARGS__          \
    } while (0)

#endif /* ENABLE_RUNTIME_DEBUG_LEVEL */

#define _PRINT_FUNC(clr, lvl, ...) \
    __PRINT_FUNC(CC_BEGIN(clr), DBG_LVL_##lvl, #lvl, __VA_ARGS__)
#define _PRINT_FUNC2(clr1, clr2, lvl, ...) \
    __PRINT_FUNC(CC_BEGIN2(clr1, clr2), DBG_LVL_##lvl, #lvl, __VA_ARGS__)

/*---------------------------------------------------------------------------*/

/* Macros print_'xxx', print_'xxx'_ex, and stmt_'xxx' are enabled if
 * DEBUG_LEVEL >= the corresponding value, with runtime debug level check if
 * ENABLE_RUNTIME_DEBUG_LEVEL is enabled.
 *
 * Macros stmt_'xxx'_raw do not have runtime debug level check. They are for
 * some old compilers do not support "Mixed Declarations and Code".
 * */

/*---------------------------------------------------------------------------*/

#if (DEBUG_LEVEL >= 0)
/* Print error msg. */
#define print_error(...) _PRINT_FUNC2(CC_FG_RED, CC_BRIGHT, ERROR, __VA_ARGS__)
/* Print extended error msg. */
#define print_error_ex(...) _PRINT_FUNC_EX(ERROR, __VA_ARGS__)
/* Statements for error. */
#define stmt_error(...) _STMT_FUNC(ERROR, __VA_ARGS__)
/* Raw statements for error, no rt check. */
#define stmt_error_raw(...) __VA_ARGS__
#else
/* Not enabled. */
#define print_error(...)
/* Not enabled. */
#define print_error_ex(...)
/* Not enabled. */
#define stmt_error(...)
/* Not enabled. */
#define stmt_error_raw(...)
#endif

/*---------------------------------------------------------------------------*/

#if (DEBUG_LEVEL >= 1)
/* Print warning msg. */
#define print_warning(...) _PRINT_FUNC(CC_FG_YELLOW, WARNING, __VA_ARGS__)
/* Print extended warning msg. */
#define print_warning_ex(...) _PRINT_FUNC_EX(WARNING, __VA_ARGS__)
/* Statements for warning. */
#define stmt_warning(...) _STMT_FUNC(WARNING, __VA_ARGS__)
/* Raw statements for warning, no rt check. */
#define stmt_warning_raw(...) __VA_ARGS__
#else
/* Not enabled. */
#define print_warning(...)
/* Not enabled. */
#define print_warning_ex(...)
/* Not enabled. */
#define stmt_warning(...)
/* Not enabled. */
#define stmt_warning_raw(...)
#endif

/*---------------------------------------------------------------------------*/

#if (DEBUG_LEVEL >= 2)
/* Print info msg. */
#define print_info(...) _PRINT_FUNC(CC_FG_GREEN, INFO, __VA_ARGS__)
/* Print extended info msg. */
#define print_info_ex(...) _PRINT_FUNC_EX(INFO, __VA_ARGS__)
/* Statements for info. */
#define stmt_info(...) _STMT_FUNC(INFO, __VA_ARGS__)
/* Raw statements for info, no rt check. */
#define stmt_info_raw(...) __VA_ARGS__
#else
/* Not enabled. */
#define print_info(...)
/* Not enabled. */
#define print_info_ex(...)
/* Not enabled. */
#define stmt_info(...)
/* Not enabled. */
#define stmt_info_raw(...)
#endif

/*---------------------------------------------------------------------------*/

#if (DEBUG_LEVEL >= 3)
/* Print debug msg. */
#define print_debug(...) _PRINT_FUNC(CC_FG_GREEN, DEBUG, __VA_ARGS__)
/* Print extended debug msg. */
#define print_debug_ex(...) _PRINT_FUNC_EX(DEBUG, __VA_ARGS__)
/* Statements for debug. */
#define stmt_debug(...) _STMT_FUNC(DEBUG, __VA_ARGS__)
/* Raw statements for debug, no rt check. */
#define stmt_debug_raw(...) __VA_ARGS__
#else
/* Not enabled. */
#define print_debug(...)
/* Not enabled. */
#define print_debug_ex(...)
/* Not enabled. */
#define stmt_debug(...)
/* Not enabled. */
#define stmt_debug_raw(...)
#endif

/*---------------------------------------------------------------------------*/

#if (DEBUG_LEVEL >= 4)
/* Print msgdump msg. */
#define print_msgdump(...) _PRINT_FUNC(CC_FG_GREEN, MSGDUMP, __VA_ARGS__)
/* Print extended msgdump msg. */
#define print_msgdump_ex(...) _PRINT_FUNC_EX(MSGDUMP, __VA_ARGS__)
/* Statements for msgdump. */
#define stmt_msgdump(...) _STMT_FUNC(MSGDUMP, __VA_ARGS__)
/* Raw statements for msgdump, no rt check. */
#define stmt_msgdump_raw(...) __VA_ARGS__
#else
/* Not enabled. */
#define print_msgdump(...)
/* Not enabled. */
#define print_msgdump_ex(...)
/* Not enabled. */
#define stmt_msgdump(...)
/* Not enabled. */
#define stmt_msgdump_raw(...)
#endif

/*---------------------------------------------------------------------------*/

#if (DEBUG_LEVEL >= 5)
/* Print excessive msg. */
#define print_excessive(...) _PRINT_FUNC(CC_FG_GREEN, EXCESSIVE, __VA_ARGS__)
/* Print extended excessive msg. */
#define print_excessive_ex(...) _PRINT_FUNC_EX(EXCESSIVE, __VA_ARGS__)
/* Statements for excessive. */
#define stmt_excessive(...) _STMT_FUNC(EXCESSIVE, __VA_ARGS__)
/* Raw statements for excessive, no rt check. */
#define stmt_excessive_raw(...) __VA_ARGS__
#else
/* Not enabled. */
#define print_excessive(...)
/* Not enabled. */
#define print_excessive_ex(...)
/* Not enabled. */
#define stmt_excessive(...)
/* Not enabled. */
#define stmt_excessive_raw(...)
#endif

/*===========================================================================*/

#if (ENABLE_DEBUG_PRINT)

#ifdef DEBUG

/* Macros dbg_'xxx' are enabled if DEBUG is defined. */

#include <stdio.h> /* printf */

#define dbg_print(...) printf(__VA_ARGS__)
#define dbg_stmt(...) \
    do {              \
        __VA_ARGS__   \
    } while (0)

#include <assert.h> /* assert */

#define dbg_require(...) assert(__VA_ARGS__)
#define dbg_assert(...) assert(__VA_ARGS__)
#define dbg_ensure(...) assert(__VA_ARGS__)

#else /* DEBUG */

/* Not enabled. */
#define dbg_print(...)
/* Not enabled. */
#define dbg_stmt(...)
/* Not enabled. */
#define dbg_require(...)
/* Not enabled. */
#define dbg_assert(...)
/* Not enabled. */
#define dbg_ensure(...)

#endif /* DEBUG */

#endif /* ENABLE_DEBUG_PRINT */

#endif /* _DEBUG_PRINT_H_ */
