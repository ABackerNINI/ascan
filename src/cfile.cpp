#include "cfile.h"
#include "common.h"
#include "debug.h"
#include "parser.h"
#include <cassert>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

const std::vector<std::string> cfile::file_type_ext = {".h", ".hpp", ".c", ".cpp", ".cc"};
const std::vector<cfile::FILE_TYPE> cfile::c_header_types = {FILE_TYPE_H};
const std::vector<cfile::FILE_TYPE> cfile::cxx_header_types = {FILE_TYPE_H, FILE_TYPE_HPP};
const std::vector<cfile::FILE_TYPE> cfile::c_source_types = {FILE_TYPE_C};
const std::vector<cfile::FILE_TYPE> cfile::cxx_source_types = {FILE_TYPE_CPP, FILE_TYPE_CC};

/*==========================================================================*/

cfile::cfile(const string &filename, const string &name) : m_filename(filename), m_name(name) {
    //! Be careful with get_ext(std::string.c_str()), the c_str() return value
    //! is a dynamic memory which could move to another place when you modified
    //! the string.
    const char *ext = get_ext(m_filename.c_str());

    // TODO: check if filename contains spaces

    assert(ext);

    m_have_main_func = false;
    m_includes_matched = false;

    m_file_type = determine_type(ext);

    m_associate = NULL;
    m_visited = false;
}

void cfile::set_have_main_func(bool have_main_func) {
    m_have_main_func = have_main_func;
}

void cfile::set_visited(bool visited) {
    m_visited = visited;
}

void cfile::match_includes_and_detect_main(vector<cfile> &files) {
    if (m_includes_matched) {
        return;
    }

    vector<string> includes = scan_includes_and_main_func(m_filename.c_str(), &m_have_main_func);

    for (auto include = includes.begin(); include != includes.end(); ++include) {
        bool found = false;
        for (auto file = files.begin(); file != files.end(); ++file) {
            if (*include == file->filename()) {
                m_includes.push_back(&(*file));
                found = true;
                break;
            }
        }
        if (!found) {
            fprintf(stderr, "File \"%s\" included by \"%s\" not found\n", include->c_str(), m_filename.c_str());
        }
    }
    m_includes_matched = true;
}

void cfile::associate_header(vector<cfile> &files) {
    assert(is_source());
    // TODO: multi-directory makefile
    for (auto file = files.begin(); file != files.end(); ++file) {
        if (file->is_header() && m_name == file->m_name) {
            m_associate = &(*file);
            file->m_associate = this;
            print_debug("associated: %s <-> %s\n", m_filename.c_str(), file->m_filename.c_str());
            break;
        }
    }
}

const string &cfile::filename() const {
    return m_filename;
}

const string &cfile::name() const {
    return m_name;
}

bool cfile::have_main_func() const {
    return m_have_main_func;
}

enum cfile::FILE_TYPE cfile::file_type() const {
    return m_file_type;
}

bool cfile::is_header() const {
    return is_c_header() || is_cxx_header();
}

bool cfile::is_source() const {
    return is_c_source() || is_cxx_source();
}

bool cfile::is_c_header() const {
    return check_type(c_header_types, file_type());
}

bool cfile::is_cxx_header() const {
    return check_type(cxx_header_types, file_type());
}

bool cfile::is_c_source() const {
    return check_type(c_source_types, file_type());
}

bool cfile::is_cxx_source() const {
    return check_type(cxx_source_types, file_type());
}

const vector<cfile *> &cfile::includes() const {
    return m_includes;
}

const cfile *cfile::associate() const {
    return m_associate;
}

cfile *cfile::associate() {
    return m_associate;
}

bool cfile::visited() const {
    return m_visited;
}

cfile::FILE_TYPE cfile::determine_type(const std::string &ext) const {
    for (int i = 0; i < (int)FILE_TYPE::FILE_TYPE_ELSE; ++i) {
        if (ext == cfile::file_type_ext[i]) {
            return (FILE_TYPE)i;
        }
    }
    assert(false);
    return FILE_TYPE_ELSE;
}

bool cfile::check_type(const std::vector<FILE_TYPE> &types, FILE_TYPE type) const {
    for (auto &t : types) {
        if (type == t) {
            return true;
        }
    }
    return false;
}

/*==========================================================================*/

bool is_concerned_file_type(const std::string &ext) {
    for (auto &t : cfile::file_type_ext) {
        if (ext == t) {
            return true;
        }
    }
    return false;
}

// TODO: rewrite it using std::filesystem
static void recursion_scan_dir_c_cxx_files_helper(char *path, vector<cfile> &vec, bool recursive) {
    DIR *p_dir = NULL;
    struct dirent *p_entry = NULL;
    struct stat statbuf;

    if ((p_dir = opendir(path)) == NULL) {
        fprintf(stderr, "Can't open dir \"%s\". @%s line %d\n", path, __func__, __LINE__);
        return;
    }

    size_t len = strlen(path);

    if (path[len - 1] != '/' && path[len - 1] != '\\') {
        path[len++] = '/';
        path[len] = '\0';
    }

    while (NULL != (p_entry = readdir(p_dir))) {
        // Ignore "." and ".." directories
        if (strcmp(p_entry->d_name, ".") == 0 || strcmp(p_entry->d_name, "..") == 0) {
            continue;
        }

        if (len + strlen(p_entry->d_name) + 1 <= FILENAME_MAX) {
            strcpy(path + len, p_entry->d_name);
        } else {
            fprintf(stderr, "File name too long: %s/%s. @%s line %d\n", path, p_entry->d_name, __func__, __LINE__);
            continue;
        }

        if (lstat(path, &statbuf) == 0) {
            if ((statbuf.st_mode & S_IFMT) == S_IFDIR) { /* dir */
                recursion_scan_dir_c_cxx_files_helper(path, vec, recursive);
            } else if ((statbuf.st_mode & S_IFMT) == S_IFREG) { /* regular file */
                const char *ext = get_ext(p_entry->d_name);
                if (ext && is_concerned_file_type(ext)) {
                    vec.emplace_back(std::string(path), string(p_entry->d_name).substr(0, ext - p_entry->d_name));
                }
            } else {
                print_warning("Not a regular file or directory: \"%s\"\n", p_entry->d_name);
            }
        } else {
            print_warning("Can't lstat file/directory: \"%s\"\n", p_entry->d_name);
        }
    }

    // END:
    closedir(p_dir);
}

vector<cfile> recursion_scan_dir_c_cxx_files(const char *dir, bool recursive) {
    vector<cfile> vec;

    char path[FILENAME_MAX];
    strcpy(path, dir);
    recursion_scan_dir_c_cxx_files_helper(path, vec, recursive);

    return vec;
}

vector<cfile> recursion_scan_dir_c_cxx_files(const string &dir, bool recursive) {
    return recursion_scan_dir_c_cxx_files(dir.c_str(), recursive);
}

/*==========================================================================*/
