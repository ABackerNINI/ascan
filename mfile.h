#ifndef _AUTO_SCAN_MFILE_H_
#define _AUTO_SCAN_MFILE_H_

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "cfile.h"
#include "config.h"
#include "debug.h"

class mfile {
  public:
    mfile(std::vector<cfile> &cfiles, Config &cfg, uint32_t flags);
    int output();

  private:
    void prepare();
    void output_header_comments();
    void output_build_details();
    void output_compile_to_objects();
    void output_build_executable();
    void output_executables();
    void output_dependencies_helper(const std::vector<cfile> &files,
                                    cfile *file);
    void output_dependencies();
    void output_mm_dependencies();
    void output_clean_up();
    void output_phony();

    void output_part();

    void output_gitignore();

    // void output_build_path_if_option_b();
    void output_mk_build_if_option_b();

    void print_all_headers(const std::vector<cfile> &files, cfile *file);

  private:
    std::vector<cfile> &m_cfiles;
    Config &m_cfg;
    // bool m_flag_a;
    uint32_t m_flags;
    std::ofstream m_fout;
    std::vector<cfile *> m_executable;
    std::vector<std::string> m_binaries; // files to be added to gitignore

    bool m_c;
    bool m_cc;
    bool m_cpp;
    std::string m_build_path; // = "$(BUILD)/" iff OPTION_B is set
};

#endif //_AUTO_SCAN_MFILE_H_
