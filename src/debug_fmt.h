#ifndef __DEBUG_PRINT_FMT_H__
#define __DEBUG_PRINT_FMT_H__

// #define DEBUG

#include "fmt.h" // IWYU pragma: keep

#ifdef DEBUG
    #include <cassert>

    #define __DBG_PRT_REAPEAT(...)                                                                                     \
        do {                                                                                                           \
            __VA_ARGS__;                                                                                               \
        } while (0)
#else
    #define __DBG_PRT_REAPEAT(...)                                                                                     \
        while (0) {                                                                                                    \
            __VA_ARGS__;                                                                                               \
        }
#endif

#define gprint(...)                                                                                                    \
    __DBG_PRT_REAPEAT(fmt::print(stderr, fmt::fg(fmt::color::green), "[DEBUG] "); fmt::print(stderr, __VA_ARGS__);)

#define gprintc(...)                                                                                                   \
    __DBG_PRT_REAPEAT(fmt::print(stderr, fmt::fg(fmt::color::green), " > "); fmt::print(stderr, __VA_ARGS__);)

#define gstmt(...) __DBG_PRT_REAPEAT(__VA_ARGS__;)

#define grequire(...) __DBG_PRT_REAPEAT(assert(__VA_ARGS__))
#define gassert(...) __DBG_PRT_REAPEAT(assert(__VA_ARGS__))
#define gensure(...) __DBG_PRT_REAPEAT(assert(__VA_ARGS__))

#endif // __DEBUG_PRINT_FMT_H__
