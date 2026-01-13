#include "settings.h"

#include "ascan.h"
#include "debug_fmt.h"
#include "libs/CLI11.hpp"
#include "libs/rang.hpp"
#include "utils.h"

class HelpFormatter : public CLI::Formatter {
  public:
    enum HELP_TYPE { HT_NONE, HT_ALL, HT_SPECIFIC };

    std::string make_help(const CLI::App *app, std::string, CLI::AppFormatMode mode) const override {
        (void)app;
        (void)mode;
        return gen_help(app, HT_ALL);
    }

  protected:
    template <typename T> static constexpr auto styled(fmt::text_style ts, T &&s) {
        if (isatty(STDOUT_FILENO)) {
            return fmt::styled(s, ts);
        } else {
            return fmt::styled(s, fmt::text_style());
        }
    }

    template <typename T> static constexpr auto red_bold(T &&s) {
        return styled(fmt::fg(fmt::terminal_color::red) | fmt::emphasis::bold, std::forward<T>(s));
    }

    template <typename T> static constexpr auto green_bold(T &&s) {
        return styled(fmt::fg(fmt::terminal_color::green) | fmt::emphasis::bold, std::forward<T>(s));
    }

    static std::string gen_title(const std::string title) { return fmt::format("{}\n", red_bold(title)); }

    static std::string gen_option(const CLI::Option *option) {
        const std::vector<std::string> &snames = option->get_snames();
        const std::vector<std::string> &lnames = option->get_lnames();
        const std::string pname = option->get_name(true);
        const std::string &option_text = option->get_option_text();
        const std::string &desc = option->get_description();

        std::string result = "";

        result += "\t";

        if (!pname.empty()) {
            result += fmt::format("{}", red_bold(pname));
        }
        if (!snames.empty()) {
            if (!pname.empty()) {
                result += ", ";
            }
            for (auto &name : snames) {
                result += fmt::format("{}, ", red_bold(std::string("-") + name));
            }
            // remove last ", "
            result.pop_back();
            result.pop_back();
        }
        if (!lnames.empty()) {
            if (!pname.empty() || !snames.empty()) {
                result += ", ";
            }
            for (auto &name : lnames) {
                result += fmt::format("{}, ", red_bold(std::string("--") + name));
            }
            // remove last ", "
            result.pop_back();
            result.pop_back();
        }

        std::string arg = !option_text.empty() ? option_text : option->get_type_name();

        if (arg != "") {
            bool is_arg_optional = !option->get_default_str().empty();
            if (!is_arg_optional) {
                result += fmt::format("={}", green_bold(arg));
            } else {
                result += fmt::format("[={}]", green_bold(arg));
            }
        }

        result += fmt::format("\n\t\t{}.\n\n", desc);

        return result;
    }

    static std::string gen_msg(const std::string &msg) { return fmt::format("\t{}\n\n", msg); }

