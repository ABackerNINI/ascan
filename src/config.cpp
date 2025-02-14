#include "config.h"
#include "common.h"
#include "debug.h"
#include <cassert>
#include <iostream>

using namespace std;

Config::Config() : output(CONFIG_DEFAULT_OUTPUT), k_bin(CONFIG_BIN), k_obj(CONFIG_OBJ), k_obj_bd(CONFIG_OBJ_BD) {
    configs[CONFIG_CC]       = CONFIG_DEFAULT_V_CC;
    configs[CONFIG_CXX]      = CONFIG_DEFAULT_V_CXX;
    configs[CONFIG_CFLAGS]   = CONFIG_DEFAULT_V_CFLAGS;
    configs[CONFIG_CXXFLAGS] = CONFIG_DEFAULT_V_CXXFLAGS;
    configs[CONFIG_LDFLAGS]  = CONFIG_DEFAULT_V_LFLAGS;
    configs[CONFIG_BD]       = CONFIG_DEFAULT_V_BD;

    // These are variable names in the Makefile, thus can NOT contain spaces
    check_space(CONFIG_CC);
    check_space(CONFIG_CXX);
    check_space(CONFIG_CFLAGS);
    check_space(CONFIG_CXXFLAGS);
    check_space(CONFIG_LDFLAGS);

    // These may show in the clean section of the Makefile, thus can NOT contain
    // spaces
    check_space(CONFIG_FILENAME);
    check_space(CONFIG_DEPENDENCIES_FILENAME);
    check_space(CONFIG_DEFAULT_V_BD);
    check_space(CONFIG_BD);
    check_space(CONFIG_BIN);
    check_space(CONFIG_OBJ);
    check_space(CONFIG_OBJ_BD);

    // These may show in the clean section of the Makefile, thus can NOT start
    // with '/'
    check_slash(CONFIG_FILENAME);
    check_slash(CONFIG_DEPENDENCIES_FILENAME);
    check_slash(CONFIG_DEFAULT_V_BD);
    check_slash(CONFIG_BD);
    check_slash(CONFIG_BIN);
    check_slash(CONFIG_OBJ);
    check_slash(CONFIG_OBJ_BD);
}

const std::string &Config::get(const std::string &config_name) const {
    auto config = configs.find(config_name);
    if (config == configs.end()) {
        print_error("no such config: \"%s\"", config_name.c_str());
        exit(EXIT_FAILURE);
    }
    return config->second;
}

void Config::set(const std::string &config_name, const std::string &config_value) {
    configs[config_name] = config_value;
}

string Config::make_bin(int i) const {
    return make(k_bin, i);
}

string Config::make_obj(int i) const {
    return make(k_obj, i);
}

string Config::make_obj_bd(int i) const {
    return make(k_obj_bd, i);
}

string Config::make(const string &s, int i) const {
    char tmp[BUFSIZ];
    if (i == -1) {
        // do not show %d
        string k_bin2 = s;
        auto it       = k_bin2.find("%d");
        assert(it != k_bin2.npos);
        k_bin2.erase(it, 2);
        sprintf(tmp, k_bin2.c_str(), i);
    } else {
        sprintf(tmp, s.c_str(), i);
    }
    return string(tmp);
}

void Config::check_space(const std::string &config) const {
    if (contain_space(config)) {
        cerr << "The configuration value \"" << config << "\" contains space, which is not allowed!" << endl;
        exit(1);
    }
}

void Config::check_slash(const std::string &config) const {
    if (config.find('/') != config.npos) {
        cerr << "The configuration value \"" << config << "\" contains '/', which is not allowed!" << endl;
        exit(1);
    }
}
