#include "settings.h"
#include "debug_fmt.h"
#include "fs.h"
#include "libs/CLI11.hpp"
#include "libs/rang.hpp"

int Settings::parse_argv(int argc, char **argv) {
    CLI::App app{"RCC - Run C/C++ codes in terminal"};
    app.allow_extras();
    app.add_flag("-f", flag_force_, "Force overwrite");
    app.add_flag("-b,--build", flag_force_, "Put all binaries to 'build' subdirectory");
    app.add_option("-o,--output", option_output_, "Output to the specified file rather than 'Makefile'");
    app.add_option("--debug", debug_level_, "Set debug level [0, 5]");
    // app.add_flag_callback("--include-all", [&]() {}, "");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        std::cout << (e.get_exit_code() == 0 ? rang::fg::blue : rang::fg::red);
        int ret = app.exit(e);
        std::cout << rang::fg::reset;
        return ret;
    }

    std::vector<std::string> remaining = app.remaining(true);

    for (auto &s : remaining) {}

    return 0;
}

void Settings::debug_print() const {}
