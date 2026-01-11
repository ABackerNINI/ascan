#include "ascan.h"

#include "debug.h"
#include "mfilev3.h"
#include "mfilev4.h"
#include "settings.h"
#include "utils.h"
#include <cstring>
#include <filesystem>
#include <getopt.h>
#include <unistd.h>

using namespace std;

int debug_level = DBG_LVL_DEBUG;

/*==========================================================================*/

ascan::ascan() {}

int ascan::start(int argc, char **argv) {
    Settings settings;

    int ret;
    if ((ret = settings.parse_argv(argc, argv)) != 0) {
        return ret; // error in parsing command line arguments
    }

    if (settings.flag_help_ || settings.flag_version_) {
        return EXIT_SUCCESS;
    }

    if (!test_makefile(settings.flag_force_, !settings.option_output_.empty())) {
        return EXIT_SUCCESS;
    }

    m_cfiles = recursion_scan_dir_c_cxx_files(settings.option_src_dir_, settings.flag_recursive_);

    match_c_cxx_includes();
    match_starter_files(settings.main_files_);
    print_cfiles();
    associate_header();

    if (settings.temp_version_ == 1) {
        // TODO:
    } else if (settings.temp_version_ == 2) {
        // TODO:
    } else if (settings.temp_version_ == 3) {
        MFileV3 mf(settings, m_cfiles);
        return mf.output();
    } else if (settings.temp_version_ == 4) {
        MFileV4 mf(settings, m_cfiles);
        return mf.output();
    }

    print_error("unknown template version\n");
    print_error("available versions: 1, 2, 3, 4\n");

    return 1;
}

/*==========================================================================*/

/*
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
 */

/*
int ascan::parse_cmd_args(int argc, char **argv) {
    // -a: all sections
    // -b: build
    // -f: force overwrite
    // -cc gcc|g++|cc: set c compiler, default: gcc
    // -cxx g++: set c++ compiler, default: g++
    // -cflags "": set c flags, default: -W -Wall -lm
    // -cxxflags "" set c++ flags, default: -W -Wall

    const char *short_opts = m_options.get_short_opts();
    const struct option *long_opts = m_options.get_long_opts();

    opterr = 0; // do NOT print error message

    int opt, long_ind;
    int err = 0;
    enum HELP_TYPE help = HT_NONE;
    const options::as_option *option = nullptr;

    //* We need to use print_msgdump() or print_debug() before --debug
    //* option has been parsed! or we just parse it here.
    if (pre_parse_debug_level(argc, argv) < 0) {
        goto ERROR_DEBUG_LEVEL;
    }

    print_debug("parsing arguments\n");
    while ((opt = getopt_long(argc, argv, short_opts, long_opts, &long_ind)) != -1) {
        print_debug_ex("\topt = ");
        stmt_debug(
            int col = -8; option = m_options.find_opt((options::OPT_TYPE)opt); if (option) {
                if (option->short_opt) {
                    print_debug_ex("%*c\t", col, option->short_opt);
                } else if (option->long_opt) {
                    print_debug_ex("%*s\t", col, option->long_opt);
                }
            } else { print_debug_ex("%*c\t", col, '?'); });

        print_debug_ex("optopt = %c\t", optopt ? optopt : ' ');

        stmt_debug(
            if (optarg) { print_debug_ex("optarg = |%s|\t\n", optarg); } else {
                print_debug_ex("optarg = (null)\t\n");
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
            m_cfg.set(CONFIG_CC, optarg);
            break;
        case options::OT_CXX:
            m_cfg.set(CONFIG_CXX, optarg);
            break;
        case options::OT_CFLAGS:
            m_cfg.set(CONFIG_CFLAGS, optarg);
            break;
        case options::OT_CXXFLAGS:
            m_cfg.set(CONFIG_CXXFLAGS, optarg);
            break;
        default:
            // find the option in case of missing argument
            option = m_options.find_opt(options::OPT_TYPE(optopt));
            if (option) {
                if ((optarg && !option->arg) || (!optarg && option->arg)) {
                    if (optarg) {
                        // control should never reach here
                        print_error("unexpected option argument, '%s'.\n", optarg);
                    } else {
                        print_error("missing option argument, '%s'.\n", option->arg);
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
                            printf(CC_BEGIN(CC_BRIGHT) "-%c" CC_END ", " CC_BEGIN(CC_BRIGHT) "--%s" CC_END, short_opt,
                                   long_opt);
                        } else if (short_opt) {
                            printf(CC_BEGIN(CC_BRIGHT) "-%c" CC_END, short_opt);
                        } else {
                            printf(CC_BEGIN(CC_BRIGHT) "--%s" CC_END, long_opt);
                        }
                        printf("\"?\n\n");
                        help = HT_SPECIFIC;
                    } else {
                        printf("Type 'ascan --help' to see useful informations.\n");
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
 */

