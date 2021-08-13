#include "ascan.h"

#include <algorithm>
#include <cassert>
#include <getopt.h>
#include <iostream>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "debug.h"
#include "mfile.h"

int debug_level = DBG_LVL_DEBUG;

#define PRINT_TITLE(title)                                                     \
    if (isatty(STDOUT_FILENO)) {                                               \
        printf(CC(CC_BRIGHT, "%s") "\n", title);                               \
    } else {                                                                   \
        printf("%s\n", title);                                                 \
    }

#define PRINT_OPTION(short_opt, long_opt, arg)                                 \
    if (isatty(STDOUT_FILENO)) {                                               \
        if (short_opt && long_opt) {                                           \
            printf("\t" CC(CC_BRIGHT, "-%c") ", " CC(CC_BRIGHT, "--%s"),       \
                   short_opt, long_opt);                                       \
        } else if (short_opt) {                                                \
            printf("\t" CC(CC_BRIGHT, "-%c"), short_opt);                      \
        } else {                                                               \
            printf("\t" CC(CC_BRIGHT, "--%s"), long_opt);                      \
        }                                                                      \
        if (arg) {                                                             \
            printf("=" CC(CC_UNDERSCORE, "%s"), arg);                          \
        }                                                                      \
    } else {                                                                   \
        if (short_opt && long_opt) {                                           \
            printf("\t-%c, --%s", short_opt, long_opt);                        \
        } else if (short_opt) {                                                \
            printf("\t-%c", short_opt);                                        \
        } else {                                                               \
            printf("\t--%s", long_opt);                                        \
        }                                                                      \
        if (arg) {                                                             \
            printf("=%s", arg);                                                \
        }                                                                      \
    }                                                                          \
    printf("\n");

#define PRINT_DESC(desc) printf("\t\t%s.\n\n", desc)

#define PRINT(msg) printf("\t%s\n\n", msg)

/*==========================================================================*/

ascan::ascan(int argc, char **argv) {
    m_flags = 0;

    m_error = parse_cmd_args(argc, argv);

    // char dir[BUFSIZ];
    // getcwd(dir, BUFSIZ);
    // m_cwd = string(dir);
}

int ascan::start() {
    if (m_error > 0) {
        return EXIT_FAILURE;
    } else if (m_error < 0) {
        return EXIT_SUCCESS;
    }

    if (!test_makefile()) {
        return EXIT_SUCCESS;
    }
    m_cfiles = recursion_scan_dir_c_cxx_files(".");

    match_c_cxx_includes();
    associate_header();

    mfile mf(m_cfiles, m_cfg, m_flags);

    return mf.output();
}

/*==========================================================================*/

int check_debug_level(const char *s) {
    int num;
    if (!all_nums(s)) {
        print_error("debug level must be a number.\n");
        return -1;
    }
    num = atoi(s);
    if (!(DBG_LVL_ERROR <= num && num <= DBG_LVL_EXCESSIVE)) {
        print_error("wrong debug level value.\n");
        return -2;
    }
    debug_level = num;

    return num;
}

int pre_parse_debug_level(int argc, char **argv) {
    int num = debug_level;
    for (int i = 0; i < argc; ++i) {
        if (strncmp(argv[i], "--debug", 7) == 0) {
            if (argv[i][7] == '=') {
                num = check_debug_level(argv[i] + 8);
            } else if (i + 1 < argc) {
                num = check_debug_level(argv[i + 1]);
            }
            if (num >= 0) {
                print_debug("pre_parse_debug_level %d\n", debug_level);
            }
            break;
        }
    }
    return num;
}

