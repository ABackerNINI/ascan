#include <unistd.h>
#include <string.h>
#include <algorithm>
#include "common.h"

int debug_level = ADL_DEBUG;  // ADL_ERROR;

const char *get_ext(const char *filename, size_t length) {
    if (length == 0) {
        length = strlen(filename);
    }

    for (const char *p = filename + length - 1; length > 0; --p, --length) {
        if (*p == '.') {
            return p;
        }
    }
    return NULL;
}

const char *get_ext(const string &filename, size_t length) {
    return get_ext(filename.c_str(), length);
}

bool is_exist(const char *filename) { return access(filename, F_OK) == 0; }

bool is_exist(const string &filename) { return is_exist(filename.c_str()); }

void get_date(char *date) {
    time_t t = time(0);
    tm *ltm = localtime(&t);
    sprintf(date, "%4d/%02d/%02d", ltm->tm_year + 1900, ltm->tm_mon + 1,
            ltm->tm_mday);
}

bool all_nums(const char *s) {
    if (*s == '\0') {
        return false;
    }
    
    while (*s && isalnum(*s)) {
        ++s;
    }
    return *s == '\0';
}

#define min(a, b) ((a < b) ? a : b)

size_t edit_distance(const char *s1, size_t len1, const char *s2, size_t len2) {
#define DP(i, j) dp[(i)*len2 + (j)]
    size_t *dp = new size_t[(len1 + 1) * (len2 + 1)];
    for (size_t i = 0; i <= len1; ++i) {
        DP(i, 0) = i;
    }
    for (size_t i = 0; i <= len2; ++i) {
        DP(0, i) = i;
    }

    size_t flag;
    for (size_t i = 1; i <= len1; ++i) {
        for (size_t j = 1; j <= len2; ++j) {
            flag = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            DP(i, j) = min(DP(i - 1, j) + 1,
                           min(DP(i, j - 1) + 1, DP(i - 1, j - 1) + flag));
        }
    }

    size_t ret = DP(len1, len2);

    delete[] dp;

    return ret;
}

size_t edit_distance(const string &s1, const string &s2) {
    return edit_distance(s1.c_str(), s1.length(), s2.c_str(), s2.length());
}
