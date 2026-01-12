#ifndef _AUTO_SCAN_CFILES_H_
#define _AUTO_SCAN_CFILES_H_

#include "fs.h" // IWYU pragma: keep
#include <string>
#include <vector>

class cfile {
  public:
    enum FILE_TYPE {
        FILE_TYPE_H, // c/c++ header file
        FILE_TYPE_HPP, // c++ header file
        FILE_TYPE_C, // c source file
        FILE_TYPE_CPP, // c++ source file
        FILE_TYPE_CC, // c++ source file
        FILE_TYPE_ELSE
    };

    static const std::vector<std::string> file_type_ext;
    static const std::vector<FILE_TYPE> c_header_types;
    static const std::vector<FILE_TYPE> cxx_header_types;
    static const std::vector<FILE_TYPE> c_source_types;
    static const std::vector<FILE_TYPE> cxx_source_types;

  public:
    cfile(const fs::path &path);
    void set_have_main_func(bool have_main_func);
    void set_visited(bool visited);
    void match_includes_and_detect_main(std::vector<cfile> &files);
    void associate_header();

    // File name with extension.
    const fs::path &path() const;
    // File name without extension and directory.
    const std::string &stem() const;
    // Whether the source file have a main function.
    bool have_main_func() const;
    // Get file type.
    enum FILE_TYPE file_type() const;
    // Whether if it is a header file, e.g ".h" or ".hpp".
    bool is_header() const;
    // Whether if it is a source file, e.g ".c" or ".cpp".
    bool is_source() const;
    // Whether if it is a c header file, e.g ".h".
    bool is_c_header() const;
    // Whether if it is a c++ header file, e.g ".h" or ".hpp".
    bool is_cxx_header() const;
    // Whether if it is a c source file, e.g ".c".
    bool is_c_source() const;
    // Whether if it is a c++ source file, e.g ".cpp" or ".cc".
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
    // For algorithm.
    bool visited() const;

  private:
    // Return the corresponding file type of the given file extension.
    FILE_TYPE determine_type(const std::string &ext) const;
    // Check if the given type is one of the types.
    bool check_type(const std::vector<FILE_TYPE> &types, FILE_TYPE type) const;

  private:
    fs::path m_path;
    std::string m_stem;
    bool m_have_main_func;
    bool m_includes_matched;
    enum FILE_TYPE m_file_type;
    cfile *m_associate;
    std::vector<cfile *> m_includes;

    bool m_visited;
};

std::vector<fs::path> recursively_scan_dir_c_cxx_files(const char *dir, bool recursive);
std::vector<fs::path> recursively_scan_dir_c_cxx_files(const std::string &dir, bool recursive);

#endif //_AUTO_SCAN_CFILES_H_
