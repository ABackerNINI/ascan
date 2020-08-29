#pragma once

/* This is a template not a library, so you have to modify it to meet your
 * requirements.
 *
 * This is designed for g++/clang++ on linux, and only for c++11 or later. */

#ifndef _OPTIONS_PARSER_H_
#define _OPTIONS_PARSER_H_

#include <getopt.h>

enum ARG_TYPE { NO_ARGUMENT, REQUIRE_ARGUMENT, OPTIONAL_ARGUMENT };

struct opt {
    char short_opt;
    const char *long_opt;
    enum ARG_TYPE type;

    int show;
    const char *arg;

    // TODO: add more
    const char *arg_name;
    const char *description;

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
    opt**make_ref(opt *opts, int n, int max_val) const;

   private:
    char *short_opts;
    struct option *long_opts;
};

#endif  //_OPTIONS_PARSER_H_
