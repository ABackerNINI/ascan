#include "mfile.h"
#include <cassert>
#include <unistd.h>

using namespace std;

int MFile::output() {
#ifdef DISABLE_WRITE
    print_warning("DISABLE_WRITE enabled\n");

    return EXIT_FAILURE;
#else

    // First write output to the temporary file, then rename it to the actual
    // output file in case exiting on error during output stage.
    string tmp = "ascan_tmp.mf";

    std::ofstream fout;

    fout.open(tmp, ios::out | ios::trunc);
    if (!fout.is_open()) {
        print_error("Can't open file \"%s\"\n", m_cfg.output.c_str());
        return EXIT_FAILURE;
    }

    this->build();

    fout << m_components.to_string();

    fout.close();

    string cmd = "mv \"" + tmp + "\" \"" + m_cfg.output + "\"";
    print_debug("%s\n", cmd.c_str());
    if (system(cmd.c_str()) != 0) {
        print_error("unkown error!");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
#endif
}
