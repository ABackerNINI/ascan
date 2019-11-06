#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <algorithm>
#include <cassert>
#include <iostream>

#include "ascan.h"
#include "mfile.h"
#include "common.h"

using std::min;

enum OPT_TYPE {
    OT_ALL_SECS = 'a',  // -a
    OT_BUILD = 'b',     // -b
    OT_FORCE = 'f',     // -f
    OT_HELP = 'h',      // -h,--help
    OT_VER = 'v',       // -v,--ver
    OT_DEBUG = 200,     // --debug
    OT_CC,              // --cc
    OT_CXX,             // --cxx
    OT_CFLAGS,          // --cflags
    OT_CXXFLAGS,        // --cxxflags
};

struct as_option {
    enum OPT_TYPE type;
    const char *short_opt;
    const char *long_opt;
    const char *arg;
    const char *description;
};

enum HELP_TYPE { HT_ALL = -1, HT_VER = -2 };

const char g_short_opts[] = "abfhv";

static const as_option g_options[] = {
    {OT_ALL_SECS, "a", "all", NULL,
     "Overwrite all sections, ascan will only overwrite the "
     "'Dependencies' section on default"},
    {OT_BUILD, "b", "build", NULL, "Put binaries to 'build' subdirectory"},
    {OT_FORCE, "f", "force", NULL, "Force overwrite"},
    {OT_HELP, "h", "help", NULL, "Print help information"},
    {OT_VER, "v", "ver", NULL, "Print ascan version"},
    {OT_DEBUG, NULL, "debug", "DEBUG_LEVEL",
     "Set debug level: \n\t\t\t- 0: ERROR\n\t\t\t- "
     "1: WARNING\n\t\t\t- 2: INFO\n\t\t\t- 3: DEBUG"},
    {OT_CC, NULL, "cc", "CC", "Set c compiler"},
    {OT_CXX, NULL, "cxx", "CXX", "Set c++ compiler"},
    {OT_CFLAGS, NULL, "cflags", "CFLAGS", "Set c compile flags"},
    {OT_CXXFLAGS, NULL, "cxxflags", "CXXFLAGS", "Set c++ compile flags"},
};

#define OPTS_SIZE (sizeof(g_options) / sizeof(as_option))

#define PRINT_TITLE(title) printf(_CLR_BEGIN(CC_BOLD) "%s\n" _CLR_END, title)
#define PRINT_FLAG(short_opt, long_opt, arg)                            \
    if (short_opt && long_opt) {                                        \
        printf("\t" _CLR_BEGIN(CC_BOLD) "-%s" _CLR_END ", " _CLR_BEGIN( \
                   CC_BOLD) "--%s" _CLR_END,                            \
               short_opt, long_opt);                                    \
    } else if (short_opt) {                                             \
        printf("\t" _CLR_BEGIN(CC_BOLD) "-%s" _CLR_END, short_opt);     \
    } else {                                                            \
        printf("\t" _CLR_BEGIN(CC_BOLD) "--%s" _CLR_END, long_opt);     \
    }                                                                   \
    if (arg) {                                                          \
        printf("=" _CLR_BEGIN(CC_UNDERLINE) "%s" _CLR_END, arg);        \
    }                                                                   \
    printf("\n");
#define PRINT_DESC(desc) printf("\t\t%s.\n\n", desc)
#define PRINT(msg) printf("\t%s\n\n", msg)

static int find_opt(enum OPT_TYPE type) {
    for (int i = 0, n = OPTS_SIZE; i < n; ++i) {
        if (g_options[i].type == type) {
            return i;
        }
    }
    return -1;
}

