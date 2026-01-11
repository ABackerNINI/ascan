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
    std::string option_output_;
    std::string option_src_dir_{"."};
    std::string option_bin_dir_{"bin"};
    std::vector<std::string> main_files_;
    std::vector<std::string> include_dirs_;
    int temp_version_{4};
    int debug_level_{0}; // 0: ERROR, 1: WARNING, 2: INFO, 3: DEBUG, 4: MSGDUMP, 5: EXCESSIVE
};

#endif // __SETTINGS_H__
