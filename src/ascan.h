#ifndef _AUTO_SCAN_ASCAN_H_
#define _AUTO_SCAN_ASCAN_H_

#include "cfile.h"
#include "options.h"
#include <string>
#include <vector>

#define ASCAN_VERSION "0.1.3 alpha"
#define ASCAN_URL "git@github.com:ABackerNINI/ascan.git"

class ascan {
  public:
    ascan();
    // Start to proceed auto-scan.
    //
    // Return:
    // -- EXIT_SUCCESS: if no error occurred.
    // -- EXIT_FAILURE: otherwise.
    int start(int argc, char **argv);

  private:
    enum HELP_TYPE { HT_NONE, HT_ALL, HT_VER, HT_SPECIFIC };

    // Parse the command arguments.
    //
    // Return:
    // -- > 0: an error occurred.
    // -- < 0: if one of the argument '--help', '-v', '--ver' is presented or
    // an error occurred.
    // -- 0: otherwise.
    int parse_cmd_args(int argc, char **argv);

    void print_help(enum HELP_TYPE help, const options::as_option *option) const;

    // Check if there is one makefile, prompt overwrite when -f is not set.
    //
    // Return:
    // -- true: if -f is set or user entered 'yes' to overwrite or no makefile
    // found.
    // -- false: otherwise.
    bool test_makefile(bool force, bool output_specified);

    void match_starter_files(const std::vector<std::string> &start_files);

    void match_c_cxx_includes();

    void associate_header();

    void print_cfiles() const;

  private:
    // int m_error; // whether there is an error during parsing args
    // uint32_t m_flags; // contains all options
    // Config m_cfg; // config
    // string m_cwd;            // current working dir
    std::vector<cfile> m_cfiles; // cfiles
    // options m_options;
};

int main(int argc, char **argv);

#endif //_AUTO_SCAN_ASCAN_H_
