#ifndef _AUTO_SCAN_ASCAN_H_
#define _AUTO_SCAN_ASCAN_H_

#include <iostream>
#include <string>
#include <vector>

#include "cfile.h"
#include "config.h"
#include "options.h"

class ascan {
  public:
    ascan(int argc, char **argv);
    // Start to proceed auto-scan.
    //
    // Return:
    // -- EXIT_SUCCESS: if no error occurred.
    // -- EXIT_FAILURE: otherwise.
    int start();

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
    void print_help(enum HELP_TYPE help,
                    const options::as_option *option) const;

    // Check if there is one makefile, prompt overwrite when -f is not set.
    //
    // Return:
    // -- true: if -f is set or user entered 'yes' to overwrite or no makefile
    // found.
    // -- false: otherwise.
    bool test_makefile();
    void match_c_cxx_includes();
    void associate_header();

  private:
    int m_error;      // whether there is an error during parsing args
    uint32_t m_flags; // contains all options
    Config m_cfg;     // config
    // string m_cwd;            // current working dir
    std::vector<cfile> m_cfiles; // cfiles
    options m_options;
};

int main(int argc, char **argv);

#endif //_AUTO_SCAN_ASCAN_H_