static int find_opt(const char *opt) {
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
        return -1;
    }

    size_t min_dis = INT32_MAX;
    int min_ind = -1;
    for (int i = 0, n = OPTS_SIZE; i < n; ++i) {
        size_t dis1, dis2, dis, len2;
        if (g_options[i].short_opt) {
            len2 = strlen(g_options[i].short_opt);
            dis1 = edit_distance(opt, len1, g_options[i].short_opt, len2);
            print_debug("'%.*s' '%s' edit_distance: %d\n", (int)len1, opt,
                        g_options[i].short_opt, (int)dis1);
            if (dis1 >= len1 || dis1 >= len2) {
                dis1 = INT32_MAX;
            }
        }

        if (g_options[i].long_opt) {
            len2 = strlen(g_options[i].long_opt);
            dis2 = edit_distance(opt, len1, g_options[i].long_opt, len2);
            print_debug("'%.*s' '%s' edit_distance: %d\n", (int)len1, opt,
                        g_options[i].long_opt, (int)dis2);
            if (dis2 >= len1 || dis2 >= len2) {
                dis2 = INT32_MAX;
            }
        }

        dis = min(dis1, dis2);
        if (dis < min_dis) {
            min_dis = dis;
            min_ind = i;
        }
    }
    return min_ind;
}

/*==========================================================================*/

ascan::ascan(int argc, char **argv) {
    m_cfg.cc = CONFIG_DEFAULT_CC;
    m_cfg.cxx = CONFIG_DEFAULT_CXX;
    m_cfg.cflag = CONFIG_DEFAULT_CFLAG;
    m_cfg.cxxflag = CONFIG_DEFAULT_CXXFLAG;

    m_flag_a = false;
    m_flag_b = false;
    m_flag_f = false;

    m_proceed = parse_cmd_args(argc, argv);

    char dir[BUFSIZ];
    getcwd(dir, BUFSIZ);
    m_cwd = string(dir);
}

int ascan::start() {
    if (!m_proceed) {
        return EXIT_FAILURE;
    }

    if (!test_makefile()) {
        return EXIT_SUCCESS;
    }
    m_cfiles = recursion_scan_dir_c_cxx_files(m_cwd);

    match_c_cpp_includes();
    associate_header();

    mfile mf(m_makefile, m_cfiles, m_cfg, m_flag_a);

    return mf.output();
}

/*==========================================================================*/

