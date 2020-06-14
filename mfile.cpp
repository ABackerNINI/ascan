#include <cassert>
#include "mfile.h"
#include "common.h"

#define OUT(...) fprintf(m_fout, __VA_ARGS__)
#define SEC(sec) g_sections[sec]
#define OUT_SEC(sec) \
    { fprintf(m_fout, "%s\n\n", g_sections[sec]); }
#define OUT_SEC2(sec, ...)                             \
    {                                                  \
        fprintf(m_fout, g_sections[sec], __VA_ARGS__); \
        OUT("\n\n");                                   \
    }

enum SECTIONS {
    SEC_HEADER_COMMENT = 0,
    SEC_BUILD_DETAILS,
    SEC_COMPILE_TO_OBJECTS,
    SEC_BUILD_EXECUTABLE,
    SEC_EXECUTABLE,
    SEC_DEPENDENCIES,
    SEC_CLEAN_UP
};

static const char *g_sections[] = {
    "# Auto generated by ascan, alpha version.\n"
    "# - ver : " AS_VERSION
    "\n"
    "# - date: %s\n"
    "# - url : " AS_URL,

    "# Build details",
    "# Compile to objects",
    "# Build Executable",
    "# Executable %d",
    "# Dependencies",
    "# Clean up"};

// static bool contain_flag_g(const char *c_cxx_flags);
static void find_all_headers(vector<cfile> &files, cfile *file);
static void print_all_headers(FILE *m_fout, vector<cfile> &files, cfile *file);

/*==========================================================================*/

mfile::mfile(const string &makefile, vector<cfile> &cfiles, Config &cfg,
             uint32_t flags)
    : m_makefile(makefile), m_cfiles(cfiles), m_cfg(cfg), m_flags(flags) {}

int mfile::output() {
#if DISABLE_WRITE == 1
    printf("disable_write enabled\n");
#else
    m_fout = fopen(m_makefile.c_str(), "w");
    if (m_fout == NULL) {
        print_error("Can't open file \"%s\", @%s line %d\n", m_makefile.c_str(),
                    __func__, __LINE__);
        return EXIT_FAILURE;
    }

    if (m_flags & FLAG_A) {
        output_header_comments();
        output_build_details_and_compile_to_objects();
        output_build_executable();
        output_dependencies();
        output_clean_up();
    } else {
        output_part();
    }
    fclose(m_fout);
#endif

    return EXIT_SUCCESS;
}

/*==========================================================================*/

void mfile::output_header_comments() {
    // char date[20];
    // get_date(date);
    // OUT_SEC2(SEC_HEADER_COMMENT, date);
}

// static bool contain_flag_g(const char *c_cxx_flags) {
//     const char *p = c_cxx_flags;

//     // only detects format: '-g', not '-O2g'
//     while (*p) {
//         if (*p == '-') {
//             if (*(p + 1) == 'g') {
//                 return true;
//             }
//         }
//         ++p;
//     }
//     return false;
// }

void mfile::output_build_details_and_compile_to_objects() {
    bool h_c = false, h_cpp = false, h_cc = false;
    for (auto it = m_cfiles.begin(); it != m_cfiles.end(); ++it) {
        if (it->file_type() == cfile::FILE_TYPE_C) {
            h_c = true;
        } else if (it->file_type() == cfile::FILE_TYPE_CPP) {
            h_cpp = true;
        } else if (it->file_type() == cfile::FILE_TYPE_CC) {
            h_cc = true;
        }
    }

    if (!h_c && !h_cpp && !h_cc) {
        h_c = h_cpp = h_cc = true;
    }

    OUT_SEC(SEC_BUILD_DETAILS);

    const char *flag_g = (m_flags & FLAG_G) ? " -g" : "";

    if (h_c) {
        OUT("%s = %s\n", m_cfg.k_cc.c_str(), m_cfg.v_cc.c_str());
    }
    if (h_cpp || h_cc) {
        OUT("%s = %s\n", m_cfg.k_cxx.c_str(), m_cfg.v_cxx.c_str());
    }
    if (h_c) {
        OUT("%s = %s%s\n", m_cfg.k_cflags.c_str(), m_cfg.v_cflag.c_str(),
            flag_g);
    }
    if (h_cpp || h_cc) {
        OUT("%s = %s%s\n", m_cfg.k_cxxflags.c_str(), m_cfg.v_cxxflag.c_str(),
            flag_g);
    }
    if (m_flags & FLAG_B) {
        OUT("%s = %s\n", m_cfg.k_bd.c_str(), m_cfg.v_bd.c_str());
    }
    OUT("\n");

    OUT_SEC(SEC_COMPILE_TO_OBJECTS);

    if (h_c) {
        if (m_flags & FLAG_B) {
            OUT("$(%s)/", m_cfg.k_bd.c_str());
        }
        OUT("%%.o: %%.c\n");
        OUT("\t$(%s) -c -o $@ $< $(%s)\n\n", m_cfg.k_cc.c_str(),
            m_cfg.k_cflags.c_str());
    }
    if (h_cpp) {
        if (m_flags & FLAG_B) {
            OUT("$(%s)/", m_cfg.k_bd.c_str());
        }
        OUT("%%.o: %%.cpp\n");
        OUT("\t$(%s) -c -o $@ $< $(%s)\n\n", m_cfg.k_cxx.c_str(),
            m_cfg.k_cxxflags.c_str());
    }
    if (h_cc) {
        if (m_flags & FLAG_B) {
            OUT("$(%s)/", m_cfg.k_bd.c_str());
        }
        OUT("%%.o: %%.cc\n");
        OUT("\t$(%s) -c -o $@ $< $(%s)\n\n", m_cfg.k_cxx.c_str(),
            m_cfg.k_cxxflags.c_str());
    }
}

