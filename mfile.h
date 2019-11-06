#pragma once

#ifndef _AUTO_SCAN_MFILE_H_
#define _AUTO_SCAN_MFILE_H_

#include <string>
#include <vector>
#include <cstdio>

#include "cfile.h"
#include "config.h"

using std::string;
using std::vector;

class mfile {
   public:
    mfile(const string &makefile, vector<cfile> &cfiles, Config &cfg,
          bool flag_all);
    int output();

   private:
    void output_header_comments();
    void output_build_details_and_compile_to_objects();
    void output_build_executable();
    void output_dependencies();
    void output_clean_up();
    void output_part();

   private:
    string m_makefile;
    vector<cfile> &m_cfiles;
    Config &m_cfg;
    bool m_flag_a;
    FILE *m_fout;
    vector<cfile *> m_executable;
};

#endif  //_AUTO_SCAN_MFILE_H_