bool ascan::parse_cmd_args(int argc, char **argv) {
    // -a: all sections
    // -b: build
    // -f: force overwrite
    // -cc gcc|g++|cc: set c compiler, default: gcc
    // -cxx g++: set c++ compiler, default: g++
    // -cflag "": set c flags, default: -W -Wall -lm
    // -cxxflag "" set c++ flags, default: -W -Wall

    static struct option long_opts[15]; /*= {
        {"all", no_argument, NULL, OT_ALL_SECS},
        {"build", no_argument, NULL, OT_BUILD},
        {"force", no_argument, NULL, OT_FORCE},
        {"help", no_argument, NULL, OT_HELP},
        {"d", required_argument, NULL, OT_HELP},
        {"cc", required_argument, NULL, OT_CC},
        {"cxx", required_argument, NULL, OT_CXX},
        {"cflag", required_argument, NULL, OT_CFLAGS},
        {"cxxflag", required_argument, NULL, OT_CXXFLAGS},
        {NULL, 0, NULL, 0}};*/

    int los = 0;  // long options size
    for (int i = 0, n = OPTS_SIZE; i < n; ++i) {
        if (g_options[los].long_opt) {
            long_opts[los].name = g_options[i].long_opt;
            long_opts[los].has_arg =
                g_options[i].arg ? required_argument : no_argument;
            long_opts[los].flag = NULL;
            long_opts[los].val = g_options[i].type;
            ++los;
        }
    }
    long_opts[los] = {NULL, 0, NULL, 0};
    print_debug("long options size: %d\n", los);
    assert(los < 15);

    m_flag_f = false;
    opterr = 0;  // do NOT print error message

    int opt, long_ind;
    int help = 0, err = 0, last_optind = -1;  // last unrecognized argument
    int opt_idx;
    while ((opt = getopt_long(argc, argv, g_short_opts, long_opts,
                              &long_ind)) != -1) {
#if (DEBUG)
        printf("opt = %d\t\t", opt);
        printf("optarg = %s\t\t", optarg);
        printf("optind = %d\t\t", optind);
        printf("argv[optind] = %s\t\t", argv[optind]);
        printf("long_index = %d\n", long_ind);
#endif

        switch (opt) {
            case OT_ALL_SECS:
                m_flag_a = true;
                break;
            case OT_BUILD:
                m_flag_b = true;
                break;
            case OT_FORCE:
                m_flag_f = true;
                break;
            case OT_HELP:
                help = 1;
                opt_idx = HT_ALL;
                goto END;
                // break;
            case OT_VER:
                help = 2;
                opt_idx = HT_VER;
                goto END;
                // break;
            case OT_DEBUG:
                if (!all_nums(optarg)) {
                    ++err;
                }
                if (err == 0) {
                    debug_level = atoi(optarg);
                    print_debug("debug_level: %d\n", debug_level);
                }
                if (err || debug_level < ADL_ERROR || debug_level > ADL_DEBUG) {
                    print_error("wrong debug level value.\n");
                    ++err;
                    help = 3;
                    opt_idx = find_opt(OT_DEBUG);
                    goto END;
                }
                break;
            case OT_CC:
                m_cfg.cc = string(optarg);
                break;
            case OT_CXX:
                m_cfg.cxx = string(optarg);
                break;
            case OT_CFLAGS:
                m_cfg.cflag = string(optarg);
                break;
            case OT_CXXFLAGS:
                m_cfg.cxxflag = string(optarg);
                break;
            default:

                opt_idx = find_opt(OPT_TYPE(optopt));
                if (opt_idx != -1) {
                    if ((optarg && !g_options[opt_idx].arg) ||
                        (!optarg && g_options[opt_idx].arg)) {
                        if (optarg) {
                            print_error("unexpected option argument, '%s'.\n",
                                        optarg);
                        } else {
                            print_error("missing option argument, '%s'.\n",
                                        g_options[opt_idx].arg);
                        }
                        help = 10;
                        break;
                    }
                }
                if (last_optind != optind && argv[optind - 1]) {
                    print_error("unrecognized option: '%s'\n",
                                argv[optind - 1]);

                    opt_idx = find_opt(argv[optind - 1]);
                    if (opt_idx != -1) {
                        printf("\tDo you mean \"");
                        const char *short_opt = g_options[opt_idx].short_opt;
                        const char *long_opt = g_options[opt_idx].long_opt;
                        if (short_opt && long_opt) {
                            printf(_CLR_BEGIN(
                                       CC_BOLD) "-%s" _CLR_END
                                                ", " _CLR_BEGIN(
                                                    CC_BOLD) "--%s" _CLR_END,
                                   short_opt, long_opt);
                        } else if (short_opt) {
                            printf(_CLR_BEGIN(CC_BOLD) "-%s" _CLR_END,
                                   short_opt);
                        } else {
                            printf(_CLR_BEGIN(CC_BOLD) "--%s" _CLR_END,
                                   long_opt);
                        }
                        printf("\"?\n\n");
                        help = 4;
                    } else {
                        printf(
                            "Type 'ascan --help' to see useful "
                            "informations.\n");
                    }

                    ++err;
                    goto END;
                }
                break;
        }
        last_optind = optind;
    }

END:
    if (help) {
        print_help(opt_idx);
    }

    return help == 0 && err == 0;
}

