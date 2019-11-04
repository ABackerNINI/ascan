#pragma once

#ifndef _AUTO_SCAN_CONFIG_H_
#define _AUTO_SCAN_CONFIG_H_

#include <map>
#include <vector>
#include <string>

using std::map;
using std::string;
using std::vector;

struct Config {
    string cc;
    string cxx;
    string cflag;
    string cxxflag;
    map<string, string> executable;
    vector<string> ignores;
};

int read_config(const char *filename, Config *cfg);

#endif  //_AUTO_SCAN_CONFIG_H_
