#ifndef _AUTO_SCAN_COMMON_H_
#define _AUTO_SCAN_COMMON_H_

#include "fs.h" // IWYU pragma: keep
#include <string>
#include <vector>

#define ASCAN_VERSION "0.1.3 alpha"
#define ASCAN_URL "git@github.com:ABackerNINI/ascan.git"

// Get file extention.
const char *get_ext(const char *filename, size_t length = 0);

// Get file extention.
const char *get_ext(const std::string &filename);

// Check if the string starts with the given prefix.
bool starts_with(const std::string &str, const std::string &prefix);

// Check if the string ends with the given suffix.
bool ends_with(const std::string &str, const std::string &suffix);

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
bool append_file_by_line(const char *file, const std::vector<std::string> &lines);

// Calculate the edit distance between two strings.
size_t edit_distance(const char *s1, size_t len1, const char *s2, size_t len2);

// Calculate the edit distance between two strings.
size_t edit_distance(const std::string &source, const std::string &target);

// Check if the string contain space(s).
bool contain_space(const std::string &s);

// Read file line by line and put them together in a string.
std::string read_file(const fs::path &path_);

// Write string to a file.
void write_file(const fs::path &path_, const std::string &content);

#endif //_AUTO_SCAN_COMMON_H_
