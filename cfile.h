#ifndef _AUTO_SCAN_CFILES_H_
#define _AUTO_SCAN_CFILES_H_

#include <string>
#include <vector>

class cfile {
  public:
    enum FILE_TYPE {
        FILE_TYPE_H,   // c/c++ header file
        FILE_TYPE_C,   // c source file
        FILE_TYPE_CPP, // c++ source file
        FILE_TYPE_CC,  // c++ source file
        FILE_TYPE_ELSE
    };

  public:
    cfile(const std::string &filename, const std::string &name);
    void set_visited(bool visited);
    void match_includes(std::vector<cfile> &files);
    void associate_header(std::vector<cfile> &files);

    // File name with extension.
    const std::string &filename() const;
    // File name without extension and directory.
    const std::string &name() const;
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
    const std::vector<cfile *> &includes() const;
    // Get associated file.
    // Two files are associated if they have the same file name and one file
    // is a header file and the other is a source file, such as "foo.h" and
    // "foo.c".
    const cfile *associate() const;
    // Get associated file.
    // Two files are associated if they have the same file name and one file
    // is a header file and the other is a source file, such as "foo.h" and
    // "foo.c".
    cfile *associate();
    // For algorithm use.
    bool visited() const;

  private:
    std::string m_filename;
    std::string m_name;
    bool m_have_main_func;
    bool m_includes_matched;
    enum FILE_TYPE m_file_type;
    cfile *m_associate;
    std::vector<cfile *> m_includes;

    bool m_visited;
};

std::vector<cfile> recursion_scan_dir_c_cxx_files(const char *dir);
std::vector<cfile> recursion_scan_dir_c_cxx_files(const std::string &dir);

#endif //_AUTO_SCAN_CFILES_H_
