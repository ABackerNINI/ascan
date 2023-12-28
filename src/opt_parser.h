/* This is a template not a library, so you have to modify it to meet your
 * requirements.
 *
 * This is designed for g++/clang++ on linux, and only for c++11 or later. */

#ifndef _OPTIONS_PARSER_H_
#define _OPTIONS_PARSER_H_

#include <string>
#include <vector>

#define ENABLE_MULTPLE_OPTION 1
#define ENABLE_HINT_MISMATCH_OPTION 1

enum ARG_TYPE { NO_ARGUMENT, REQUIRE_ARGUMENT, OPTIONAL_ARGUMENT };

// Since there is already a option struct defined in getopt.h
struct opt {
    // INPUT

    char short_opt;
    const char *long_opt;
    enum ARG_TYPE arg_type;

    const char *arg_name;
    const char *description;

    // OUTPUT

    int count;       // number of times presented
    int index;       // order in the arguments
    const char *arg; // argument

    opt();
};

class opt_parser {
  public:
    opt_parser();
    ~opt_parser();
    int parse(opt *opts, int n, int argc, char *const *argv);

  private:
    void make_short_opt(const opt *opts, int n);
    void make_long_opt(const opt *opts, int n);
    opt **make_ref(opt *opts, int n, int max_val) const;
    void handle_missing_arg_opt();
    void handle_unrecognized_opt();

  private:
    char *short_opts;
    struct option *long_opts;
};

#endif //_OPTIONS_PARSER_H_
