#pragma once

#ifndef _AUTO_SCAN_ASCAN_H_
#define _AUTO_SCAN_ASCAN_H_

#include <vector>
#include <string>
#include <iostream>
#include "cfile.h"
#include "config.h"

using std::string;
using std::vector;

class ascan {
   public:
    ascan(int argc, char **argv);
    int start();

   private:
    void parse_cmd_args(int argc, char **argv);
    void print_help();

    bool test_makefile();
    void match_c_cpp_includes();
    void associate_header();

   private:
    string m_cwd;            // current working dir
    bool m_flag_f;           // -f: force
    Config m_cfg;            // config
    string m_makefile;       // filename of makefile
    vector<cfile> m_cfiles;  // cfiles
};

int main(int argc, char **argv);

#endif  //_AUTO_SCAN_ASCAN_H_
