#ifndef __AUTO_SCAN_MFILE_V4_H__
#define __AUTO_SCAN_MFILE_V4_H__

#include "mfile.h"
#include "traits.h"
#include <string>
#include <type_traits>
#include <vector>

class MFileV4 : public MFile {
  public:
    MFileV4(const Settings &settings, std::vector<cfile> &cfiles) : MFile(settings, cfiles) {}

    virtual ~MFileV4() {}

    virtual int build() override;

    virtual std::string to_string() const override { return m_result; }

  protected:
    template <typename T> void import_setting(T &feild, const T &from_setting) {
        if constexpr (std::is_same_v<std::string, T> || (is_container_v<T> && has_empty_v<T>)) {
            if (!from_setting.empty()) {
                feild = from_setting;
            }
        } else if constexpr (std::is_arithmetic_v<T> || std::is_same_v<bool, T>) {
            feild = from_setting;
        } else {
            static_assert(std::is_same_v<T, void>, "Not supported type");
        }
    }

    void import_settings();
    void prepare();

    MSimpleVariableDef *add_svardef(
        MCompComponent &mcc,
        const std::string &varname,
        const std::string &varval,
        VariableAssignmentType assignment_type = VariableAssignmentType::RECURSIVELY_EXPANDED);
    MSimpleVariable *add_svar(MCompComponent &mcc, const std::string &varname);

    void build_options_section();
    void build_obj_dir_config_file_cc_cxx_std();
    void build_c_cxx_flags();
    void build_targets();
    void build_c_cxx_flags_amend();
    void build_sources_section();
    void build_targets_section();

  protected:
    std::vector<cfile *> m_executable;

    bool m_c{false};
    bool m_cc{false};
    bool m_cpp{false};

    const std::vector<std::string> default_flags = {"-Wall", "-Wextra"};

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
        MCompComponent c_cxx_flags{"\n"};
        MCompComponent targets{"\n"};
        MCompComponent c_cxx_flags_debug_amend{"\n"};
        MCompComponent c_cxx_flags_release_amend{"\n"};
        MCompComponent sources_section{"\n"};
        MCompComponent targets_section{"\n"};
    } t;

    std::string m_result;
};

#endif // __AUTO_SCAN_MFILE_V4_H__
