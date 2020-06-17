#pragma once

#ifndef _AUTO_SCAN_CONFIG_H_
#define _AUTO_SCAN_CONFIG_H_

#include <map>
#include <vector>
#include <string>

using std::map;
using std::string;
using std::vector;

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
    string output;

    string k_cc;
    string k_cxx;
    string k_cflags;
    string k_cxxflags;
    string k_bd;
    string k_bin;
    string k_obj;
    string k_obj_bd;

    string v_cc;
    string v_cxx;
    string v_cflag;
    string v_cxxflag;
    string v_bd;

    map<string, string> executable;
    vector<string> ignores;

    Config();

    string make_bin(int i) const;
    string make_obj(int i) const;
    string make_obj_bd(int i) const;

   private:
    string make(const string &s, int i) const;
};

int read_config(const char *filename, Config *cfg);

#endif  //_AUTO_SCAN_CONFIG_H_
