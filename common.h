#pragma once

#ifndef _AUTO_SCAN_COMMON_H_
#define _AUTO_SCAN_COMMON_H_

#include <string>
#include <vector>

using std::string;
using std::vector;

#define AS_VERSION "0.1.0"
#define AS_URL "git@github.com:ABackerNINI/ascan.git"

// Get file extention.
const char *get_ext(const char *filename, size_t length = 0);
// Get file extention.
const char *get_ext(const string &filename);

// Check if the file is exist
bool is_exist(const char *filename);
// Check if the file is exist
bool is_exist(const string &filename);

// Get date string, format: yyyy/mm/dd
void get_date(char *date);

// Check if the string are all digit numbers.
bool all_nums(const char *s);

size_t edit_distance(const char *s1, size_t len1, const char *s2, size_t len2);
size_t edit_distance(const string &source, const string &target);

#endif  //_AUTO_SCAN_COMMON_H_
