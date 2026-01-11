#include "settings.h"

#include "ascan.h"
#include "debug_fmt.h"
#include "libs/CLI11.hpp"
#include "libs/rang.hpp"
#include "options.h"

class HelpFormatter : public CLI::Formatter {
  public:
    enum HELP_TYPE { HT_NONE, HT_ALL, HT_SPECIFIC };

    // std::string make_option_opts(const CLI::Option *option) const override { return gen_help(HT_ALL, nullptr); }

    std::string make_help(const CLI::App *app, std::string, CLI::AppFormatMode mode) const override {
        (void)app;
        (void)mode;
        return gen_help(HT_ALL, nullptr);
    }

    static std::string gen_help(enum HELP_TYPE help, const options::as_option *option);

  protected:
    template <typename T> static constexpr auto format(fmt::text_style ts, T &&s) {
        if (isatty(STDOUT_FILENO)) {
            return fmt::styled(s, ts);
        } else {
            return fmt::styled(s, fmt::text_style());
        }
    }

    template <typename T> static constexpr auto red_bold(T &&s) {
        return format(fmt::fg(fmt::terminal_color::red) | fmt::emphasis::bold, std::forward<T>(s));
    }

    static std::string gen_title(const std::string title) { return fmt::format("{}\n", red_bold(title)); }

    static std::string gen_option(char short_opt, const char *long_opt, const char *arg = nullptr) {
        std::string result = "";

        if (short_opt && long_opt) {
            result += fmt::format("\t{}, {}", red_bold(std::string("-") + short_opt),
                                  red_bold(std::string("--") + long_opt));
        } else if (short_opt) {
            result += fmt::format("\t{}", red_bold(std::string("-") + short_opt));
        } else {
            result += fmt::format("\t{}", red_bold(std::string("--") + long_opt));
        }
        if (arg) {
            result += fmt::format("={}", format(fg(fmt::terminal_color::green) | fmt::emphasis::bold, arg));
        }

        return result;
    }

    static std::string gen_desc(const std::string &desc) { return fmt::format("\t\t{}.\n\n", desc); }

    static std::string gen_msg(const std::string &msg) { return fmt::format("\t{}\n\n", msg); }
};

int Settings::parse_argv(int argc, char **argv) {
    CLI::App app{"ASCAN - Scan c/c++ project and create simple Makefile for it."};

    app.add_flag("-f", flag_force_, "Force overwrite");
    app.add_flag("-b,--build", flag_force_, "Put all binaries to 'build' subdirectory");
    app.add_flag("--recursive", flag_recursive_, "Recursively scan subdirectories");
    app.add_flag("-v,--version", flag_version_, "Print version information and exit");
    app.add_option("-o,--output", option_output_, "Output to the specified file rather than 'Makefile'");
    app.add_option("--debug", debug_level_, "Set debug level [0, 5]");
    app.add_option("source-dir,--src-dir", option_src_dir_, "Source directory");
    app.add_option("main-files", main_files_, "Specify source files containing main() function to compile");
    app.add_option("-I", include_dirs_, "Include directories");
    app.add_option("--use-v,--use-template,--template", temp_version_, "Use which template for Makefile generation")
        ->check(CLI::Range(1, 4));
    app.add_flag_callback(
        "-s, --simple", [&]() { temp_version_ = 1; }, "Generate really simple Makefile, same as --template=1");

    app.formatter(std::make_shared<HelpFormatter>());

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        if (app.count("-h") + app.count("--help") > 0) {
            flag_help_ = true;
        }
        std::cout << (e.get_exit_code() == 0 ? rang::fg::blue : rang::fg::red);
        int ret = app.exit(e);
        std::cout << rang::fg::reset;
        return ret;
    }

    if (flag_version_) {
        std::cout << ("ascan version: " ASCAN_VERSION "\n") << std::endl;
    }

    gstmt(debug_print());

    return 0;
}

void Settings::debug_print() const {
    gprint("Settings:\n");
    gprintc("flag_force_: {}\n", flag_force_);
    gprintc("flag_build_: {}\n", flag_build_);
    gprintc("flag_recursive_: {}\n", flag_recursive_);
    gprintc("flag_help_: {}\n", flag_help_);
    gprintc("flag_version_: {}\n", flag_version_);
    gprintc("option_output_: {}\n", option_output_);
    gprintc("option_src_dir_: {}\n", option_src_dir_);
    gprintc("main_files_: {}\n", main_files_);
    gprintc("include_dirs_: {}\n", include_dirs_);
    gprintc("debug_level_: {}\n", debug_level_);
}

std::string HelpFormatter::gen_help(enum HELP_TYPE help, const options::as_option *option) {
    static const char *desc = "Ascan will scan the c/c++ project and create simple makefile.\n\n"
                              "\tAscan is suitable for c/c++ projects that are:\n"
                              "\t\t1. Simple structured that all source codes are in one "
                              "directory.\n"
                              "\t\t2. Source codes are `.h` or `.c` or `.cpp` or `.cc`.\n"
                              "\t`cd` to the project directory and run `ascan`.";

    std::string s;

    if (help == HT_ALL) {
        s += gen_title("NAME");
        s += gen_msg("ascan - auto scan");

        s += gen_title("SYNOPSIS");
        s += gen_msg("ascan [OPTION]...");

        s += gen_title("DESCRIPTION");
        s += gen_msg(desc);
        s += gen_msg("Mandatory arguments to long options are mandatory for short "
                     "options too.");

        options m_options;

        size_t n;
        const options::as_option *options = m_options.get_as_opts(&n);
        for (unsigned int i = 0; i < n; ++i) {
            s += gen_option(options[i].short_opt, options[i].long_opt, options[i].arg);
            s += gen_desc(options[i].description);
        }

        s += gen_title("AUTHOR");
        s += gen_msg("Written by ABacker.");

        s += gen_title("REPORTING BUGS");
        s += gen_msg("<" ASCAN_URL ">");

        s += gen_title("COPYRIGHT");
        s += gen_msg("All rights reserved. "
                     "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.");

        // PRINT_TITLE("SEE ALSO");
    } else if (help == HT_SPECIFIC) {
        s += gen_title("OPTION");
        s += gen_msg("Mandatory arguments to long options are mandatory for short "
                     "options too.");
        s += gen_option(option->short_opt, option->long_opt, option->arg);
        s += gen_desc(option->description);
    }

    return s;
}
