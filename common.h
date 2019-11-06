#pragma once

#ifndef _AUTO_SCAN_COMMON_H_
#define _AUTO_SCAN_COMMON_H_

#include <string>
#include <vector>

using std::string;
using std::vector;

#define AS_VERSION "0.1.0"
#define AS_URL "git@github.com:ABackerNINI/ascan.git"

#define DEBUG 1

enum ASCAN_DEBUG_LEVEL {
    ADL_ERROR = 0,
    ADL_WARNING = 1,
    ADL_INFO = 2,
    ADL_DEBUG = 3,
    // MSG_MSGDUMP = 4,
    // MSG_EXCESSIVE = 5,
};

extern int debug_level;  // --d=0: debug level

/* Console colors */

#define CC_RED "31"
#define CC_GREEN "32"
#define CC_YELLOW "33"
#define CC_BLUE "34"
#define CC_PURPLE "35"
#define CC_RED_B "41"
#define CC_BOLD "1"
#define CC_UNDERLINE "4"

#define _CLR_BEGIN(clr) "\033[" clr "m"
#define _CLR_BEGIN2(clr1, clr2) "\033[" #clr1 ";" #clr2 "m"
#define _CLR_END "\033[0m"

#define _DEBUG_PRINT(...) fprintf(stderr, __VA_ARGS__);

#define print_error(...)                                             \
    {                                                                \
        _DEBUG_PRINT("[" _CLR_BEGIN(CC_RED) "ERROR" _CLR_END "]: "); \
        _DEBUG_PRINT(__VA_ARGS__);                                   \
    }
#define print_warning(...)                                                    \
    {                                                                         \
        if (debug_level >= ADL_WARNING) {                                     \
            _DEBUG_PRINT("[" _CLR_BEGIN(CC_YELLOW) "WARNING" _CLR_END "]: "); \
            _DEBUG_PRINT(__VA_ARGS__);                                        \
        }                                                                     \
    }
#define print_info(...)                \
    {                                  \
        if (debug_level >= ADL_INFO) { \
            _DEBUG_PRINT(__VA_ARGS__); \
        }                              \
    }
#define print_debug(...)                \
    {                                   \
        if (debug_level >= ADL_DEBUG) { \
            _DEBUG_PRINT(__VA_ARGS__);  \
        }                               \
    }

const char *get_ext(const char *filename, size_t length = 0);
const char *get_ext(const string &filename, size_t length = 0);

/* Check if the file is exist */
bool is_exist(const char *filename);
/* Check if the file is exist */
bool is_exist(const string &filename);

// Get date string, format: yyyy/mm/dd
void get_date(char *date);

bool all_nums(const char *s);

// dp should be longer than dp[(len1+1)*(len2+1)]
size_t edit_distance(const char *s1, size_t len1, const char *s2, size_t len2);
size_t edit_distance(const string &source, const string &target);


#endif  //_AUTO_SCAN_COMMON_H_