int ascan::parse_cmd_args(int argc, char **argv) {
    // -a: all sections
    // -b: build
    // -f: force overwrite
    // -cc gcc|g++|cc: set c compiler, default: gcc
    // -cxx g++: set c++ compiler, default: g++
    // -cflag "": set c flags, default: -W -Wall -lm
    // -cxxflag "" set c++ flags, default: -W -Wall

    const char *short_opts = m_options.get_short_opts();
    const struct option *long_opts = m_options.get_long_opts();

    opterr = 0; // do NOT print error message

    int opt, long_ind;
    int err = 0;
    enum HELP_TYPE help = HT_NONE;
    const options::as_option *option;

    //* We need to use print_msgdump() or print_debug() before --debug
    //* option has been parsed! or we just parse it here.
    if (pre_parse_debug_level(argc, argv) < 0) {
        goto ERROR_DEBUG_LEVEL;
    }

    print_debug("parsing arguments\n");
    while ((opt = getopt_long(argc, argv, short_opts, long_opts, &long_ind)) !=
           -1) {
        print_debug_ex("\topt = ");
        stmt_debug(
            int col = -8; option = m_options.find_opt((options::OPT_TYPE)opt);
            if (option) {
                if (option->short_opt) {
                    print_debug_ex("%*c\t", col, option->short_opt);
                } else if (option->long_opt) {
                    print_debug_ex("%*s\t", col, option->long_opt);
                }
            } else { print_debug_ex("%*c\t", col, '?'); });

        print_debug_ex("optopt = %c\t", optopt ? optopt : ' ');

        stmt_debug(
            if (optarg) { print_debug_ex("optarg = |%s|\t\n", optarg); } else {
                print_debug_ex("optarg = %s\t\n", optarg);
            });

        // print_debug_ex("optind = %d\t\n", optind);
        // print_debug_ex("argv[optind] = %s\t\n", argv[optind]);
        // print_debug_ex("long_index = %d\n", long_ind);

        switch (opt) {
        case options::OT_ALL_SECS:
            m_flags |= OPTION_A;
            break;
        case options::OT_BUILD:
            m_flags |= OPTION_B;
            break;
        case options::OT_FORCE:
            m_flags |= OPTION_F;
            break;
        case options::OT_G:
            m_flags |= OPTION_G;
            break;
        case options::OT_HELP:
            help = HT_ALL;
            goto END;
        case options::OT_OUTPUT:
            m_flags |= OPTION_O;
            m_cfg.output = string(optarg);
            break;
        case options::OT_VER:
            help = HT_VER;
            goto END;
        case options::OT_DEBUG:
            if (check_debug_level(optarg) < 0) {
                goto ERROR_DEBUG_LEVEL;
            }
            break;
        case options::OT_CC:
            m_cfg.v_cc = string(optarg);
            break;
        case options::OT_CXX:
            m_cfg.v_cxx = string(optarg);
            break;
        case options::OT_CFLAGS:
            m_cfg.v_cflag = string(optarg);
            break;
        case options::OT_CXXFLAGS:
            m_cfg.v_cxxflag = string(optarg);
            break;
        default:
            // find the option in case of missing argument
            option = m_options.find_opt(options::OPT_TYPE(optopt));
            if (option) {
                if ((optarg && !option->arg) || (!optarg && option->arg)) {
                    if (optarg) {
                        // control should never reach here
                        print_error("unexpected option argument, '%s'.\n",
                                    optarg);
                    } else {
                        print_error("missing option argument, '%s'.\n",
                                    option->arg);
                    }
                    help = HT_SPECIFIC;
                }
            } else {
                if (optopt != '\0') {
                    print_error("unrecognized option: '%c'\n", optopt);
                } else {
                    // find similar option
                    option = m_options.find_similar_opt(argv[optind - 1]);
                    if (option) {
                        printf("\tDo you mean \"");
                        char short_opt = option->short_opt;
                        const char *long_opt = option->long_opt;
                        if (short_opt && long_opt) {
                            printf(
                                CC_BEGIN(CC_BRIGHT) "-%c" CC_END ", " CC_BEGIN(
                                    CC_BRIGHT) "--%s" CC_END,
                                short_opt, long_opt);
                        } else if (short_opt) {
                            printf(CC_BEGIN(CC_BRIGHT) "-%c" CC_END, short_opt);
                        } else {
                            printf(CC_BEGIN(CC_BRIGHT) "--%s" CC_END, long_opt);
                        }
                        printf("\"?\n\n");
                        help = HT_SPECIFIC;
                    } else {
                        printf("Type 'ascan --help' to see useful "
                               "informations.\n");
                    }
                }

                ++err;
                goto END;
            }
            break;
        }
    }

    // TODO add ascan [options] [main files]
    print_debug("Start file:\n");
    stmt_debug(if (optind == argc) { print_debug_ex("\tnot specified\n"); });
    for (int i = optind; i < argc; ++i) {
        m_cfg.start_files.push_back(argv[i]);
        print_debug_ex("\t|%s|\n", argv[i]);
    }

END:
    if (help != HT_NONE) {
        print_help(help, option);
    }

    return err == 0 ? -help : err;

ERROR_DEBUG_LEVEL:
    ++err;
    help = HT_SPECIFIC;
    option = m_options.find_opt(options::OT_DEBUG);

    goto END;
}

