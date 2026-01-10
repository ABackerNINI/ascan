#ifndef __MFILE_V3_H__
#define __MFILE_V3_H__

#include "mfile.h"

class MFileV3 : public MFile {
  public:
    MFileV3(std::vector<cfile> &cfiles, Config &cfg, uint32_t flags);

    virtual ~MFileV3() {}

    virtual int output() override;

  protected:
    void prepare();

    void output_build_details();
    void output_targets();
    void output_compile_to_objects();
    void output_executable_details();
    void output_mode_control();
    void output_clean_up();
    void output_phony();

    // Output dependencies using gcc -MM.
    void output_mm_dependencies();

    void add_mkdir_build_cmd_if_option_b(MRule *rule);

    void output_part();

    void output_gitignore();

  protected:
    std::ofstream m_fout;
    std::vector<cfile *> m_executable;
    std::vector<std::string> m_binaries; // files to be added to gitignore

    bool m_c;
    bool m_cc;
    bool m_cpp;
    std::string m_build_path; // = "$(BUILD)/" iff OPTION_B is set
};

#endif // __MFILE_V3_H__
