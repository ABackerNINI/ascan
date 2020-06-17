#include "options.h"

#include <string.h>

#include "common.h"
#include "dbg_print.h"

// const char options::s_short_opts[] = "abfgho:v";

const options::as_option options::s_as_opts[] = {
    {OT_ALL_SECS, 'a', "all", NULL, true,
     "Overwrite all sections, ascan will only overwrite the "
     "'Dependencies' section on default"},
    {OT_BUILD, 'b', "build", NULL, true,
     "Put binaries to 'build' subdirectory, Option '-b' is not implemented "
     "yet!"},
    {OT_FORCE, 'f', "force", NULL, true, "Force overwrite"},
    {OT_G, 'g', NULL, NULL, true, "add '-g' flag to cflags or cxxflags"},
    {OT_HELP, 'h', "help", NULL, true, "Print help information"},
    {OT_OUTPUT, 'o', "output", "OUTPUT_FILE", false,
     "Output to the specified file rather than 'Makefile'"},
    {OT_VER, 'v', "ver", NULL, true, "Print ascan version"},
    {OT_DEBUG, '\0', "debug", "DEBUG_LEVEL", false,
     "Set debug level: \n\t\t\t- 0: ERROR\n\t\t\t- "
     "1: WARNING\n\t\t\t- 2: INFO\n\t\t\t- 3: DEBUG"},
    {OT_CC, '\0', "cc", "CC", false,
     "Set c compiler, default: '" CONFIG_DEFAULT_V_CC "'"},
    {OT_CXX, '\0', "cxx", "CXX", false,
     "Set c++ compiler, default: '" CONFIG_DEFAULT_V_CXX "'"},
    {OT_CFLAGS, '\0', "cflags", "CFLAGS", false,
     "Set c compile flags, default: '" CONFIG_DEFAULT_V_CFLAG "'"},
    {OT_CXXFLAGS, '\0', "cxxflags", "CXXFLAGS", false,
     "Set c++ compile flags, default: '" CONFIG_DEFAULT_V_CXXFLAG "'"},
};

const size_t options::s_as_opt_size = (sizeof(s_as_opts) / sizeof(as_option));

/*==========================================================================*/

options::options() {
    m_long_opts = make_long_opts();
    m_short_opts = make_short_opts();
}

const char *options::get_short_opts() const { return m_short_opts; }

const struct option *options::get_long_opts() const { return m_long_opts; }

const options::as_option *options::get_as_opts(size_t *size) const {
    *size = s_as_opt_size;
    return s_as_opts;
}

char *options::make_short_opts() const {
    char *short_opts = new char[s_as_opt_size * 3 + 1];
    int los = 0, size = 0;  // long options size
    for (int i = 0, n = s_as_opt_size; i < n; ++i) {
        if (s_as_opts[i].short_opt != '\0') {
            short_opts[los++] = s_as_opts[i].short_opt;
            if (s_as_opts[i].arg) {
                short_opts[los++] = ':';
                if (s_as_opts[i].optional) {
                    short_opts[los++] = ':';
                }
            }
            ++size;
        }
    }
    short_opts[los] = '\0';
    print_debug("short options size: %d |%s|\n", size, short_opts);

    return short_opts;
}

struct option *options::make_long_opts() const {
    struct option *long_opts = new option[s_as_opt_size + 1];
    int los = 0;  // long options size
    for (int i = 0, n = s_as_opt_size; i < n; ++i) {
        if (s_as_opts[i].long_opt) {
            long_opts[los].name = s_as_opts[i].long_opt;
            long_opts[los].has_arg =
                s_as_opts[i].arg ? required_argument : no_argument;
            long_opts[los].flag = NULL;
            long_opts[los].val = s_as_opts[i].type;
            ++los;
        }
    }
    long_opts[los] = {NULL, 0, NULL, 0};
    print_debug("long options size: %d\n", los);

    return long_opts;
}

const options::as_option *options::find_opt(enum OPT_TYPE type) const {
    for (int i = 0, n = s_as_opt_size; i < n; ++i) {
        if (s_as_opts[i].type == type) {
            return &s_as_opts[i];
        }
    }
    return NULL;
}

const options::as_option *options::find_similar_opt(const char *opt) const {
    print_debug("%s\n", opt);
    size_t len1 = 0;
    while (*opt == '-') {
        ++opt;
    }

    const char *opt1 = opt;
    while (isalpha(*opt1)) {
        ++len1;
        ++opt1;
    }

    if (len1 == 0) {
        return NULL;
    }

    size_t min_dis = INT32_MAX;
    int min_ind = -1;
    for (int i = 0, n = s_as_opt_size; i < n; ++i) {
        size_t dis, len2;

        if (s_as_opts[i].long_opt) {
            len2 = strlen(s_as_opts[i].long_opt);
            dis = edit_distance(opt, len1, s_as_opts[i].long_opt, len2);
            print_debug("'%.*s' '%s' edit_distance: %d\n", (int)len1, opt,
                        s_as_opts[i].long_opt, (int)dis);
            if (dis > len1 / 2 || dis > len2 / 2) {
                dis = INT32_MAX;
            }
        }

        if (dis < min_dis) {
            min_dis = dis;
            min_ind = i;
        }
    }
    return min_ind == -1 ? NULL : &s_as_opts[min_ind];
}

const char *options::opt_type_to_str(enum OPT_TYPE type) const {
    const char *str;
    switch (type) {
        case OT_ALL_SECS:
            str = "a";
            break;
        case OT_BUILD:
            str = "b";
            break;
        case OT_FORCE:
            str = "f";
            break;
        case OT_G:
            str = "g";
            break;
        case OT_HELP:
            str = "h";
            break;
        case OT_VER:
            str = "v";
            break;
        case OT_DEBUG:
            str = "debug";
            break;
        case OT_CC:
            str = "cc";
            break;
        case OT_CXX:
            str = "cxx";
            break;
        case OT_CFLAGS:
            str = "cflags";
            break;
        case OT_CXXFLAGS:
            str = "cxxflags";
            break;

        default:
            str = "?";
            break;
    }
    return str;
}

options::~options() {
    delete[] m_short_opts;
    delete[] m_long_opts;
}