void ascan::print_help(enum HELP_TYPE help,
                       const options::as_option *option) const {
    static const char *desc =
        "Ascan will scan the c/c++ project and create simple makefile.\n\n"
        "\tAscan is suitable for c/c++ projects that are:\n"
        "\t\t1. Simple structured that all source codes are in one "
        "directory.\n"
        "\t\t2. Source codes are `.h` or `.c` or `.cpp` or `.cc`.\n"
        "\t`cd` to the project directory and run `ascan`.";

    if (help == HT_ALL) {
        PRINT_TITLE("NAME");
        PRINT("ascan - auto scan");

        PRINT_TITLE("SYNOPSIS");
        PRINT("ascan [OPTION]...");

        PRINT_TITLE("DESCRIPTION");
        PRINT(desc);
        PRINT("Mandatory arguments to long options are mandatory for short "
              "options too.");

        size_t n;
        const options::as_option *options = m_options.get_as_opts(&n);
        for (unsigned int i = 0; i < n; ++i) {
            PRINT_OPTION(options[i].short_opt, options[i].long_opt,
                         options[i].arg);
            PRINT_DESC(options[i].description);
        }

        PRINT_TITLE("AUTHOR");
        PRINT("Written by ABacker.");

        PRINT_TITLE("REPORTING BUGS");
        PRINT("<" AS_URL ">");

        PRINT_TITLE("COPYRIGHT");
        PRINT("License GPLv3+: GNU GPL version 3 or later "
              "<http://gnu.org/licenses/gpl.html>.");

        // PRINT_TITLE("SEE ALSO");
    } else if (help == HT_VER) {
        printf("ascan version: " AS_VERSION "\n");
    } else if (help == HT_SPECIFIC) {
        PRINT_TITLE("OPTION");
        PRINT("Mandatory arguments to long options are mandatory for short "
              "options too.");
        PRINT_OPTION(option->short_opt, option->long_opt, option->arg);
        PRINT_DESC(option->description);
    }
}

bool ascan::test_makefile() {
    int exist = 0;
    if (m_flags & OPTION_O) {
        if (is_exist(m_cfg.output)) {
            exist = 3;
        }
    } else {
        string makefile1 = "Makefile";
        string makefile2 = "makefile";

        if (is_exist(makefile1)) {
            m_cfg.output = makefile1;
            exist = 1;
        } else if (is_exist(makefile2)) {
            m_cfg.output = makefile2;
            exist = 2;
        }
    }

    if (!(m_flags & OPTION_F) && exist) {
        printf("There is already one %s, overwrite it? [y/N] ",
               (exist == 1 ? "Makefile" : "makefile"));

        char cmd[BUFSIZ];
        if (fgets(cmd, BUFSIZ, stdin)) {
            if (strcasecmp(cmd, "y\n") != 0 && strcasecmp(cmd, "yes\n") != 0) {
                return false;
            }
        } else {
            return false;
        }
    }

    if (!exist) {
        m_flags |= OPTION_A;
    }

    return true;
}

void ascan::match_c_cxx_includes() {
    for (auto file = m_cfiles.begin(); file != m_cfiles.end(); ++file) {
        if (file->is_source()) {
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
        if (file->is_source()) {
            file->associate_header(m_cfiles);
        }
    }
}

/*==========================================================================*/

int main(int argc, char **argv) {
    ascan ascan(argc, argv);

    return ascan.start();
}
