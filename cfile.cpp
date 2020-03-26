#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

#include "common.h"
#include "cfile.h"
#include "parser.h"

/*==========================================================================*/

cfile::cfile(const string &filename, const string &name)
    : m_filename(filename), m_name(name) {
    m_ext = get_ext(m_filename.c_str());
    m_have_main_func = false;
    m_includes_matched = false;

    if (strcmp(m_ext, ".h") == 0) {
        m_file_type = FILE_TYPE_H;
    } else if (strcmp(m_ext, ".c") == 0) {
        m_file_type = FILE_TYPE_C;
    } else if (strcmp(m_ext, ".cpp") == 0) {
        m_file_type = FILE_TYPE_CPP;
    } else if (strcmp(m_ext, ".cc") == 0) {
        m_file_type = FILE_TYPE_CC;
    } else {
        m_file_type = FILE_TYPE_ELSE;
    }

    m_associate = NULL;
    m_visited = false;
}

void cfile::set_visited(bool visited) { m_visited = visited; }

void cfile::match_includes(vector<cfile> &files) {
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

void cfile::associate_header(vector<cfile> &files) {
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

const string &cfile::filename() const { return m_filename; }

const string &cfile::name() const { return m_name; }

const char *cfile::ext() const { return m_ext; }

bool cfile::have_main_func() const { return m_have_main_func; }

enum cfile::FILE_TYPE cfile::file_type() const { return m_file_type; }

bool cfile::is_header() const { return file_type() == FILE_TYPE_H; }

bool cfile::is_source() const {
    return file_type() == FILE_TYPE_C || file_type() == FILE_TYPE_CPP ||
           file_type() == FILE_TYPE_CC;
}

bool cfile::is_c_source() const { return file_type() == FILE_TYPE_C; }

bool cfile::is_cxx_source() const {
    return file_type() == FILE_TYPE_CPP || file_type() == FILE_TYPE_CC;
}

const vector<cfile *> &cfile::includes() const { return m_includes; }

const cfile *cfile::associate() const { return m_associate; }

cfile *cfile::associate() { return m_associate; }

bool cfile::visited() const { return m_visited; }

/*==========================================================================*/

static bool recursion_scan_dir_c_cxx_files_helper(const char *dir,
                                                  vector<cfile> &vec,
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
                        !strcmp(ext, ".cpp") ||
                        !strcmp(ext, ".cc") /*|| !strcmp(ext, ".hpp")*/)) {
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

vector<cfile> recursion_scan_dir_c_cxx_files(const char *dir) {
    vector<cfile> vec;

    char *cur_path = getcwd(NULL, 0);

    string path;
    path.reserve(128);
    recursion_scan_dir_c_cxx_files_helper(dir, vec, path);

    chdir(cur_path);

    free(cur_path);

    return vec;
}

vector<cfile> recursion_scan_dir_c_cxx_files(const string &dir) {
    return recursion_scan_dir_c_cxx_files(dir.c_str());
}

/*==========================================================================*/
