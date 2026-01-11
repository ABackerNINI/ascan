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

    void build_options_section();
    void build_targets();
    void build_sources_section();
    void build_targets_section();

  protected:
    std::vector<cfile *> m_executable;

    bool m_c;
    bool m_cc;
    bool m_cpp;

    struct {
        std::string proj_name{};
        std::string src_dir{"src"};
        std::string bld_dir{"build"};
        std::string bin_dir{"bin"};
        MCompComponent options_section{"\n"};
        MCompComponent targets;
        MCompComponent sources_section{"\n"};
        MCompComponent targets_section{"\n"};
    } t;

    std::string m_result;
};

#endif // __AUTO_SCAN_MFILE_V4_H__
