#pragma once

#ifndef _AUTO_SCAN_OPTIONS_H_
#define _AUTO_SCAN_OPTIONS_H_

#include <iostream>
#include <getopt.h>

#include "config.h"

#define OPTION_A 0x1   // option -a: overwrite all sections
#define OPTION_B 0x2   // option -b: put binaries to 'build' subdirectory
#define OPTION_F 0x4   // option -f: force overwrite
#define OPTION_G 0x8   // option -g: add '-g' flag to cflags or cxxflags
#define OPTION_C 0x10  // option -c: add header comments

class options {
   public:
    enum OPT_TYPE {
        OT_ALL_SECS = 'a',  // -a
        OT_BUILD = 'b',     // -b
        OT_FORCE = 'f',     // -f
        OT_G = 'g',         // -g
        OT_HELP = 'h',      // -h,--help
        OT_VER = 'v',       // -v,--ver
        OT_DEBUG = 200,     // --debug
        OT_CC,              // --cc
        OT_CXX,             // --cxx
        OT_CFLAGS,          // --cflags
        OT_CXXFLAGS,        // --cxxflags
    };

    struct as_option {
        enum OPT_TYPE type;
        const char *short_opt;
        const char *long_opt;
        const char *arg;
        const char *description;
    };

   public:
    options();
    const char *get_short_opts() const;
    const struct option *get_long_opts() const;
    const as_option *get_as_opts(size_t *size) const;
    const as_option *find_opt(enum OPT_TYPE type) const;
    const as_option *find_similar_opt(const char *opt) const;

    void print_opt_type(enum OPT_TYPE type);

   private:
    struct option *make_long_opts() const;

   private:
    struct option *long_opts;

   private:
    static const char s_short_opts[];
    static const as_option s_as_opts[];
    static const size_t s_as_opt_size;
};

#endif  //_AUTO_SCAN_OPTIONS_H_
