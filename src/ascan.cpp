#include "ascan.h"

#include "debug.h"
#include "fs.h" // IWYU pragma: keep
#include "mfilev3.h"
#include "mfilev4.h"
#include "settings.h"
#include "utils.h"
#include <cstring>
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

    // If no source directory specified, use current directory
    if (settings.option_src_dir_.empty()) {
        settings.option_src_dir_ = ".";
    }

    // Scan the source directory for C/C++ source files
    auto srcfiles = recursively_scan_dir_c_cxx_files(settings.option_src_dir_, settings.flag_recursive_);

    for (auto &cf : srcfiles) {
        m_cfiles.emplace_back(fs::relative(cf));
    }

    // If no C/C++ source files found, return error
    if (m_cfiles.empty()) {
        print_error("no C/C++ source files found!\n");
        return EXIT_FAILURE;
    }

    match_includes_and_detect_main();
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

void ascan::match_includes_and_detect_main() {
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
                fs::path p1(cfile.path());
                print_debug("%s", fs::relative(p1).c_str());
                if (cfile.have_main_func()) {
                    print_debug_ex(" <----- [main]");
                }
                print_debug_ex("\n");
                for (auto &inc : cfile.includes()) {
                    try {
                        fs::path p2(inc->path());
                        print_debug_ex("\t|%s|\n", fs::relative(p2).c_str());
                    } catch (...) { print_debug_ex("\n"); }
                }
            } catch (...) { print_debug_ex("\n"); }
        }
    }
}

void ascan::associate_header() {
    for (auto &cfile : m_cfiles) {
        if (cfile.is_source()) {
            cfile.associate_header();
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
