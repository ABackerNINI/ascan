#ifndef _AUTO_SCAN_CONFIG_H_
#define _AUTO_SCAN_CONFIG_H_

#include <map>
#include <string>
#include <vector>

//! Some configurations can NOT contain spaces including " \r\n\t\v\f"
//! (isspace()) and can NOT contain '/'.
//! Serious consequences may occur if the configuration contains spaces.
//! Because they may show in the clean section of the Makefile.
//! For example, if CONFIG_DEPENDENCIES_FILENAME is set to "depend.mk /",
//! so the clean section of the generated Makefile will be:
//!     clean:
//!         rm -rf depend.mk /
//! This could be a disaster.
//! Similarly, if CONFIG_DEPENDENCIES_FILENAME is set to "/home",
//! The clean section of the generated Makefile will be:
//!     clean:
//!         rm -rf /home
//! Another disaster.

// Can NOT contain spaces.
// Can NOT contain '/'.
#define CONFIG_FILENAME "ascan.conf"

#define CONFIG_DEFAULT_OUTPUT "Makefile"
// Can NOT contain spaces.
// Can NOT contain '/'.
#define CONFIG_DEPENDENCIES_FILENAME "depend.mk"

// Can NOT contain spaces.
#define CONFIG_CC "CC"
// Can NOT contain spaces.
#define CONFIG_CXX "CXX"
// Can NOT contain spaces.
#define CONFIG_CFLAGS "CFLAGS"
// Can NOT contain spaces.
#define CONFIG_CXXFLAGS "CXXFLAGS"
// Can NOT contain spaces.
#define CONFIG_LFLAGS "LFLAGS"
// Can NOT contain spaces.
// Can NOT contain '/'.
#define CONFIG_BD "BD"

#define CONFIG_DEFAULT_V_CC "gcc"
#define CONFIG_DEFAULT_V_CXX "g++"
#define CONFIG_DEFAULT_V_CFLAGS "-W -Wall"
#define CONFIG_DEFAULT_V_CXXFLAGS "-W -Wall"
#define CONFIG_DEFAULT_V_LFLAGS "-lm"
// Can NOT contain spaces.
// Can NOT contain '/'.
#define CONFIG_DEFAULT_V_BD "build"

// Can NOT contain spaces.
// Can NOT contain '/'.
#define CONFIG_BIN "TARGET%d"
// Can NOT contain spaces.
// Can NOT contain '/'.
#define CONFIG_OBJ "OBJ%d"
// Can NOT contain spaces.
// Can NOT contain '/'.
#define CONFIG_OBJ_BD "OBJ%dBD"

class Config {
  public:
    // User specified output file name.
    std::string output;
    // User specified start files. Dependencies will be generated start from
    // them. If not specified, ascan will scan c/c++ source files in the current
    // directory that contains main functions and start from them.
    std::vector<std::string> start_files;

    std::string k_bin;
    std::string k_obj;
    std::string k_obj_bd;

    // std::map<std::string, std::string> executable;
    // std::vector<std::string> ignores; // TODO add ignores?

  public:
    Config();

    const std::string &get_config_value(const std::string &config_name) const;
    void set_config(const std::string &config_name,
                    const std::string &config_value);

    std::string make_bin(int i) const;
    std::string make_obj(int i) const;
    std::string make_obj_bd(int i) const;

  private:
    std::string make(const std::string &s, int i) const;

    // Check if the given config string contains spaces (isspace()).
    // If it does, print error message and exit the program.
    void check_space(const std::string &config) const;

    // Check if the given config string contains '/'.
    // If it does, print error message and exit the program.
    void check_slash(const std::string &config) const;

  private:
    std::map<std::string, std::string> configs;
};

// int read_config(const char *filename, Config *cfg);

#endif //_AUTO_SCAN_CONFIG_H_
