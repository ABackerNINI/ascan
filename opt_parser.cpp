#include <cstring>
#include <cassert>

#include "opt_parser.h"

#include "debug.h"

#define TEST 1
#define VAL_SHIFT 150

opt::opt()
    : short_opt('\0'),
      long_opt(NULL),
      type(NO_ARGUMENT),
      count(0),
      arg(NULL),
      arg_name(NULL),
      description(NULL) {}

static int make_options(struct opt *&options) {
    const int n = 12;

    options = new opt[n];

    options[0].short_opt = 'a';
    options[0].long_opt = "all";

    options[1].short_opt = 'b';
    options[1].long_opt = "build";

    options[2].short_opt = 'f';
    options[2].long_opt = "force";

    options[3].short_opt = 'g';

    options[4].short_opt = 'h';
    options[4].long_opt = "help";

    options[5].short_opt = 'o';
    options[5].long_opt = "output";
    options[5].type = REQUIRE_ARGUMENT;

    options[6].short_opt = 'v';
    options[6].long_opt = "ver";

    options[7].long_opt = "debug";
    options[7].type = REQUIRE_ARGUMENT;

    options[8].long_opt = "cc";
    options[8].type = REQUIRE_ARGUMENT;

    options[9].long_opt = "cxx";
    options[9].type = REQUIRE_ARGUMENT;

    options[10].long_opt = "cflags";
    options[10].type = REQUIRE_ARGUMENT;

    options[11].long_opt = "cxxflags";
    options[11].type = REQUIRE_ARGUMENT;

    return n;
}

// const options::as_option options::s_as_opts[] = {
//     {OT_ALL_SECS, 'a', "all", NULL, true,
//      "Overwrite all sections, ascan will only overwrite the "
//      "'Dependencies' section on default"},
//     {OT_BUILD, 'b', "build", NULL, true,
//      "Put binaries to 'build' subdirectory, Option '-b' is not implemented "
//      "yet!"},
//     {OT_FORCE, 'f', "force", NULL, true, "Force overwrite"},
//     {OT_G, 'g', NULL, NULL, true, "add '-g' flag to cflags or cxxflags"},
//     {OT_HELP, 'h', "help", NULL, true, "Print help information"},
//     {OT_OUTPUT, 'o', "output", "OUTPUT_FILE", false,
//      "Output to the specified file rather than 'Makefile'"},
//     {OT_VER, 'v', "ver", NULL, true, "Print ascan version"},
//     {OT_DEBUG, '\0', "debug", "DEBUG_LEVEL", false,
//      "Set debug level: \n\t\t\t- 0: ERROR\n\t\t\t- "
//      "1: WARNING\n\t\t\t- 2: INFO\n\t\t\t- 3: DEBUG\n\t\t\t- 4: "
//      "MSGDUMP\n\t\t\t- 5: EXCESSIVE"},
//     {OT_CC, '\0', "cc", "CC", false,
//      "Set c compiler, default: '" CONFIG_DEFAULT_V_CC "'"},
//     {OT_CXX, '\0', "cxx", "CXX", false,
//      "Set c++ compiler, default: '" CONFIG_DEFAULT_V_CXX "'"},
//     {OT_CFLAGS, '\0', "cflags", "CFLAGS", false,
//      "Set c compile flags, default: '" CONFIG_DEFAULT_V_CFLAG "'"},
//     {OT_CXXFLAGS, '\0', "cxxflags", "CXXFLAGS", false,
//      "Set c++ compile flags, default: '" CONFIG_DEFAULT_V_CXXFLAG "'"},
// };

opt_parser::opt_parser() : short_opts(NULL), long_opts(NULL) {}

opt_parser::~opt_parser() {
    delete[] short_opts;
    delete[] long_opts;
}

int opt_parser::parse(opt *opts, int n, int argc, char *const *argv) {
    make_short_opt(opts, n);
    make_long_opt(opts, n);

    opt **opts_ref = make_ref(opts, n, n + VAL_SHIFT);

    opterr = 0;  // do NOT print error message

    int opt, long_ind;

    while ((opt = getopt_long(argc, argv, short_opts, long_opts, &long_ind)) !=
           -1) {
        if (opt == '?') {
            // '?' is returned if there are unrecognized/missing-arguments
            // options. Unrecognized options is set to optopt.
            // Note: don't need to concern about no-argument options having
            // arguments.
                
        } else {
            assert(opt <= n + VAL_SHIFT && opts_ref[opt]);

            if (opts_ref[opt]->type != NO_ARGUMENT) {
                opts_ref[opt]->arg = optarg;
            }
            ++opts_ref[opt]->count;
        }
    }

    delete[] opts_ref;

    return optind;
}

void opt_parser::make_short_opt(const opt *opts, int n) {
    short_opts = new char[n * 3 + 1];
    int los = 0;
    for (int i = 0; i < n; ++i) {
        if (opts[i].short_opt != '\0') {
            short_opts[los++] = opts[i].short_opt;
            if (opts[i].type == REQUIRE_ARGUMENT) {
                short_opts[los++] = ':';
            } else if (opts[i].type == OPTIONAL_ARGUMENT) {
                short_opts[los++] = ':';
                short_opts[los++] = ':';
            }
        }
    }
    short_opts[los] = '\0';
    dbg_assert(los < n * 3 + 1);

    print_debug("short options: |%s|\n", short_opts);
}

void opt_parser::make_long_opt(const opt *opts, int n) {
    long_opts = new struct option[n + 1];

    int los = 0;
    for (int i = 0; i < n; ++i) {
        if (opts[i].long_opt) {
            long_opts[los].name = opts[i].long_opt;
            long_opts[los].has_arg =
                opts[i].type == REQUIRE_ARGUMENT
                    ? required_argument
                    : (opts[i].type == OPTIONAL_ARGUMENT ? optional_argument
                                                         : no_argument);
            long_opts[los].flag = NULL;
            long_opts[los].val =
                opts[i].short_opt ? opts[i].short_opt : VAL_SHIFT + i;
            ++los;
        }
    }
    long_opts[los] = {NULL, 0, NULL, 0};
    print_msgdump("long options size: %d\n", los);
}

opt **opt_parser::make_ref(opt *opts, int n, int max_val) const {
    opt **opts_ref = new opt *[max_val + 1];

    memset(opts_ref, 0, sizeof(opt *) * (max_val + 1));

    for (int i = 0; i < n; ++i) {
        if (opts[i].short_opt) {
            opts_ref[(int)opts[i].short_opt] = &opts[i];
        } else if (opts[i].long_opt) {
            opts_ref[i + VAL_SHIFT] = &opts[i];
        }
    }

    return opts_ref;
}

/*==========================================================================*/

#if (TEST)

int debug_level = 5;

int main(int argc, char **argv) {
    struct opt *options;

    int n = make_options(options);

    opt_parser opt;
    opt.parse(options, n, argc, argv);

    delete[] options;

    return 0;
}

#endif
