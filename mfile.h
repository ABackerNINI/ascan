#ifndef _AUTO_SCAN_MFILE_H_
#define _AUTO_SCAN_MFILE_H_

#include <cstdio>
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
    void output_header_comments();
    void output_build_details_and_compile_to_objects();
    void output_build_executable();
    void output_dependencies_helper(FILE *m_fout, std::vector<cfile> &files,
                                    cfile *file);
    void output_dependencies();
    void output_clean_up();
    void output_phony();

    void output_part();

    void output_gitignore();

    void _output_build_path_if_option_b();
    void _output_mk_build_if_option_b();

  private:
    std::vector<cfile> &m_cfiles;
    Config &m_cfg;
    // bool m_flag_a;
    uint32_t m_flags;
    FILE *m_fout;
    std::vector<cfile *> m_executable;
    std::vector<std::string> m_binaries; // files to be added to gitignore
};

#endif //_AUTO_SCAN_MFILE_H_
