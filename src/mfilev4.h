#ifndef __AUTO_SCAN_MFILE_V4_H__
#define __AUTO_SCAN_MFILE_V4_H__

#include "mfile.h"

class MFileV4 : public MFile {
  public:
    MFileV4(const Settings &settings, std::vector<cfile> &cfiles) : MFile(settings, cfiles) {}

    virtual ~MFileV4() {}

    virtual int build() override;

    virtual std::string to_string() const override { return m_result; }

  protected:
    void prepare();
    void import_setting(std::string &feild, const std::string &from_setting);
    void import_settings();

    void add_svardef(MCompComponent &mcc, const std::string &varname, const std::string &varval);
    void add_svar(MCompComponent &mcc, const std::string &varname);

    void build_options_section();
    void build_obj_dir_config_file_cc_cxx_std();
    void build_targets();
    void build_sources_section();
    void build_targets_section();

  protected:
    std::vector<cfile *> m_executable;

    bool m_c;
    bool m_cc;
    bool m_cpp;

    struct {
        std::string proj_name;
        std::string config{"debug"};
        std::string cc{"gcc"};
        std::string cxx{"g++"};
        std::string stdc{"c17"};
        std::string stdcxx{"c++17"};
        std::string src_dir{"src"};
        std::string bld_dir{"build"};
        std::string bin_dir{"bin"};
        std::string ldflags;
        MCompComponent options_section{"\n"};
        MCompComponent obj_dir_config_file_cc_cxx_std{""};
        MCompComponent targets;
        MCompComponent sources_section{"\n"};
        MCompComponent targets_section{"\n"};
    } t;

    std::string m_result;
};

#endif // __AUTO_SCAN_MFILE_V4_H__