/*

 */

bool ascan::test_makefile(bool force, bool output_specified) {
    Config config;

    int exist = 0;
    if (output_specified) {
        if (is_exist(config.output)) {
            exist = 3;
        }
    } else {
        string makefile1 = "Makefile";
        string makefile2 = "makefile";

        if (is_exist(makefile1)) {
            config.output = makefile1;
            exist = 1;
        } else if (is_exist(makefile2)) {
            config.output = makefile2;
            exist = 2;
        }
    }

    if (!force && exist) {
        printf("There is already one %s, overwrite it? [y/N] ", (exist == 1 ? "Makefile" : "makefile"));

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

// Set 'have_main_func' for cfiles that are starter files specified by command line arguments.
void ascan::match_starter_files(const std::vector<std::string> &start_files) {
    for (auto &sfile : start_files) {
        for (auto &cfile : m_cfiles) {
            if (cfile.is_source()) {
                filesystem::path p1(cfile.path());
                filesystem::path p2(sfile);
                if (filesystem::relative(p1) == filesystem::relative(p2)) {
                    cfile.set_have_main_func(true);
                    break;
                }
            }
        }
    }
}

void ascan::match_c_cxx_includes() {
    // Match includes for cfiles
    for (auto &cfile : m_cfiles) {
        if (cfile.is_source()) {
            cfile.match_includes_and_detect_main(m_cfiles);
            for (auto &inc : cfile.includes()) {
                inc->match_includes_and_detect_main(m_cfiles);
            }
        }
    }
}

void ascan::print_cfiles() const {
    // Print cfiles and their includes
    for (auto &cfile : m_cfiles) {
        if (cfile.is_source()) {
            try {
                filesystem::path p1(cfile.path());
                print_debug("%s", filesystem::relative(p1).c_str());
                if (cfile.have_main_func()) {
                    print_debug_ex(" <----- [main]");
                }
                print_debug_ex("\n");
                for (auto &inc : cfile.includes()) {
                    try {
                        filesystem::path p2(inc->path());
                        print_debug_ex("\t|%s|\n", filesystem::relative(p2).c_str());
                    } catch (...) { print_debug_ex("\n"); }
                }
            } catch (...) { print_debug_ex("\n"); }
        }
    }
}

void ascan::associate_header() {
    for (auto &cfile : m_cfiles) {
        if (cfile.is_source()) {
            cfile.associate_header(m_cfiles);
        }
    }
}

/*==========================================================================*/

#include "libs/rang.hpp"
#include <csignal>

// Signal handler for SIGINT (Control-C) to exit the program gracefully.
void signal_handler(int s) {
    (void)s;
    std::cout << std::endl << rang::style::reset << rang::fg::red << rang::style::bold;
    std::cout << "Control-C detected, exiting..." << rang::style::reset << std::endl;
    std::exit(1);
}

// Register signal handler for SIGINT to exit the program gracefully.
void register_signal_handler() {
    // Nice Control-C
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = signal_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, nullptr);
}

int main(int argc, char **argv) {
    // Reset colors at exit to avoid terminal issues after program termination
    std::atexit([]() { std::cout << rang::style::reset; });

    // Handle signals gracefully
    register_signal_handler();

    ascan ascan;

    return ascan.start(argc, argv);
}