static void find_all_headers(vector<cfile> &files, cfile *file) {
    file->set_visited(true);
    for (auto include = file->includes().begin();
         include != file->includes().end(); ++include) {
        if (!(*include)->visited()) {
            find_all_headers(files, *include);
            if ((*include)->associate() &&
                !(*include)->associate()->visited()) {
                find_all_headers(files, (*include)->associate());
            }
        }
    }
}

void mfile::output_build_executable() {
    // Find all executable
    for (auto cfile = m_cfiles.begin(); cfile != m_cfiles.end(); ++cfile) {
        if (cfile->have_main_func() && cfile->is_source()) {
            m_executable.push_back(&(*cfile));
        }
    }

    OUT_SEC(SEC_BUILD_EXECUTABLE);

    // Print bin1 var before all
    int i = m_executable.size() == 1 ? -1 : 1;
    for (auto &exec : m_executable) {
        // OUT: bin1 = ascan
        OUT("%s = %s\n", m_cfg.make_bin(i).c_str(), exec->name().c_str());
        ++i;
    }
    OUT("\n");

    // Print all
    OUT(".PHONY: all\n");
    OUT("all:");
    i = m_executable.size() == 1 ? -1 : 1;
    for (auto exec = m_executable.begin(); exec != m_executable.end(); ++exec) {
        OUT(" $(%s)", m_cfg.make_bin(i).c_str());
        ++i;
    }
    OUT("\n\n");

    // Print prepare
    if (m_flags & FLAG_B) {
        OUT(".PHONY: prepare\n");
        OUT("prepare:\n");
        OUT("\t$(if $(wildcard $(%s)),,mkdir -p $(%s))\n\n", m_cfg.k_bd.c_str(),
            m_cfg.k_bd.c_str());
    }

    // Print rebuild
    OUT(".PHONY: rebuild\nrebuild: clean all\n\n");

    // Print executable
    // if only one executable, hide the index number
    // TODO:add new flag to disable "hide the index number"
    i = m_executable.size() == 1 ? -1 : 1;
    for (auto &exec : m_executable) {
        // OUT("# Executable %d\n", i);
        OUT_SEC2(SEC_EXECUTABLE, (i == -1 ? 1 : i));
        // OUT: obj1 = ascan.o
        OUT("%s = %s.o", m_cfg.make_obj(i).c_str(), exec->name().c_str());
        // OUT: all objects dependecy.
        find_all_headers(m_cfiles, exec);
        for (auto cfile = m_cfiles.begin(); cfile != m_cfiles.end(); ++cfile) {
            if (cfile->visited()) {
                if (cfile->associate() != NULL && cfile->is_source() &&
                    &(*cfile) != exec) {
                    OUT(" %s.o", cfile->associate()->name().c_str());
                }
                cfile->set_visited(false);
            }
        }
        OUT("\n");

        if (m_flags & FLAG_B) {
            // OUT: obj1_bd = $(obj1:%=$(BD)/%)
            OUT("%s = $(%s:%%=$(%s)/%%)\n", m_cfg.make_obj_bd(i).c_str(),
                m_cfg.make_obj(i).c_str(), m_cfg.k_bd.c_str());
        }
        OUT("\n");

        if (m_flags & FLAG_B) {
            // OUT: $(bin1): prepare $(obj1_bd)
            OUT("$(%s): prepare $(%s)\n", m_cfg.make_bin(i).c_str(),
                m_cfg.make_obj_bd(i).c_str());
        } else {
            // OUT: $(bin1): $(obj1)
            OUT("%s: $(%s)\n", exec->name().c_str(), m_cfg.make_obj(i).c_str());
        }
        //! CXXFLAGS may contain dynamic libs such as -lm, this should be
        //! put behind the objects which use them.
        if (exec->is_c_source()) {
            if (m_flags & FLAG_B) {
                // OUT: $(CC) -o $(bin1) $(obj1_bd) $(CXXFLAGS)
                OUT("\t$(%s) -o $(%s) $(%s) $(%s)\n", m_cfg.k_cc.c_str(),
                    m_cfg.make_bin(i).c_str(), m_cfg.make_obj_bd(i).c_str(),
                    m_cfg.k_cflags.c_str());
            } else {
                // OUT: $(CC) -o $(bin1) $(obj1) $(CXXFLAGS)
                OUT("\t$(%s) -o $(%s) $(%s) $(%s)\n", m_cfg.k_cc.c_str(),
                    m_cfg.make_bin(i).c_str(), m_cfg.make_obj(i).c_str(),
                    m_cfg.k_cflags.c_str());
            }
        } else if (exec->is_cxx_source()) {
            if (m_flags & FLAG_B) {
                // OUT: $(CXX) -o $(bin1) $(obj1_bd) $(CXXFLAGS)
                OUT("\t$(%s) -o $(%s) $(%s) $(%s)\n", m_cfg.k_cxx.c_str(),
                    m_cfg.make_bin(i).c_str(), m_cfg.make_obj_bd(i).c_str(),
                    m_cfg.k_cxxflags.c_str());
            } else {
                // OUT: $(CXX) -o $(bin1) $(obj1) $(CXXFLAGS)
                OUT("\t$(%s) -o $(%s) $(%s) $(%s)\n", m_cfg.k_cxx.c_str(),
                    m_cfg.make_bin(i).c_str(), m_cfg.make_obj(i).c_str(),
                    m_cfg.k_cxxflags.c_str());
            }
        } else {
            // TODO error handle
            assert(0);
        }
        OUT("\n");

        ++i;
    }
}

