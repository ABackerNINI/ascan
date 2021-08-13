#ifndef _AUTO_SCAN_CONFIG_H_
#define _AUTO_SCAN_CONFIG_H_

#include <map>
#include <string>
#include <vector>

#define CONFIG_FILENAME "ascan.conf"

#define CONFIG_DEFAULT_OUTPUT "Makefile"

#define CONFIG_DEFAULT_K_CC "CC"
#define CONFIG_DEFAULT_K_CXX "CXX"
#define CONFIG_DEFAULT_K_CFLAGS "CFLAGS"
#define CONFIG_DEFAULT_K_CXXFLAGS "CXXFLAGS"
#define CONFIG_DEFAULT_K_BD "BD"
#define CONFIG_DEFAULT_K_BIN "bin%d"
#define CONFIG_DEFAULT_K_OBJ "obj%d"
#define CONFIG_DEFAULT_K_OBJ_BD "obj%dbd"

#define CONFIG_DEFAULT_V_CC "gcc"
#define CONFIG_DEFAULT_V_CXX "g++"
#define CONFIG_DEFAULT_V_CFLAG "-W -Wall -lm"
#define CONFIG_DEFAULT_V_CXXFLAG "-W -Wall"
#define CONFIG_DEFAULT_V_BD "./build"

class Config {
  public:
    // User specified output file name.
    std::string output;
    // User specified start files. Dependencies will be generated start from
    // them. If not specified, ascan will scan c/c++ source files in the current
    // directory that contains main functions and start from them.
    std::vector<std::string> start_files;

    std::string k_cc;
    std::string k_cxx;
    std::string k_cflags;
    std::string k_cxxflags;
    std::string k_bd;
    std::string k_bin;
    std::string k_obj;
    std::string k_obj_bd;

    std::string v_cc;
    std::string v_cxx;
    std::string v_cflag;
    std::string v_cxxflag;
    std::string v_bd;

    std::map<std::string, std::string> executable;
    std::vector<std::string> ignores; // TODO add ignores?

    Config();

    std::string make_bin(int i) const;
    std::string make_obj(int i) const;
    std::string make_obj_bd(int i) const;

  private:
    std::string make(const std::string &s, int i) const;
};

int read_config(const char *filename, Config *cfg);

#endif //_AUTO_SCAN_CONFIG_H_
