#include "settings.h"
#include "debug_fmt.h"
#include "libs/CLI11.hpp"
#include "libs/rang.hpp"

int Settings::parse_argv(int argc, char **argv) {
    CLI::App app{"ASCAN - Scan c/c++ project and create simple Makefile for it."};

    app.add_flag("-f", flag_force_, "Force overwrite");
    app.add_flag("-b,--build", flag_force_, "Put all binaries to 'build' subdirectory");
    app.add_flag("--recursive", flag_recursive_, "Recursively scan subdirectories");
    app.add_option("-o,--output", option_output_, "Output to the specified file rather than 'Makefile'");
    app.add_option("--debug", debug_level_, "Set debug level [0, 5]");
    app.add_option("source-dir,--src-dir", option_src_dir_, "Source directory");
    app.add_option("main-files", main_files_, "Specify source files containing main() function to compile");
    app.add_option("-I", include_dirs_, "Include directories");

    // app.add_flag_callback("--include-all", [&]() {}, "");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        std::cout << (e.get_exit_code() == 0 ? rang::fg::blue : rang::fg::red);
        int ret = app.exit(e);
        std::cout << rang::fg::reset;
        return ret;
    }

    gstmt(debug_print());

    return 0;
}

void Settings::debug_print() const {
    gprint("Settings:\n");
    gprintc("flag_force_: {}\n", flag_force_);
    gprintc("flag_build_: {}\n", flag_build_);
    gprintc("flag_recursive_: {}\n", flag_recursive_);
    gprintc("option_output_: {}\n", option_output_);
    gprintc("option_src_dir_: {}\n", option_src_dir_);
    gprintc("main_files_: {}\n", main_files_);
    gprintc("include_dirs_: {}\n", include_dirs_);
    gprintc("debug_level_: {}\n", debug_level_);
}
