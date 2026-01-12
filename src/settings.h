#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <string>
#include <vector>

class Settings {
  public:
    // Parse the command line arguments and initialize the settings.
    int parse_argv(int argc, char **argv);

    void debug_print() const;

    bool flag_force_{false};
    bool flag_no_build_{true};
    bool flag_recursive_{false};
    bool flag_help_{false};
    bool flag_version_{false};
    int temp_version_{4};
    std::string option_src_dir_;
    std::string option_output_;
    std::string option_proj_name_;
    std::string option_default_config_;
    std::string option_cc_;
    std::string option_cxx_;
    std::string option_std_;
    std::string option_std_c_;
    std::string option_std_cxx_;
    std::string option_build_dir_;
    std::string option_bin_dir_;
    std::vector<std::string> option_ldflags_;
    std::vector<std::string> main_files_;
    std::vector<std::string> include_dirs_;
    int debug_level_{0}; // 0: ERROR, 1: WARNING, 2: INFO, 3: DEBUG, 4: MSGDUMP, 5: EXCESSIVE
};

#endif // __SETTINGS_H__
