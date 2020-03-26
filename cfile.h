#pragma once

#ifndef _AUTO_SCAN_CFILES_H_
#define _AUTO_SCAN_CFILES_H_

#include <vector>
#include <string>

using std::string;
using std::vector;

class cfile {
   public:
    enum FILE_TYPE {
        FILE_TYPE_H,
        FILE_TYPE_C,
        FILE_TYPE_CPP,
        FILE_TYPE_CC,
        FILE_TYPE_ELSE
    };

   public:
    cfile(const string &filename, const string &name);
    void set_visited(bool visited);
    void match_includes(vector<cfile> &files);
    void associate_header(vector<cfile> &files);

    const string &filename() const;
    const string &name() const;
    const char *ext() const;
    bool have_main_func() const;
    enum FILE_TYPE file_type() const;
    bool is_header() const;
    bool is_source() const;
    bool is_c_source() const;
    bool is_cxx_source() const;
    const vector<cfile *> &includes() const;
    const cfile *associate() const;
    cfile *associate();
    bool visited() const;

   private:
    string m_filename;
    string m_name;
    const char *m_ext;
    bool m_have_main_func;
    bool m_includes_matched;
    enum FILE_TYPE m_file_type;
    cfile *m_associate;
    vector<cfile *> m_includes;

    bool m_visited;
};

vector<cfile> recursion_scan_dir_c_cxx_files(const char *dir);
vector<cfile> recursion_scan_dir_c_cxx_files(const string &dir);

#endif  //_AUTO_SCAN_CFILES_H_
