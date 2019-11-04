#include <vector>
#include <string>
#include <unistd.h>
#include <string.h>
#include <iostream>

#include "cfiles.h"
#include "ascan.h"
#include "parser.h"
#include "config.h"

using std::string;
using std::vector;

#define CONFIG_FILENAME "ascan.conf"
#define CONFIG_DEFAULT_CC "gcc"
#define CONFIG_DEFAULT_CXX "g++"
#define CONFIG_DEFAULT_CFLAG "-W -Wall -lm"
#define CONFIG_DEFAULT_CXXFLAG "-W -Wall"

// bool test_config(const char *dir, string &config_filename) {}

bool test_makefile(const char *dir, string &makefile, bool force) {
    string makefile1 = string(dir) + "/Makefile";
    string makefile2 = string(dir) + "/makefile";
    bool e = false;
    if (is_exist(makefile1.c_str())) {
        makefile = makefile1;
        e = true;
    } else if (is_exist(makefile2.c_str())) {
        makefile = makefile2;
        e = true;
    } else {
        makefile = makefile1;
    }

    if (!force && e) {
        printf(
            "There is already one Makefile in \"%s\", "
            "overwrite it? [yes/No]",
            dir);
        char cmd[BUFSIZ];
        if (fgets(cmd, BUFSIZ, stdin)) {
            if (strcasecmp(cmd, "y\n") != 0 && strcasecmp(cmd, "yes\n") != 0) {
                return false;
            }
        } else {
            return false;
        }
    }

    return true;
}

void match_c_cpp_includes(vector<cfiles> &files) {
    for (auto file = files.begin(); file != files.end(); ++file) {
        if (file->file_type() == cfiles::C ||
            file->file_type() == cfiles::CPP) {
            printf("%s\n", file->filename().c_str());
            file->match_includes(files);
            for (auto include = file->includes().begin();
                 include != file->includes().end(); ++include) {
                (*include)->match_includes(files);
            }
        }
    }
}

void associate(vector<cfiles> &files) {
    for (auto file = files.begin(); file != files.end(); ++file) {
        if (file->file_type() == cfiles::C ||
            file->file_type() == cfiles::CPP) {
            file->associate_h(files);
        }
    }
}

int main(int argc, char *argv[]) {
    // -f: force overwrite
    // -cc gcc|g++|cc: set c compiler, default: gcc
    // -cxx g++: set c++ compiler, default: g++
    // -cflag "": set c flags, default: -W -Wall -lm
    // -cxxflag "" set c++ flags, default: -W -Wall

    bool force = false;
    char dir[BUFSIZ];
    getcwd(dir, BUFSIZ);

    Config cfg;
    cfg.cc = CONFIG_DEFAULT_CC;
    cfg.cxx = CONFIG_DEFAULT_CXX;
    cfg.cflag = CONFIG_DEFAULT_CFLAG;
    cfg.cxxflag = CONFIG_DEFAULT_CXXFLAG;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-f") == 0) {
            force = true;
        }
    }

    // string config_filename = string(dir) + CONFIG_FILENAME;
    // if (!is_exist(config_filename.c_str())) {
    //     output_config(config_filename, cfg);
    // }

    // read_config((string(dir) + "/ascan.conf").c_str(), &cfg);

    string makefile;
    if (!test_makefile(dir, makefile, force)) {
        return EXIT_SUCCESS;
    }

    vector<cfiles> files = recursion_scan_dir_c_cxx_files(dir);

    match_c_cpp_includes(files);
    associate(files);

    output_makefile(makefile, files, cfg);

    return EXIT_SUCCESS;
}