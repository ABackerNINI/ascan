#include <unistd.h>
#include <string.h>
#include "common.h"

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