static void print_all_headers(FILE *m_fout, vector<cfile> &files, cfile *file) {
    file->set_visited(true);
    for (auto include = file->includes().begin();
         include != file->includes().end(); ++include) {
        if (!(*include)->visited()) {
            OUT(" %s.h", (*include)->name().c_str());
            print_all_headers(m_fout, files, *include);
        }
    }
}

void mfile::output_dependencies_helper(FILE *m_fout, vector<cfile> &files,
                                       cfile *file) {
    file->set_visited(true);
    if (file->includes().size() > 0) {
        // .c/.cpp/.cc depends on all headers it includes, recursively

        if (m_flags & FLAG_B) {
            OUT("$(%s)/%s.o:", m_cfg.k_bd.c_str(), file->name().c_str());
        } else {
            OUT("%s.o:", file->name().c_str());
        }
        print_all_headers(m_fout, files, file);
        OUT("\n");

        for (auto include = file->includes().begin();
             include != file->includes().end(); ++include) {
            if (!(*include)->visited()) {
                output_dependencies_helper(m_fout, files, *include);
            }
        }
    } else {
        // .c/.cpp/.cc depends on itself if it has no includes
        OUT("%s.o: %s%s\n", file->name().c_str(), file->name().c_str(),
            file->ext());
    }
}

void mfile::output_dependencies() {
    OUT_SEC(SEC_DEPENDENCIES);

    for (auto cfile = m_cfiles.begin(); cfile != m_cfiles.end(); ++cfile) {
        if (cfile->is_source()) {
            output_dependencies_helper(m_fout, m_cfiles, &(*cfile));
        }
        for (auto cfile2 = m_cfiles.begin(); cfile2 != m_cfiles.end();
             ++cfile2) {
            if (cfile2->is_header()) {
                cfile2->set_visited(false);
            }
        }
    }
    OUT("\n");
}

void mfile::output_clean_up() {
    OUT_SEC(SEC_CLEAN_UP);

    OUT(".PHONY: clean\n");
    OUT("clean:\n");

    // OUT: rm -f "$(bin1)" $(obj1)
    OUT("\trm -f");
    // if only one executable, hide the index number
    int idx = m_executable.size() == 1 ? -1 : 1;
    if (m_flags & FLAG_B) {
        for (size_t i = 0; i < m_executable.size(); ++i) {
            OUT(" \"$(%s)\" $(%s)", m_cfg.make_bin(idx).c_str(),
                m_cfg.make_obj_bd(idx).c_str());
            ++idx;
        }
        OUT("\n");
        OUT("\trm -fd \"$(%s)\"\n", m_cfg.k_bd.c_str());
    } else {
        for (size_t i = 0; i < m_executable.size(); ++i) {
            OUT(" \"$(%s)\" $(%s)", m_cfg.make_bin(idx).c_str(),
                m_cfg.make_obj(idx).c_str());
            ++idx;
        }
        OUT("\n");
    }
}

void mfile::output_part() {
    char buff[BUFSIZ];

    while (fgets(buff, BUFSIZ, m_fout)) {
    }
}

/*==========================================================================*/