void ascan::print_help(int ind) const {
    //     OT_ALL = 'a',    // -a
    //     OT_BUILD = 'b',  // -b
    //     OT_FORCE = 'f',  // -f
    //     OT_HELP = 200,   // --help
    //     OT_DEBUG,        // --d
    //     OT_CC,           // --cc
    //     OT_CXX,          // --cxx
    //     OT_CFLAGS,       // --cflags
    //     OT_CXXFLAGS,     // --cxxflags
    //     OT_ALL_SECS

    // static const char *help_msg[] = {
    //     "-a: Overwrite all sections, ascan will only overwrite the "
    //     "'# Dependencies' section on default",
    //     "-b: Put binaries to 'build' subdirectory",
    //     "-f: Force overwrite",
    //     "--d: Set debug level: \n\t\t- 0: ERROR\n\t\t- "
    //     "1: WARNING\n\t\t- 2: INFO\n\t\t- 3: DEBUG",
    //     "--cc: Set c compiler",
    //     "--cxx: Set c++ compiler",
    //     "--cflags: Set c compile flags",
    //     "--cxxflags: Set c++ compile flags"};

    static const char *desc =
        "Auto scan c/c++ project and create simple makefile.\n\n"
        "\tAscan is suitable for c/c++ projects are:\n"
        "\t\t1. Simple structured that all source codes are in one "
        "directory.\n"
        "\t\t2. Source codes are `.h` or `.c` or `.cpp`.\n"
        "\t`cd` to the project directory and run `ascan`.";

    if (ind == HT_ALL) {
        PRINT_TITLE("NAME");
        PRINT("ascan - auto scan");

        PRINT_TITLE("SYNOPSIS");
        PRINT("ascan [OPTION]");

        PRINT_TITLE("DESCRIPTION");
        PRINT(desc);
        PRINT(
            "Mandatory arguments to long options are mandatory for short "
            "options too.");

        for (int i = 0, n = OPTS_SIZE; i < n; ++i) {
            PRINT_FLAG(g_options[i].short_opt, g_options[i].long_opt,
                       g_options[i].arg);
            PRINT_DESC(g_options[i].description);
        }
        // printf("\n");
    } else if (ind == HT_VER) {
        printf("ascan version: " AS_VERSION "\n");
    } else {
        PRINT_TITLE("OPTION");
        PRINT(
            "Mandatory arguments to long options are mandatory for short "
            "options too.");
        PRINT_FLAG(g_options[ind].short_opt, g_options[ind].long_opt,
                   g_options[ind].arg);
        PRINT_DESC(g_options[ind].description);
    }
}

bool ascan::test_makefile() {
    string makefile1 = m_cwd + "/Makefile";
    string makefile2 = m_cwd + "/makefile";

    int exist = 0;
    if (is_exist(makefile1)) {
        m_makefile = makefile1;
        exist = 1;
    } else if (is_exist(makefile2)) {
        m_makefile = makefile2;
        exist = 2;
    } else {
        // default makefile name
        m_makefile = makefile1;
    }

    if (!m_flag_f && exist) {
        printf(
            "There is already one %s in \"%s\", "
            "overwrite it? [y/N] ",
            (exist == 1 ? "Makefile" : "makefile"), m_cwd.c_str());

        char cmd[BUFSIZ];
        if (fgets(cmd, BUFSIZ, stdin)) {
            if (strcasecmp(cmd, "y\n") != 0 && strcasecmp(cmd, "yes\n") != 0) {
                return false;
            }
        } else {
            return false;
        }
    }

    return true;
}

void ascan::match_c_cpp_includes() {
    for (auto file = m_cfiles.begin(); file != m_cfiles.end(); ++file) {
        if (file->file_type() == cfile::C || file->file_type() == cfile::CPP) {
            print_info("%s\n", file->filename().c_str());
            file->match_includes(m_cfiles);
            for (auto include = file->includes().begin();
                 include != file->includes().end(); ++include) {
                (*include)->match_includes(m_cfiles);
            }
        }
    }
}

void ascan::associate_header() {
    for (auto file = m_cfiles.begin(); file != m_cfiles.end(); ++file) {
        if (file->file_type() == cfile::C || file->file_type() == cfile::CPP) {
            file->associate_header(m_cfiles);
        }
    }
}

/*==========================================================================*/

int main(int argc, char **argv) {
    ascan ascan(argc, argv);

    return ascan.start();
}
