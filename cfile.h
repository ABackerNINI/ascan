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
        FILE_TYPE_H,    // c/c++ header file
        FILE_TYPE_C,    // c source file
        FILE_TYPE_CPP,  // c++ source file
        FILE_TYPE_CC,   // c++ source file
        FILE_TYPE_ELSE
    };

   public:
    cfile(const string &filename, const string &name);
    void set_visited(bool visited);
    void match_includes(vector<cfile> &files);
    void associate_header(vector<cfile> &files);

    // File name with extension.
    const string &filename() const;
    // File name without extension.
    const string &name() const;
    // File extension.
    const char *ext() const;
    // Whether the source file have a main function.
    bool have_main_func() const;
    // Get file type.
    enum FILE_TYPE file_type() const;
    // Whether if it is a header file.
    bool is_header() const;
    // Whether if it is a source file.
    bool is_source() const;
    // Whether if it is a c source file.
    bool is_c_source() const;
    // Whether if it is a c++ source file.
    bool is_cxx_source() const;
    // Get header files this file include.
    const vector<cfile *> &includes() const;
    // -- Get associated file.
    // -- Two files are associated if they have the same file name and one file
    // is a header file and the other is a source file, such as "foo.h" and
    // "foo.c".
    const cfile *associate() const;
    // -- Get associated file.
    // -- Two files are associated if they have the same file name and one file
    // is a header file and the other is a source file, such as "foo.h" and
    // "foo.c".
    cfile *associate();
    // For algorithm use.
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
