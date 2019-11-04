#pragma once

#ifndef _AUTO_SCAN_ASCAN_H_
#define _AUTO_SCAN_ASCAN_H_

#include <vector>
#include <iostream>
#include "cfiles.h"
#include "config.h"

using std::vector;

// void output_config(const string &config, Config &cfg);

void output_makefile(const string &makefile, vector<cfiles> &files,
                     Config &cfg);

#endif  //_AUTO_SCAN_ASCAN_H_
