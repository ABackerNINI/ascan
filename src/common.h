#ifndef _AUTO_SCAN_COMMON_H_
#define _AUTO_SCAN_COMMON_H_

#include <string>
#include <vector>

#define AS_VERSION "0.1.0 alpha"
#define AS_URL "git@github.com:ABackerNINI/ascan.git"

// Get file extention.
const char *get_ext(const char *filename, size_t length = 0);
// Get file extention.
const char *get_ext(const std::string &filename);

// Check if the file is exist
bool is_exist(const char *filename);
// Check if the file is exist
bool is_exist(const std::string &filename);

// Get date string, format: yyyy/mm/dd
void get_date(char *date);

// Check if the string are all digit numbers.
bool all_nums(const char *s);

// Read the file line by line.
bool read_file_by_line(std::vector<std::string> &lines, const char *file);

// Append contents to the file file line by line.
bool append_file_by_line(const char *file,
                         const std::vector<std::string> &lines);

size_t edit_distance(const char *s1, size_t len1, const char *s2, size_t len2);
size_t edit_distance(const std::string &source, const std::string &target);

bool contain_space(const std::string &s);

#endif //_AUTO_SCAN_COMMON_H_