    std::string gen_help(const CLI::App *app, enum HELP_TYPE help) const {
        static const char *desc =
            "Scan c/c++ source files in SOURCE_DIR directory and create a simple makefile for MAIN_FILEs.\n"
            "\tWhen MAIN_FILE is not specified, it will detect automatically.\n\n"
            "\tAscan is suitable for c/c++ projects that are:\n"
            "\t\t1. Simple structured that all source codes are in one directory.\n"
            "\t\t2. Source codes are `.h` or `.c` or `.cpp` or `.cc`.\n"
            "\t`cd` to the project directory and run `ascan`.";

        std::string s;

        if (help == HT_ALL) {
            s += gen_title("NAME");
            s += gen_msg("ascan - auto scan");

            s += gen_title("SYNOPSIS");
            s += gen_msg(fmt::format("ascan [{}]... [{}] [{}]...", green_bold("OPTION"), green_bold("SRC_DIR"),
                                     green_bold("MAIN_FILE")));

            s += gen_title("DESCRIPTION");
            s += gen_msg(desc);
            s += gen_msg("Mandatory arguments to long options are mandatory for short options too.");

            std::vector<const CLI::Option *> options = app->get_options();
            for (auto opt : options) {
                s += gen_option(opt);
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
            s += gen_msg("Mandatory arguments to long options are mandatory for short options too.");
            // s += gen_option(option->short_opt, option->long_opt, option->arg);
            // s += gen_desc(option->description);
        }

        return s;
    }
};

std::string suggest_similar_option(std::string input, const CLI::App &app) {
    // Remove leading dashes
    size_t leading_dashes = 0;
    while (input[leading_dashes] == '-' && leading_dashes < input.length()) {
        leading_dashes++;
    }
    input = input.substr(leading_dashes);

    // No suggestion for very short inputs
    if (input.size() <= 1) {
        return "";
    }

    // Find the closest match
    std::vector<const CLI::Option *> options = app.get_options();
    std::string suggestion;
    size_t min_distance = INT32_MAX;
    for (const CLI::Option *opt : options) {
        for (auto &opt_name : opt->get_lnames()) { // Consider only long names for now
            if (opt_name.empty()) {
                continue;
            }
            size_t distance = edit_distance(input, opt_name);
            if ((distance <= input.size() / 2 && distance <= opt_name.size() / 2) && distance < min_distance) {
                min_distance = distance;
                suggestion = "--" + opt_name;
            }
        }
    }

    return suggestion;
}

bool suggest_similar_option_based_on_CLI11_error_string(const std::string &error_string, const CLI::App &app) {
    // The CLI11 error string looks like:
    //   The following argument was not expected: -x
    //   The following arguments were not expected: -x -y
    // We just suggest the first unexpected option.
    if (starts_with(error_string, "The following argument was not expected: ") ||
        starts_with(error_string, "The following arguments were not expected: ")) {
        std::string::size_type option_pos = error_string.find(": ");
        if (option_pos == std::string::npos) {
            return false;
        }
        std::string::size_type option_end_pos = error_string.find(" ", option_pos + 2);
        if (option_end_pos == std::string::npos)
            option_end_pos = error_string.length();
        std::string option = error_string.substr(option_pos + 2, option_end_pos - option_pos - 2);
        std::string suggestion = suggest_similar_option(option, app);
        if (!suggestion.empty()) {
            print("Unrecognized option: '{}', did you mean: '{}'?\n",
                  fmt::styled(option, fg(fmt::terminal_color::red) | fmt::emphasis::bold),
                  fmt::styled(suggestion, fg(fmt::terminal_color::green) | fmt::emphasis::bold));
            return true;
        }
    }
    return false;
}

// TODO: add option --static-lib/--shared-lib, to create static or shared library.

int Settings::parse_argv(int argc, char **argv) {
    CLI::App app{"ASCAN - Scan c/c++ project and create simple Makefile for it."};

    app.allow_non_standard_option_names();

    app.add_flag("-f", flag_force_, "Force overwrite");
    app.add_flag("--no-build", flag_no_build_, "Do NOT put all binaries to 'build' subdirectory");
    app.add_flag("-r,--recursive", flag_recursive_, "Recursively scan subdirectories");
    app.add_flag("-v,--version", flag_version_, "Print version information and exit");
    app.add_option("-o,--output", option_output_, "Output to the specified file rather than 'Makefile'")
        ->option_text("FILE");
    app.add_option("--debug", debug_level_, "Set debug level [0, 5]")->check(CLI::Range(0, 5))->option_text("LEVEL");
    app.add_option("-t,--template", temp_version_, "Use which template for Makefile generation")
        ->check(CLI::Range(1, 4))
        ->option_text("VERSION");
    app.add_flag_callback(
        "--simple", [&]() { temp_version_ = 1; }, "Generate really simple Makefile, same as --template=1");
    app.add_option("--build-dir", option_build_dir_, "Build directory, objects will be put here, default is 'build'")
        ->option_text("DIR");
    app.add_option("--bin-dir", option_bin_dir_,
                   "Binary output directory, the final binary will be put here, default is 'bin'")
        ->option_text("DIR");
    app.add_option("--project-name", option_proj_name_, "Project name, default is the name of the current directory")
        ->option_text("NAME");
    app.add_option("--default-config", option_default_config_,
                   "Default build configuration {debug, release}, default is 'debug'")
        ->check(CLI::IsMember({"debug", "release"}))
        ->option_text("CONFIG");
    app.add_option("--cc", option_cc_, "C compiler to use, default is 'gcc'")->option_text("COMPILER");
    app.add_option("--cxx", option_cxx_, "C++ compiler to use, default is 'g++'")->option_text("COMPILER");
    app.add_option("--stdc", option_std_c_, "C standard to use, default is 'c11'")->option_text("STD");
    app.add_option("--stdcxx", option_std_cxx_, "C++ standard to use, default is 'c++17'")->option_text("STD");
    app.add_option("SOURCE_DIR,--src-dir", option_src_dir_, "Source directory, default is the current directory")
        ->check(CLI::ExistingDirectory)
        ->option_text("DIR");
    app.add_option("MAIN_FILE", main_files_, "Specify source files containing main() function to compile")
        ->check(CLI::ExistingFile)
        ->option_text("FILE");

    app.add_option("-I", include_dirs_, "Specify include directory")->check(CLI::ExistingDirectory)->option_text("DIR");
    app.add_option_function<std::string>(
           "-L", [&](const std::string &lib_dir) { option_ldflags_.push_back("-L" + lib_dir); },
           "Specify library directory")
        ->multi_option_policy(CLI::MultiOptionPolicy::TakeAll)
        ->trigger_on_parse()
        ->check(CLI::ExistingDirectory)
        ->option_text("DIR");
    app.add_option_function<std::string>(
           "-l", [&](const std::string &lib) { option_ldflags_.push_back("-l" + lib); }, "Specify library to link with")
        ->multi_option_policy(CLI::MultiOptionPolicy::TakeAll)
        ->trigger_on_parse()
        ->option_text("LIB");
    app.add_option("-std", option_std_, "C/C++ standard to use, default is 'c11'/'c++17' respectively")
        ->option_text("STD");

    // DEBUG FLAGS & OPTIONS

    app.add_option_function<std::string>(
           "-x",
           [&](const std::string &lang) {
               if (lang == "c") {
                   debug_flag_xc_ = true;
               } else if (lang == "cc") {
                   debug_flag_xcc_ = true;
               } else if (lang == "c++") {
                   debug_flag_xcpp_ = true;
               } else {
                   throw CLI::ValidationError("Invalid language: " + lang);
               }
           },
           "[DEBUG ONLY] Specify language included")
        ->check(CLI::IsMember({"c", "cc", "c++"}))
        ->multi_option_policy(CLI::MultiOptionPolicy::TakeAll)
        ->trigger_on_parse()
        ->option_text("LANG");

    app.formatter(std::make_shared<HelpFormatter>());

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        if (app.count("-h") + app.count("--help") > 0) {
            flag_help_ = true;
        }

        // Handle OptionNotFound error by tring to suggest similar option
        // OptionNotFound error was converted to ExtrasError by CLI11
        if (e.get_exit_code() == static_cast<int>(CLI::ExitCodes::ExtrasError)) {
            if (suggest_similar_option_based_on_CLI11_error_string(e.what(), app)) {
                print("Run with --help for more information.\n");
                return 1;
            }
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

#define print_setting3(name1, name2, name3, width)                                                                     \
    gprintc("{:<{}}", fmt::format(#name1 ": {}", name1), width);                                                       \
    gprintc("{:<{}}", fmt::format(#name2 ": {}", name2), width);                                                       \
    gprintc("{:<{}}\n", fmt::format(#name3 ": {}", name3), width);

void Settings::debug_print() const {
    gprint("Settings:\n");
    const int width = 35;
    print_setting3(flag_force_, flag_no_build_, flag_recursive_, width);
    print_setting3(flag_help_, flag_version_, temp_version_, width);
    print_setting3(option_src_dir_, option_output_, option_proj_name_, width);
    print_setting3(option_default_config_, option_cc_, option_cxx_, width);
    print_setting3(option_std_, option_std_c_, option_std_cxx_, width);
    print_setting3(option_build_dir_, option_bin_dir_, option_ldflags_, width);
    print_setting3(main_files_, include_dirs_, debug_level_, width);
}
