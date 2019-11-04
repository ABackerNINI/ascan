#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

#include "cfiles.h"
#include "parser.h"

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

cfiles::cfiles(const string &filename, const string &name)
    : m_filename(filename), m_name(name) {
    m_ext = get_ext(m_filename.c_str());
    m_have_main_func = false;
    m_includes_matched = false;
    m_file_type = strcmp(m_ext, ".h") == 0
                      ? H
                      : (strcmp(m_ext, ".c") == 0
                             ? C
                             : (strcmp(m_ext, ".cpp") == 0 ? CPP : ELSE));
    m_associate = NULL;
    m_visited = false;
}

void cfiles::set_visited(bool visited) { m_visited = visited; }

void cfiles::match_includes(vector<cfiles> &files) {
    if (m_includes_matched) {
        return;
    }

    vector<string> includes =
        scan_includes_and_main_func(m_filename.c_str(), &m_have_main_func);

    for (auto include = includes.begin(); include != includes.end();
         ++include) {
        bool found = false;
        for (auto file = files.begin(); file != files.end(); ++file) {
            if (*include == file->filename()) {
                m_includes.push_back(&(*file));
                found = true;
                break;
            }
        }
        if (!found) {
            fprintf(stderr, "File \"%s\" included by \"%s\" not found\n",
                    include->c_str(), m_filename.c_str());
        }
    }
    m_includes_matched = true;
}

void cfiles::associate_h(vector<cfiles> &files) {
    size_t len = m_filename.length() - strlen(m_ext);
    for (auto file = files.begin(); file != files.end(); ++file) {
        if (strncmp(m_filename.c_str(), file->m_filename.c_str(), len) == 0 &&
            &(*file) != this &&
            file->m_filename == m_filename.substr(0, len) + ".h") {
            m_associate = &(*file);
            file->m_associate = this;
            printf("associated: %s <-> %s\n", m_filename.c_str(),
                   file->m_filename.c_str());
            break;
        }
    }
}

const string &cfiles::filename() const { return m_filename; }

const string &cfiles::name() const { return m_name; }

const char *cfiles::ext() const { return m_ext; }

bool cfiles::have_main_func() const { return m_have_main_func; }

enum cfiles::FILE_TYPE cfiles::file_type() const { return m_file_type; }

const vector<cfiles *> &cfiles::includes() const { return m_includes; }

const cfiles *cfiles::associate() const { return m_associate; }

cfiles *cfiles::associate() { return m_associate; }

bool cfiles::visited() const { return m_visited; }

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

bool is_exist(const char *filename) { return access(filename, F_OK) == 0; }

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

static bool recursion_scan_dir_c_cxx_files_helper(const char *dir,
                                                  vector<cfiles> &vec,
                                                  string &path) {
    DIR *p_dir = NULL;
    struct dirent *p_entry = NULL;
    struct stat statbuf;
    bool ret = true;

    if ((p_dir = opendir(dir)) == NULL) {
        fprintf(stderr, "Can't open dir \"%s\". @%s line %d\n", dir, __func__,
                __LINE__);
        return false;
    }

    chdir(dir);

    path = path + dir + "/";
    // std::cout << path << std::endl;
    while (NULL != (p_entry = readdir(p_dir))) {
        lstat(p_entry->d_name, &statbuf);

        if (S_IFDIR & statbuf.st_mode) {
            // if (strcmp(".", p_entry->d_name) != 0 &&
            //     strcmp("..", p_entry->d_name) != 0) {
            //     if (!recursion_scan_dir_c_cxx_files_helper(p_entry->d_name,
            //     vec,
            //                                                path)) {
            //         ret = false;
            //         goto END;
            //     }
            // }
        } else {
            const char *ext = get_ext(p_entry->d_name);
            if (ext && (!strcmp(ext, ".h") || !strcmp(ext, ".c") ||
                        !strcmp(ext, ".cpp") /*|| !strcmp(ext, ".hpp")*/)) {
                vec.emplace_back(
                    path + p_entry->d_name,
                    string(p_entry->d_name).substr(0, ext - p_entry->d_name));
            }
        }
    }

    // END:
    path.resize(path.size() - strlen(dir) - 1);
    chdir("..");
    closedir(p_dir);

    return ret;
}

vector<cfiles> recursion_scan_dir_c_cxx_files(const char *dir) {
    vector<cfiles> vec;

    char *cur_path = getcwd(NULL, 0);

    string path;
    path.reserve(128);
    recursion_scan_dir_c_cxx_files_helper(dir, vec, path);

    chdir(cur_path);

    free(cur_path);

    return vec;
}

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
