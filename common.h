#pragma once

#ifndef _AUTO_SCAN_COMMON_H_
#define _AUTO_SCAN_COMMON_H_

#include <string>

using std::string;

#define DEBUG 1

const char *get_ext(const char *filename, size_t length = 0);
const char *get_ext(const string &filename, size_t length = 0);

/* Check if the file is exist */
bool is_exist(const char *filename);
/* Check if the file is exist */
bool is_exist(const string &filename);

#endif  //_AUTO_SCAN_COMMON_H_
