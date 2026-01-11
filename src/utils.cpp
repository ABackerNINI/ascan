#include "utils.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <ios>
#include <iostream>
#include <unistd.h>

using namespace std;

const char *get_ext(const char *filename, size_t length) {
    if (length == 0) {
        length = strlen(filename);
    }

    for (const char *p = filename + length - 1; length > 0; --p, --length) {
        if (*p == '.') {
            return p;
        }
    }
    return NULL;
}

const char *get_ext(const string &filename) {
    return get_ext(filename.c_str(), filename.length());
}

bool starts_with(const std::string &str, const std::string &prefix) {
    return str.length() >= prefix.length() && str.compare(0, prefix.length(), prefix) == 0;
}

bool ends_with(const std::string &str, const std::string &suffix) {
    return str.length() >= suffix.length() && str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

bool is_exist(const char *filename) {
    return access(filename, F_OK) == 0;
}

bool is_exist(const string &filename) {
    return is_exist(filename.c_str());
}

void get_date(char *date) {
    time_t t = time(NULL);
    tm *ltm = localtime(&t);
    sprintf(date, "%4d/%02d/%02d", ltm->tm_year + 1900, ltm->tm_mon + 1, ltm->tm_mday);
}

bool all_nums(const char *s) {
    if (*s == '\0') {
        return false;
    }

    while (*s && isdigit(*s)) {
        ++s;
    }
    return *s == '\0';
}

bool read_file(std::vector<std::string> &lines, const char *file) {
    ifstream input(file);
    if (!input) {
        return false;
    }
    string line;
    while (getline(input, line)) {
        lines.push_back(line);
    }
    input.close();
    return true;
}

bool append_file_by_line(const char *file, const std::vector<std::string> &lines) {
    ofstream output(file, ios_base::app);
    if (!output) {
        return false;
    }
    for (auto &line : lines) {
        output << line << "\n";
    }
    output.close();
    return true;
}

size_t edit_distance(const char *s1, size_t len1, const char *s2, size_t len2) {
#define DP(i, j) dp[(i) * (len2 + 1) + (j)]
    // dp should be longer than dp[(len1+1)*(len2+1)]
    size_t *dp = new size_t[(len1 + 1) * (len2 + 1)];
    for (size_t i = 0; i <= len1; ++i) {
        DP(i, 0) = i;
    }
    for (size_t i = 0; i <= len2; ++i) {
        DP(0, i) = i;
    }

    size_t flag;
    for (size_t i = 1; i <= len1; ++i) {
        for (size_t j = 1; j <= len2; ++j) {
            flag = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            DP(i, j) = std::min(DP(i - 1, j) + 1, std::min(DP(i, j - 1) + 1, DP(i - 1, j - 1) + flag));
        }
    }

    size_t ret = DP(len1, len2);

    delete[] dp;

    return ret;
#undef DP
}

size_t edit_distance(const string &s1, const string &s2) {
    return edit_distance(s1.c_str(), s1.length(), s2.c_str(), s2.length());
}

bool contain_space(const std::string &s) {
    for (auto &c : s) {
        if (isspace(c)) {
            return true;
        }
    }
    return false;
}

std::string read_file(const fs::path &path_) {
    std::ifstream infile(path_);
    if (!infile) {
        std::cerr << "Can't open file: " << path_ << std::endl;
        exit(1);
    }

    std::string content;
    content.reserve(1024);

    char buf[1024];
    std::streamsize bytes_read = 0;
    while (infile.read(buf, 1024), (bytes_read = infile.gcount()) > 0) {
        content.append(buf, bytes_read);
    }

    if (!infile.eof()) {
        if (infile.fail()) {
            throw std::runtime_error("Error: Failed to read from file.");
        } else if (infile.bad()) {
            throw std::runtime_error("Error: Bad stream state.");
        }
    }

    return content;
}

void write_file(const fs::path &path_, const std::string &content) {
    std::ofstream outfile(path_);
    if (!outfile) {
        std::cerr << "Can't open file: " << path_ << std::endl;
        exit(1);
    }
    outfile << content;
}

std::string vector_to_string(const std::vector<std::string> &vec,
                             const std::string &sep,
                             const std::string &default_for_empty) {
    if (vec.empty()) {
        return default_for_empty;
    }

    std::string result = "";
    for (const auto &item : vec) {
        result += item + sep;
    }
    if (result.size() > 0) {
        for (size_t i = 0; i < sep.size(); i++) {
            result.pop_back();
        }
    }
    return result;
}
