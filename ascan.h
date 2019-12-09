#pragma once

#ifndef _AUTO_SCAN_ASCAN_H_
#define _AUTO_SCAN_ASCAN_H_

#include <vector>
#include <string>
#include <iostream>
#include "cfile.h"
#include "config.h"
#include "flags.h"

using std::string;
using std::vector;

class ascan {
   public:
    ascan(int argc, char **argv);
    // Start to proceed auto-scan.
    //
    // Return:
    // --- EXIT_SUCCESS: if no error occurred.
    // --- EXIT_FAILURE: otherwise.
    int start();

   private:
    // enum HELP_CMD_TYPE {
    //     HCT_ALL,
    //     HCT_ALL_SECS,  // -a
    //     HCT_BUILD,     // -b
    //     HCT_FORCE,     // -f
    //     HCT_DEBUG,     // --d
    //     HCT_CC,        // --cc
    //     HCT_CXX,       // --cxx
    //     HCT_CFLAGS,    // --cflags
    //     HCT_CXXFLAGS   // --cxxflags
    // };

    // Parse the command arguments.
    //
    // Return:
    // --- true: if one of the argument '--help', '-v', '--ver' is presented or
    // an error occurred.
    // --- false: otherwise.
    bool parse_cmd_args(int argc, char **argv);
    void print_help(int ind) const;

    // Check if there is one makefile, prompt overwrite when -f is not set.
    //
    // Return:
    // --- true: if -f is set or user entered 'yes' to overwrite or no makefile
    // found.
    // --- false: otherwise.
    bool test_makefile();
    void match_c_cpp_includes();
    void associate_header();

   private:
    bool m_proceed;          // whether need to proceed makefile
    // bool m_flag_a;           // flag -a: overwrite all sections
    // bool m_flag_b;           // flag -b: put binaries to 'build' subdirectory
    // bool m_flag_f;           // flag -f: force overwrite
    // bool m_flag_g;           // flag -g: add '-g' flag to cflags or cxxflags
    uint32_t m_flags;       // contains all flags
    Config m_cfg;            // config
    string m_cwd;            // current working dir
    string m_makefile;       // filename of makefile
    vector<cfile> m_cfiles;  // cfiles
};

int main(int argc, char **argv);

#endif  //_AUTO_SCAN_ASCAN_H_
