#pragma once

#ifndef _AUTO_SCAN_CFILES_H_
#define _AUTO_SCAN_CFILES_H_

#include <vector>
#include <string>

using std::string;
using std::vector;

class cfiles {
   public:
    enum FILE_TYPE { H, C, CPP,ELSE };

   public:
    cfiles(const string &filename, const string &name);
    void set_visited(bool visited);
    void match_includes(vector<cfiles> &files);
    void associate_h(vector<cfiles> &files);

    const string &filename() const;
    const string &name() const;
    const char *ext() const;
    bool have_main_func() const;
    enum FILE_TYPE file_type() const;
    const vector<cfiles *> &includes() const;
    const cfiles *associate() const;
    cfiles *associate();
    bool visited() const;

   private:
    string m_filename;
    string m_name;
    const char *m_ext;
    bool m_have_main_func;
    bool m_includes_matched;
    enum FILE_TYPE m_file_type;
    cfiles *m_associate;
    vector<cfiles *> m_includes;

    bool m_visited;
};

bool is_exist(const char *filename);

const char *get_ext(const char *filename, size_t length = 0);

vector<cfiles> recursion_scan_dir_c_cxx_files(const char *dir);

#endif  //_AUTO_SCAN_CFILES_H_
