#include <string.h>
#include <unistd.h>
#include <iostream>

#include "ascan.h"
#include "mfile.h"
#include "common.h"

ascan::ascan(int argc, char **argv) {
    char dir[BUFSIZ];
    getcwd(dir, BUFSIZ);
    m_cwd = string(dir);

    parse_cmd_args(argc, argv);

    m_cfg.cc = CONFIG_DEFAULT_CC;
    m_cfg.cxx = CONFIG_DEFAULT_CXX;
    m_cfg.cflag = CONFIG_DEFAULT_CFLAG;
    m_cfg.cxxflag = CONFIG_DEFAULT_CXXFLAG;
}

int ascan::start() {
    if (!test_makefile()) {
        return EXIT_SUCCESS;
    }
    m_cfiles = recursion_scan_dir_c_cxx_files(m_cwd);

    match_c_cpp_includes();
    associate_header();

    mfile mf(m_makefile, m_cfiles, m_cfg);

    return mf.output();
}

/*==========================================================================*/

void ascan::parse_cmd_args(int argc, char **argv) {
    // -f: force overwrite
    // -cc gcc|g++|cc: set c compiler, default: gcc
    // -cxx g++: set c++ compiler, default: g++
    // -cflag "": set c flags, default: -W -Wall -lm
    // -cxxflag "" set c++ flags, default: -W -Wall
    m_flag_f = false;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-f") == 0) {
            m_flag_f = true;
        }
    }
}

void ascan::print_help() {}

bool ascan::test_makefile() {
    string makefile1 = m_cwd + "/Makefile";
    string makefile2 = m_cwd + "/makefile";

    int exist = 0;
    if (is_exist(makefile1)) {
        m_makefile = makefile1;
        exist = 1;
    } else if (is_exist(makefile2)) {
        m_makefile = makefile2;
        exist = 2;
    } else {
        // default makefile name
        m_makefile = makefile1;
    }

    if (!m_flag_f && exist) {
        printf(
            "There is already one %s in \"%s\", "
            "overwrite it? [yes/No] ",
            (exist == 1 ? "Makefile" : "makefile"), m_cwd.c_str());

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

void ascan::match_c_cpp_includes() {
    for (auto file = m_cfiles.begin(); file != m_cfiles.end(); ++file) {
        if (file->file_type() == cfile::C || file->file_type() == cfile::CPP) {
            printf("%s\n", file->filename().c_str());
            file->match_includes(m_cfiles);
            for (auto include = file->includes().begin();
                 include != file->includes().end(); ++include) {
                (*include)->match_includes(m_cfiles);
            }
        }
    }
}

void ascan::associate_header() {
    for (auto file = m_cfiles.begin(); file != m_cfiles.end(); ++file) {
        if (file->file_type() == cfile::C || file->file_type() == cfile::CPP) {
            file->associate_header(m_cfiles);
        }
    }
}

/*==========================================================================*/

int main(int argc, char **argv) {
    ascan ascan(argc, argv);

    return ascan.start();
}
