#include "mfile.h"

#include "debug.h"
#include <cassert>
#include <fstream>
#include <unistd.h>

int MFile::output() {
#ifdef DISABLE_WRITE
    print_warning("DISABLE_WRITE enabled\n");

    return EXIT_FAILURE;
#else

    // First write output to the temporary file, then rename it to the actual
    // output file in case exiting on error during output stage.
    std::string tmp = "ascan_tmp.mf";

    std::ofstream fout;

    fout.open(tmp, std::ios::out | std::ios::trunc);
    if (!fout.is_open()) {
        print_error("Can't open file \"%s\"\n", tmp.c_str());
        return EXIT_FAILURE;
    }

    if (this->build() != 0) {
        return EXIT_FAILURE;
    }

    fout << this->to_string();

    fout.close();

    std::string cmd = "mv \"" + tmp + "\" \"" + settings.option_output_ + "\"";
    print_debug("%s\n", cmd.c_str());
    if (system(cmd.c_str()) != 0) {
        print_error("unkown error!");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
#endif
}
