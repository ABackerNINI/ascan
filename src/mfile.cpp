#include "mfile.h"
#include "options.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <fstream>
#include <unistd.h>

using namespace std;

/*==========================================================================*/

MFile::MFile(std::vector<cfile> &cfiles, Config &cfg, uint32_t flags) : m_cfiles(cfiles), m_cfg(cfg), m_flags(flags) {}

int MFile::output() {
#ifdef DISABLE_WRITE
    print_warning("DISABLE_WRITE enabled\n");

    return EXIT_FAILURE;
#else

    // First write output to the temporary file, then rename it to the actual
    // output file in case exiting on error during output stage.
    string tmp = "ascan_tmp.mf";

    m_fout.open(tmp, ios::out | ios::trunc);
    if (!m_fout.is_open()) {
        print_error("Can't open file \"%s\"\n", m_cfg.output.c_str());
        return EXIT_FAILURE;
    }

    if (m_flags & OPTION_A) {
        prepare();

        // output_header_comments();
        output_build_details();
        output_targets();
        output_phony();
        output_clean_up();
        output_executable_details();
        output_compile_to_objects();
        output_mode_control();
        output_mm_dependencies();

        output_gitignore();

        m_fout << to_string();
    } else {
        // TODO:
        output_part();
    }

    m_fout.close();

    string cmd = "mv \"" + tmp + "\" \"" + m_cfg.output + "\"";
    print_debug("%s\n", cmd.c_str());
    if (system(cmd.c_str()) != 0) {
        print_error("unkown error!");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
#endif
}

/*==========================================================================*/

void MFile::prepare() {
    // Set m_c, m_cpp, m_cc flags
    m_c = m_cc = m_cpp = false;
    for (auto &cfile : m_cfiles) {
        if (cfile.file_type() == cfile::FILE_TYPE_C) {
            m_c = true;
        } else if (cfile.file_type() == cfile::FILE_TYPE_CPP) {
            m_cpp = true;
        } else if (cfile.file_type() == cfile::FILE_TYPE_CC) {
            m_cc = true;
        }
    }

    // If no source files, then compile all source files
    if (!m_c && !m_cpp && !m_cc) {
        m_c = m_cpp = m_cc = true;
    }

    // Set m_build_path
    if (m_flags & OPTION_B) {
        m_build_path = MSimpleVariable(CONFIG_BD).to_string() + "/";
    }

    // Find all executable
    for (auto &cfile : m_cfiles) {
        if (cfile.have_main_func() && cfile.is_source()) {
            m_executable.push_back(&cfile);
        }
    }

    // Sort executables by name
    sort(m_executable.begin(), m_executable.end(),
         [](const cfile *a, const cfile *b) { return a->name() < b->name(); });
}

void MFile::output_build_details() {
    const char *flag_g = (m_flags & OPTION_G) ? " -g" : "";

    add_component(new MComment("BUILD DETAILS"));
    add_component(new MBlankLine());

    Align align;

    if (m_c) {
        align.add(CONFIG_CC);
        align.add(CONFIG_CFLAGS);
    }

    if (m_cpp || m_cc) {
        align.add(CONFIG_CXX);
        align.add(CONFIG_CXXFLAGS);
    }

    align.add(CONFIG_LDFLAGS);
    align.add(CONFIG_BD);

    auto newSpVarDef = [&align](const string &name, const string &value) {
        return new MSimpleVariableDef(align(name).to_string(), value);
    };

    // OUT: CC = gcc
    if (m_c) {
        add_component(newSpVarDef(CONFIG_CC, m_cfg.get(CONFIG_CC)));
    }

    // OUT: CXX = g++
    if (m_cpp || m_cc) {
        add_component(newSpVarDef(CONFIG_CXX, m_cfg.get(CONFIG_CXX)));
    }

    // OUT: CFLAGS = -W -Wall -lm -g
    if (m_c) {
        add_component(newSpVarDef(CONFIG_CFLAGS, m_cfg.get(CONFIG_CFLAGS) + flag_g));
    }

    // OUT: CXXFLAGS = -W -Wall -g
    if (m_cpp || m_cc) {
        add_component(newSpVarDef(CONFIG_CXXFLAGS, m_cfg.get(CONFIG_CXXFLAGS) + flag_g));
    }

    // OUT: LDFLAGS = -lm
    add_component(newSpVarDef(CONFIG_LDFLAGS, m_cfg.get(CONFIG_LDFLAGS)));

    // OUT: BUILD = build
    if (m_flags & OPTION_B) {
        add_component(newSpVarDef(CONFIG_BD, m_cfg.get(CONFIG_BD)));
    }

    add_component(new MBlankLine());
}

void MFile::output_targets() {
    add_component(new MComment("TARGETS"));
    add_component(new MBlankLine());

    // OUT: TARGET1 = xxx
    // OUT: TARGET2 = xxx
    int idx = m_executable.size() == 1 ? -1 : 1;
    for (auto &exec : m_executable) {
        MVariableDef *target = new MVariableDef(m_cfg.make_bin(idx++));
        target->add_sub_component(new MFilename(exec->name()));
        add_component(target);
    }
    add_component(new MBlankLine());

    /*
        OUT:
            default: debug
     */
    MRule *default_rule = new MRule("default");
    default_rule->add_dependency(new MComponent("debug"));
    add_component(default_rule);
    add_component(new MBlankLine());

    /*
        OUT:
            debug: CXXFLAGS += -g -DDEBUG=1
            debug: $(BUILD)/debug.mode $(TARGET1) $(TARGET2)
     */
    if (m_c) {
        MRule *debug_rule = new MRule("debug");
        debug_rule->add_dependency(new MComponent(string(CONFIG_CFLAGS) + " += -g -DDEBUG=1")); // TODO
        add_component(debug_rule);
    }
    if (m_cpp || m_cc) {
        MRule *debug_rule = new MRule("debug");
        debug_rule->add_dependency(new MComponent(string(CONFIG_CXXFLAGS) + " += -g -DDEBUG=1")); // TODO
        add_component(debug_rule);
    }
    MRule *debug_rule = new MRule("debug");
    debug_rule->add_dependency(new MFilename(m_build_path + "debug.mode"));

    idx = m_executable.size() == 1 ? -1 : 1;
    for (size_t i = 0; i < m_executable.size(); ++i) {
        debug_rule->add_dependency(new MSimpleVariable(m_cfg.make_bin(idx++)));
    }
    add_component(debug_rule);
    add_component(new MBlankLine());

    /*
        OUT:
            release: CXXFLAGS += -O3 # -DNDEBUG=1
            release: $(BUILD)/release.mode $(TARGET1) $(TARGET2)
     */
    if (m_c) {
        MRule *release_rule = new MRule("release");
        release_rule->add_dependency(new MComponent(string(CONFIG_CFLAGS) + " += -O3 # -DNDEBUG=1")); // TODO
        add_component(release_rule);
    }
    if (m_cpp || m_cc) {
        MRule *release_rule = new MRule("release");
        release_rule->add_dependency(new MComponent(string(CONFIG_CXXFLAGS) + " += -O3 # -DNDEBUG=1")); // TODO
        add_component(release_rule);
    }
    MRule *release_rule = new MRule("release");
    release_rule->add_dependency(new MFilename(m_build_path + "release.mode"));
    idx = m_executable.size() == 1 ? -1 : 1;
    for (size_t i = 0; i < m_executable.size(); ++i) {
        release_rule->add_dependency(new MSimpleVariable(m_cfg.make_bin(idx++)));
    }
    add_component(release_rule);
    add_component(new MBlankLine());
}

static void find_all_headers(vector<cfile> &files, cfile *file) {
    file->set_visited(true);
    for (auto include = file->includes().begin(); include != file->includes().end(); ++include) {
        if (!(*include)->visited()) {
            find_all_headers(files, *include);
            if ((*include)->associate() && !(*include)->associate()->visited()) {
                find_all_headers(files, (*include)->associate());
            }
        }
    }
}

void MFile::output_executable_details() {
    add_component(new MComment("EXECUTABLE DETAILS"));
    add_component(new MBlankLine());

    // Print executable
    // TODO: add new flag to disable "hide the index number"

    // OUT: OBJS1 = xxx.o
    //      OBJS2 = xxx.o
    int idx = m_executable.size() == 1 ? -1 : 1;
    for (auto &exec : m_executable) {
        // if only one executable, hide the index number

        // OUT: OBJS1 = xxx.o
        MVariableDef *obj = new MVariableDef(m_cfg.make_objs(idx++));
        obj->add_sub_component(new MFilename(exec->name() + ".o"));

        // OUT: all objects dependency.
        find_all_headers(m_cfiles, exec);
        for (auto cfile = m_cfiles.begin(); cfile != m_cfiles.end(); ++cfile) {
            if (cfile->visited()) {
                if (cfile->associate() != NULL && cfile->is_source() && &(*cfile) != exec) {
                    // OUT: xxx.o
                    obj->add_sub_component(new MFilename(cfile->associate()->name() + ".o"));
                }
                cfile->set_visited(false);
            }
        }

        add_component(obj);
    }

    add_component(new MBlankLine());

    // OUT: OBJS1BD = $(OBJS1:%=$(BUILD)/%)
    //      OBJS2BD = $(OBJS2:%=$(BUILD)/%)
    if (m_flags & OPTION_B) {
        idx = m_executable.size() == 1 ? -1 : 1;
        for (size_t i = 0; i < m_executable.size(); ++i) {
            // OUT: OBJS1BD = $(OBJS1:%=$(BUILD)/%)
            MVariableDef *obj_bd = new MVariableDef(m_cfg.make_objs_bd(idx));
            obj_bd->add_sub_component(new MSimpleVariable(m_cfg.make_objs(idx) + string(":%=$(") + CONFIG_BD + ")/%"));
            add_component(obj_bd);
            ++idx;
        }

        add_component(new MBlankLine());
    }

    // OUT: $(TARGET1): $(OBJS1BD)
    //          $(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
    //      $(TARGET2): $(OBJS2BD)
    //          $(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
    idx = m_executable.size() == 1 ? -1 : 1;
    for (auto &exec : m_executable) {
        MRule *rule = new MRule(MSimpleVariable(m_cfg.make_bin(idx)));
        if (m_flags & OPTION_B) {
            // OUT: $(TARGET1): $(OBJS1BD)
            rule->add_dependency(new MSimpleVariable(m_cfg.make_objs_bd(idx)));
        } else {
            // OUT: $(TARGET1): $(OBJS1)
            rule->add_dependency(new MSimpleVariable(m_cfg.make_objs(idx)));
        }

        //! CXXFLAGS may contain dynamic libs such as -lm, this should be
        //! put behind the objects which uses them.
        if (exec->is_c_source()) {
            // OUT: $(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
            MCommand *cmd = new MCommand();
            *cmd << MSimpleVariable(CONFIG_CC) << MSimpleVariable(CONFIG_CFLAGS) << "-o $@ $^"
                 << MSimpleVariable(CONFIG_LDFLAGS);
            rule->add_command(cmd);
        } else if (exec->is_cxx_source()) {
            // OUT: $(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
            MCommand *cmd = new MCommand();
            *cmd << MSimpleVariable(CONFIG_CXX) << MSimpleVariable(CONFIG_CXXFLAGS) << "-o $@ $^"
                 << MSimpleVariable(CONFIG_LDFLAGS);
            rule->add_command(cmd);
        } else {
            // TODO error handle
            assert(0);
        }

        add_component(rule);
        add_component(new MBlankLine());

        ++idx;
    }
}

void MFile::output_compile_to_objects() {
    add_component(new MComment("COMPILE TO OBJECTS"));
    add_component(new MBlankLine());

    if (m_c) {
        // OUT: $(BUILD)/%.o: %.c
        // OUT:     @mkdir -p "$(BUILD)"
        // OUT:     $(CC) $(CFLAGS) -c -o $@ $<
        MRule *rule = new MRule(MFilename(m_build_path + "%.o"));
        rule->add_dependency(new MFilename("%.c"));
        add_mkdir_build_cmd_if_option_b(rule);

        MCommand *cmd = new MCommand();
        *cmd << MSimpleVariable(CONFIG_CC) << MSimpleVariable(CONFIG_CFLAGS) << "-c -o $@ $<";
        rule->add_command(cmd);

        add_component(rule);
    }
    if (m_cpp) {
        // OUT: $(BUILD)/%.o: %.cpp
        // OUT:     @mkdir -p "$(BUILD)"
        // OUT:     $(CXX) $(CXXFLAGS) -c -o $@ $<
        MRule *rule = new MRule(MFilename(m_build_path + "%.o"));
        rule->add_dependency(new MFilename("%.cpp"));
        add_mkdir_build_cmd_if_option_b(rule);

        MCommand *cmd = new MCommand();
        *cmd << MSimpleVariable(CONFIG_CXX) << MSimpleVariable(CONFIG_CXXFLAGS) << "-c -o $@ $<";
        rule->add_command(cmd);

        add_component(rule);
    }
    if (m_cc) {
        // OUT: $(BUILD)/%.o: %.cc
        // OUT:     @mkdir -p "$(BUILD)"
        // OUT:     $(CXX) $(CXXFLAGS) -c -o $@ $<
        MRule *rule = new MRule(MFilename(m_build_path + "%.o"));
        rule->add_dependency(new MFilename("%.cc"));
        add_mkdir_build_cmd_if_option_b(rule);

        MCommand *cmd = new MCommand();
        *cmd << MSimpleVariable(CONFIG_CXX) << MSimpleVariable(CONFIG_CXXFLAGS) << "-c -o $@ $<";
        rule->add_command(cmd);

        add_component(rule);
    }

    add_component(new MBlankLine());
}

void MFile::output_mode_control() {
    // OUT:
    //    # MODE CONTROL
    //
    //    $(BUILD)/%.mode:
    //        @if [ ! -f "$@" ]; then
    //            if [ -d "$(BUILD)" ]; then
    //                echo "Switching to $* mode...";
    //                $(MAKE) clean;
    //            fi;
    //            mkdir -p "$(BUILD)";
    //            touch "$@";
    //        fi

    add_component(new MComment("MODE CONTROL"));
    add_component(new MBlankLine());

    MRule *rule = new MRule(MFilename(m_build_path + "%.mode"));

    MCommand *cmd = new MCommand();
    cmd->set_separator("");
    cmd->set_prefix(M_COMMAND_PREFIX_ECHO_OFF);

    *cmd << "if [ ! -f \"$@\" ]; then \\\n";

    if (m_flags & OPTION_B) {
        // If build directory exists, then switch to another mode.
        // If not, then "make clean" is not needed.
        *cmd << "\t\tif [ -d \"" << MSimpleVariable(CONFIG_BD) << "\" ]; then \\\n";
        *cmd << "\t\t\techo \"Switching to $* mode...\"; \\\n";
        *cmd << "\t\t\t" << MSimpleVariable(CONFIG_MAKE) << " clean; \\\n";
        *cmd << "\t\tfi; \\\n";
        *cmd << "\t\tmkdir -p \"" << MSimpleVariable(CONFIG_BD) << "\"; \\\n";
    } else {
        // "make clean" anyway
        *cmd << "\t\t" << MSimpleVariable(CONFIG_MAKE) << " clean; \\\n";
    }

    *cmd << "\t\ttouch \"$@\"; \\\n";
    *cmd << "\tfi\n";

    rule->add_command(cmd);

    add_component(rule);
}

void MFile::output_clean_up() {
    MRule *clean_rule = new MRule("clean");

    // if only one executable, hide the index number
    if (m_flags & OPTION_B) {
        // OUT: rm -f "$(TARGET1)" "$(TARGET2)" $(BUILD)

        MCommand *cmd = new MCommand();
        *cmd << "rm -rf";

        int idx = m_executable.size() == 1 ? -1 : 1;
        for (size_t i = 0; i < m_executable.size(); ++i) {
            // OUT: "$(TARGET1)"
            *cmd << MQuoted(MSimpleVariable(m_cfg.make_bin(idx++)));
        }

        // OUT: $(BUILD)
        *cmd << MQuoted(MSimpleVariable(CONFIG_BD));

        clean_rule->add_command(cmd);
    } else {
        //? Why not "rm -f *.o"?
        //* Because sometimes we may compile with other pre-compiled .o files
        //* This is not a problem if option -b.

        // OUT rm -f depend.mk "$(TARGET1)" "$(TARGET2)" $(OBJ1) $(OBJ2)

        // OUT("\trm -f");
        MCommand *cmd = new MCommand();
        *cmd << "rm -f";

        // OUT: depend.mk
        *cmd << MQuoted(m_build_path + CONFIG_DEPENDENCIES_FILENAME);

        int idx = m_executable.size() == 1 ? -1 : 1;
        for (size_t i = 0; i < m_executable.size(); ++i) {
            // OUT: "$(TARGET1)"
            *cmd << MQuoted(MSimpleVariable(m_cfg.make_bin(idx++)));
        }
        idx = m_executable.size() == 1 ? -1 : 1;
        for (size_t i = 0; i < m_executable.size(); ++i) {
            // OUT: $(OBJS1)
            *cmd << MSimpleVariable(m_cfg.make_objs(idx++));
        }
    }

    add_component(clean_rule);
    add_component(new MBlankLine());
}

void MFile::output_phony() {
    // OUT: .PHONY: default debug release clean
    MRule *rule = new MRule(".PHONY");
    rule->add_dependency(new MComponent("default"));
    rule->add_dependency(new MComponent("debug"));
    rule->add_dependency(new MComponent("release"));
    rule->add_dependency(new MComponent("clean"));

    add_component(rule);
    add_component(new MBlankLine());
}

void MFile::output_mm_dependencies() {
    string c_types, cxx_types;

    if (m_c) {
        c_types += " *.c";
    }

    if (m_cc) {
        cxx_types += " *.cc";
    }

    if (m_cpp) {
        cxx_types += " *.cpp";
    }

    add_component(new MComment("DEPENDENCIES"));
    add_component(new MBlankLine());

    // OUT: SRC = $(wildcard *.h *.hpp *.c *.cpp *.cc)
    add_component(new MSimpleVariableDef("SRC", "$(wildcard *.h *.hpp *.c *.cpp *.cc)"));
    add_component(new MBlankLine());

    // OUT: $(BUILD)/depend.mk: $(SRC)
    MRule *depend_rule = new MRule(MFilename(m_build_path + CONFIG_DEPENDENCIES_FILENAME));
    depend_rule->add_dependency(new MSimpleVariable("SRC"));

    add_mkdir_build_cmd_if_option_b(depend_rule);

    if (m_flags & OPTION_B) {
        if (m_c) {
            // OUT: @$(CC) $(CFLAGS) -MM *.c $(LDFLAGS) | sed 's/^\\(.*\\).o:/$$(BUILD)\/\1.o:/' > $@
            MCommand *cmd = new MCommand(M_COMMAND_PREFIX_ECHO_OFF);
            cmd->set_separator("");
            *cmd << MSimpleVariable(CONFIG_CC) << " " << MSimpleVariable(CONFIG_CFLAGS) << " -MM" << c_types << " "
                 << MSimpleVariable(CONFIG_LDFLAGS) << " | sed 's/^\\(.*\\).o:/$" << MSimpleVariable(CONFIG_BD)
                 << "\\/\\1.o:/' > $@"; // TODO
            depend_rule->add_command(cmd);
        }

        if (m_cc || m_cpp) {
            // OUT: @$(CXX) $(CXXFLAGS) -MM *.cpp *.cc $(LDFLAGS) | sed 's/^\\(.*\\).o:/$$(BUILD)\/\1.o:/' > $@
            MCommand *cmd = new MCommand(M_COMMAND_PREFIX_ECHO_OFF);
            cmd->set_separator("");
            *cmd << MSimpleVariable(CONFIG_CXX) << " " << MSimpleVariable(CONFIG_CXXFLAGS) << " -MM" << cxx_types << " "
                 << MSimpleVariable(CONFIG_LDFLAGS) << " | sed 's/^\\(.*\\).o:/$" << MSimpleVariable(CONFIG_BD)
                 << "\\/\\1.o:/' > $@"; // TODO
            depend_rule->add_command(cmd);
        }
    } else {
        if (m_c) {
            // OUT: @$(CC) $(CFLAGS) -MM *.c $(LDFLAGS) > $@
            MCommand *cmd = new MCommand(M_COMMAND_PREFIX_ECHO_OFF);
            cmd->set_separator("");
            *cmd << MSimpleVariable(CONFIG_CC) << " " << MSimpleVariable(CONFIG_CFLAGS) << " -MM" << c_types << " "
                 << MSimpleVariable(CONFIG_LDFLAGS) << " > $@"; // TODO
            depend_rule->add_command(cmd);
        }
        if (m_cc || m_cpp) {
            // OUT: @$(CXX) $(CXXFLAGS) -MM *.cpp *.cc $(LDFLAGS) > $@
            MCommand *cmd = new MCommand(M_COMMAND_PREFIX_ECHO_OFF);
            cmd->set_separator("");
            *cmd << MSimpleVariable(CONFIG_CXX) << " " << MSimpleVariable(CONFIG_CXXFLAGS) << " -MM" << cxx_types << " "
                 << MSimpleVariable(CONFIG_LDFLAGS) << " > $@"; // TODO
            depend_rule->add_command(cmd);
        }
    }
    add_component(depend_rule);
    add_component(new MBlankLine());

    // OUT: include $(BUILD)/depend.mk
    MCompComponent *comp = new MCompComponent();
    comp->set_separator(" ");
    *comp << "include" << MFilename(m_build_path + CONFIG_DEPENDENCIES_FILENAME); // TODO
    add_component(comp);
}

/*==========================================================================*/

void MFile::output_part() {}

void MFile::output_gitignore() {
    /*
    if (m_flags & OPTION_B) {
        m_binaries.push_back("build/");
    } else {
        m_binaries.push_back("*.o");
    }

    // TODO
    vector<string> ignores;
    read_file_by_line(ignores, ".gitignore");

    size_t n = ignores.size();
    for (size_t i = 0; i < n; ++i) {
        if (ignores[i].length() && ignores[i].back() == '/') {
            ignores.push_back(ignores[i].substr(0, ignores[i].length()));
        }
    }

    for (auto &binary : m_binaries) {
        for (auto &ignore : ignores) {
            if (binary == ignore) {
                binary = "";
                break;
            }
        }
    }

    append_file_by_line(".gitignore", m_binaries);

    append_file_by_line("", m_binaries);
    */
}

/*==========================================================================*/

void MFile::add_mkdir_build_cmd_if_option_b(MRule *rule) {
    // Character before command:
    //   '@': turn off echo.
    //   '-': ignore error, make will exit when error occurs.
    //   '+': ignore make's -n -t -q options.
    if (m_flags & OPTION_B) {
        // OUT @mkdir -p "$(BUILD)"
        MCommand *cmd = new MCommand();
        cmd->set_prefix(M_COMMAND_PREFIX_ECHO_OFF);
        *cmd << "mkdir -p" << MQuoted(MSimpleVariable(CONFIG_BD));
        rule->add_command(cmd);
    }
}
