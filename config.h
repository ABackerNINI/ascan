#ifndef _AUTO_SCAN_CONFIG_H_
#define _AUTO_SCAN_CONFIG_H_

#include <map>
#include <string>
#include <vector>

#define CONFIG_FILENAME "ascan.conf"

#define CONFIG_DEFAULT_OUTPUT "Makefile"

#define CONFIG_CC "CC"
#define CONFIG_CXX "CXX"
#define CONFIG_CFLAGS "CFLAGS"
#define CONFIG_CXXFLAGS "CXXFLAGS"
#define CONFIG_LFLAGS "LFLAGS"
#define CONFIG_BD "BD"

#define CONFIG_DEFAULT_V_CC "gcc"
#define CONFIG_DEFAULT_V_CXX "g++"
#define CONFIG_DEFAULT_V_CFLAGS "-W -Wall"
#define CONFIG_DEFAULT_V_CXXFLAGS "-W -Wall"
#define CONFIG_DEFAULT_V_LFLAGS "-lm"
#define CONFIG_DEFAULT_V_BD "build"

#define CONFIG_BIN "bin%d"
#define CONFIG_OBJ "obj%d"
#define CONFIG_OBJ_BD "obj%dbd"

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

    std::map<std::string, std::string> executable;
    std::vector<std::string> ignores; // TODO add ignores?

    Config();

    const std::string &get_config(const std::string &config_name) const;
    void set_config(const std::string &config_name,
                    const std::string &config_value);

    std::string make_bin(int i) const;
    std::string make_obj(int i) const;
    std::string make_obj_bd(int i) const;

  private:
    std::string make(const std::string &s, int i) const;

  private:
    std::map<std::string, std::string> configs;
};

// int read_config(const char *filename, Config *cfg);

#endif //_AUTO_SCAN_CONFIG_H_
