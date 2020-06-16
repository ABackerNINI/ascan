// #pragma once

#ifndef _DEBUG_PRINT_H_
#define _DEBUG_PRINT_H_

#define ENABLE_DEBUG_LEVEL 1
#define ENABLE_DEBUG_PRINT 1

// TODO: console color redirection to file

/* Print file&func&line info when call print_'dbg' functions */
#define PRINT_FILE_FUNC_LINE 0
/* Force print file&func&line info when call print_'dbg'_ex functions */
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

#if (ENABLE_DEBUG_LEVEL)

enum DEBUG_LEVEL {
    DBG_LVL_ERROR = 0,
    DBG_LVL_WARNING = 1,
    DBG_LVL_INFO = 2,
    DBG_LVL_DEBUG = 3,
    DBG_LVL_MSGDUMP = 4,
    DBG_LVL_EXCESSIVE = 5,
};

extern int debug_level;  // --d=0: debug level

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

/* Print error msg. */
#define print_error(...) _PRINT_FUNC2(CC_FG_RED, CC_BRIGHT, ERROR, __VA_ARGS__)

/* Print warning msg. */
#define print_warning(...) _PRINT_FUNC(CC_FG_YELLOW, WARNING, __VA_ARGS__)

/* Print info msg. */
#define print_info(...) _PRINT_FUNC(CC_FG_GREEN, INFO, __VA_ARGS__)

/* Print debug msg. */
#define print_debug(...) _PRINT_FUNC(CC_FG_GREEN, DEBUG, __VA_ARGS__)

/* Print msgdump msg. */
#define print_msgdump(...) _PRINT_FUNC(CC_FG_GREEN, MSGDUMP, __VA_ARGS__)

/* Print excessive msg. */
#define print_excessive(...) _PRINT_FUNC(CC_FG_GREEN, EXCESSIVE, __VA_ARGS__)

/*---------------------------------------------------------------------------*/

#define _PRINT_FUNC_EX(lvl, ...)            \
    {                                       \
        if (debug_level >= DBG_LVL_##lvl) { \
            _FORCE_PRINT_FILE_FUNC_LINE;    \
            _DEBUG_PRINT(__VA_ARGS__);      \
        }                                   \
    }

/* Print extended error msg. */
#define print_error_ex(...) _PRINT_FUNC_EX(ERROR, __VA_ARGS__)

/* Print extended warning msg. */
#define print_warning_ex(...) _PRINT_FUNC_EX(WARNING, __VA_ARGS__)

/* Print extended info msg. */
#define print_info_ex(...) _PRINT_FUNC_EX(INFO, __VA_ARGS__)

/* Print extended debug msg. */
#define print_debug_ex(...) _PRINT_FUNC_EX(DEBUG, __VA_ARGS__)

/* Print extended msgdump msg. */
#define print_msgdump_ex(...) _PRINT_FUNC_EX(MSGDUMP, __VA_ARGS__)

/* Print extended excessive msg. */
#define print_excessive_ex(...) _PRINT_FUNC_EX(EXCESSIVE, __VA_ARGS__)

#endif  // ENABLE_DEBUG_LEVEL

/*===========================================================================*/

#if (ENABLE_DEBUG_PRINT)

#ifdef DEBUG

#define dbg_print(...)     \
    _PRINT_FILE_FUNC_LINE; \
    printf(__VA_ARGS__);

#else  // DEBUG

#define dbg_print(...)

#endif  // DEBUG

#endif  // ENABLE_DEBUG_PRINT

#endif  //_DEBUG_PRINT_H_
