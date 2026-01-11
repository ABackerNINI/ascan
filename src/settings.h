#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <string>

class Settings {
  public:
    // Parse the command line arguments and initialize the settings.
    int parse_argv(int argc, char **argv);

    void debug_print() const;

    bool flag_force_;
    bool flag_build_;
    std::string option_output_;
    int debug_level_; // 0: ERROR, 1: WARNING, 2: INFO, 3: DEBUG, 4: MSGDUMP, 5: EXCESSIVE
};

#endif // __SETTINGS_H__
