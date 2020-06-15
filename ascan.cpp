#include "ascan.h"

#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <algorithm>
#include <cassert>
#include <iostream>

#include "mfile.h"
#include "common.h"

using std::min;

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

/*==========================================================================*/

ascan::ascan(int argc, char **argv) {
    m_flags = 0;

    m_proceed = parse_cmd_args(argc, argv);

    // char dir[BUFSIZ];
    // getcwd(dir, BUFSIZ);
    // m_cwd = string(dir);
}

int ascan::start() {
    if (!m_proceed) {
        return EXIT_FAILURE;
    }

    if (!test_makefile()) {
        return EXIT_SUCCESS;
    }
    m_cfiles = recursion_scan_dir_c_cxx_files(".");

    match_c_cxx_includes();
    associate_header();

    mfile mf(m_makefile, m_cfiles, m_cfg, m_flags);

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

    const char *short_opts = m_options.get_short_opts();
    const struct option *long_opts = m_options.get_long_opts();

    opterr = 0;  // do NOT print error message

    int opt, long_ind;
    enum HELP_TYPE help = HT_NONE;
    int err = 0, last_optind = -1;  // last unrecognized argument
    const options::as_option *option;
    while ((opt = getopt_long(argc, argv, short_opts, long_opts, &long_ind)) !=
           -1) {
#if (DEBUG)
        // printf("opt = %d\t\t", opt);
        printf("opt = ");
        m_options.print_opt_type((options::OPT_TYPE)opt);
        printf("\t\t");
        printf("optarg = %s\t\t", optarg);
        printf("optind = %d\t\t", optind);
        printf("argv[optind] = %s\t\t", argv[optind]);
        printf("long_index = %d\n", long_ind);
#endif

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
            case options::OT_VER:
                help = HT_VER;
                goto END;
            case options::OT_DEBUG:
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
                    help = HT_SPECIFIC;
                    option = m_options.find_opt(options::OT_DEBUG);
                    goto END;
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
                        help = HT_ALL;
                        break;
                    }
                }
                if (last_optind != optind && argv[optind - 1]) {
                    print_error("unrecognized option: '%s'\n",
                                argv[optind - 1]);

                    // find similar option
                    option = m_options.find_similar_opt(argv[optind - 1]);
                    if (option) {
                        printf("\tDo you mean \"");
                        const char *short_opt = option->short_opt;
                        const char *long_opt = option->long_opt;
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
                        help = HT_SPECIFIC;
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

    // TODO add option: -o output-file

END:
    if (help != HT_NONE) {
        print_help(help, option);
    }

    return help == 0 && err == 0;
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
        PRINT(
            "Mandatory arguments to long options are mandatory for short "
            "options too.");

        size_t n;
        const options::as_option *options = m_options.get_as_opts(&n);
        for (unsigned int i = 0; i < n; ++i) {
            PRINT_FLAG(options[i].short_opt, options[i].long_opt,
                       options[i].arg);
            PRINT_DESC(options[i].description);
        }

        PRINT_TITLE("AUTHOR");
        PRINT("Written by ABacker.");

        PRINT_TITLE("REPORTING BUGS");
        PRINT("<" AS_URL ">");

        PRINT_TITLE("COPYRIGHT");
        PRINT(
            "License GPLv3+: GNU GPL version 3 or later "
            "<http://gnu.org/licenses/gpl.html>.");

        // PRINT_TITLE("SEE ALSO");
    } else if (help == HT_VER) {
        printf("ascan version: " AS_VERSION "\n");
    } else if (help == HT_SPECIFIC) {
        PRINT_TITLE("OPTION");
        PRINT(
            "Mandatory arguments to long options are mandatory for short "
            "options too.");
        PRINT_FLAG(option->short_opt, option->long_opt, option->arg);
        PRINT_DESC(option->description);
    }
}

bool ascan::test_makefile() {
    string makefile1 = "Makefile";
    string makefile2 = "makefile";

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
