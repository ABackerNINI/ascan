#include "mfilev4.h"

#include "debug.h"
#include "utils.h"
#include <algorithm>
#include <cassert>
#include <unistd.h>

class safe_replacer {
  private:
    struct pos {
        size_t p;
        size_t index;
    };

  public:
    static size_t replace(std::string &str, const std::vector<std::pair<std::string, std::string>> &replaces) {
        // Find all occurrences of the strings to be replaced and store their positions along with their indices in the
        // replaces vector
        std::vector<pos> positions;
        for (size_t i = 0; i < replaces.size(); i++) {
            size_t p = str.find(replaces[i].first);
            while (p != std::string::npos) {
                positions.push_back({p, i});
                p = str.find(replaces[i].first, p + replaces[i].first.size());
            }
        }

        // If no occurrences were found, return 0
        if (positions.empty()) {
            return 0;
        }

        // Sort positions by position in reverse order so that replacements do not interfere with each other
        std::sort(positions.begin(), positions.end(), [](const pos &a, const pos &b) { return a.p > b.p; });

        // Replace all occurrences in reverse order
        for (auto &p : positions) {
            str.replace(p.p, replaces[p.index].first.size(), replaces[p.index].second);
        }

        // Return the number of replacements made
        return positions.size();
    }
};

int MFileV4::build() {
    // Import settings from CLI parser
    // * Why not use the Settings class directly?
    // * Because each template version may have different default values for the settings.
    import_settings();

    prepare();

    build_options_section();
    build_obj_dir_config_file_cc_cxx_std();
    build_c_cxx_flags();
    build_c_cxx_flags_amend();
    build_targets();
    build_sources_section();
    build_objects_section();
    build_targets_section();

    fs::path tempfile = "templates/v4/template.mk";
    std::string temp = read_file(tempfile);

    std::vector<std::pair<std::string, std::string>> replaces;
    replaces.push_back({"__ASCAN::PROJECT_NAME__", t.proj_name});
    replaces.push_back({"__ASCAN::OPTIONS_SECTION__", t.options_section.to_string()});
    replaces.push_back({"__ASCAN::SRC_DIR__", t.src_dir});
    replaces.push_back({"__ASCAN::BLD_DIR__", t.bld_dir});
    replaces.push_back({"__ASCAN::BIN_DIR__", t.bin_dir});
    replaces.push_back({"__ASCAN::OBJ_DIR_CC_CXX_STD__", t.obj_dir_config_file_cc_cxx_std.to_string()});
    replaces.push_back({"__ASCAN::CONFIG_FILE_CC_CXX_STD__", t.obj_dir_config_file_cc_cxx_std.to_string()});
    replaces.push_back({"__ASCAN::C_CXX_FLAGS__", t.c_cxx_flags.to_string()});
    replaces.push_back({"__ASCAN::TARGETS__", t.targets.to_string()});
    replaces.push_back({"__ASCAN::C_CXX_FLAGS_DEBUG_AMEND__", t.c_cxx_flags_debug_amend.to_string()});
    replaces.push_back({"__ASCAN::C_CXX_FLAGS_RELEASE_AMEND__", t.c_cxx_flags_release_amend.to_string()});
    replaces.push_back({"__ASCAN::LD_FLAGS__", t.ldflags});
    replaces.push_back({"__ASCAN::SOURCES_SECTION__", t.sources_section.to_string()});
    replaces.push_back({"__ASCAN::OBJECTS_SECTION__", t.objects_section.to_string()});
    replaces.push_back({"__ASCAN::TARGETS_SECTION__", t.targets_section.to_string()});

    size_t num_replaced = safe_replacer::replace(temp, replaces);

    if (num_replaced != replaces.size()) {
        print_warning("Failed to replace all strings in template\n");
    }

    if (temp.find("__ASCAN::") != std::string::npos) {
        print_error("There are still __ASCAN:: strings in the template\n");
        return 2;
    }

    m_result = temp;

    return 0;
}

void MFileV4::import_settings() {
    // Project name is the directory name of the project root
    fs::path cwd = fs::current_path();
    t.proj_name = cwd.filename().string();

    import_setting(t.proj_name, settings.option_proj_name_);
    import_setting(t.config, settings.option_default_config_);
    import_setting(t.cc, settings.option_cc_);
    import_setting(t.cxx, settings.option_cxx_);
    import_setting(t.stdc, !settings.option_std_c_.empty() ? settings.option_std_c_ : settings.option_std_);
    import_setting(t.stdcxx, !settings.option_std_cxx_.empty() ? settings.option_std_cxx_ : settings.option_std_);
    import_setting(t.src_dir, settings.option_src_dir_);
    import_setting(t.bld_dir, settings.option_build_dir_);
    import_setting(t.bin_dir, settings.option_bin_dir_);
    import_setting(t.ldflags, vector_to_string(settings.option_ldflags_, " "));

    import_setting(m_c, settings.debug_flag_xc_);
    import_setting(m_cc, settings.debug_flag_xcc_);
    import_setting(m_cpp, settings.debug_flag_xcpp_);
}

void MFileV4::prepare() {
    // TODO: set m_c, m_cpp, m_cc flags based on only the source files used, not all source files found
    // Set m_c, m_cpp, m_cc flags
    for (auto &cfile : cfiles_) {
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

    // Find all executables
    if (!settings.main_files_.empty()) { // If main files are specified, then only compile those files
        for (auto &main_file : settings.main_files_) {
            fs::path main_file_path = fs::relative(main_file);
            for (auto &cfile : cfiles_) {
                if (cfile.is_source() && cfile.path() == main_file_path) {
                    if (!cfile.have_main_func()) {
                        print_warning("File '%s' seems to have no main function, but it is marked as main file\n",
                                      cfile.path().c_str());
                    }
                    m_executable.push_back(&cfile);
                    break;
                }
            }
        }
    } else { // Otherwise, find all files with main function
        for (auto &cfile : cfiles_) {
            if (cfile.have_main_func() && cfile.is_source()) {
                m_executable.push_back(&cfile);
            }
        }
        // Sort executables by name
        sort(m_executable.begin(), m_executable.end(),
             [](const cfile *a, const cfile *b) { return a->stem() < b->stem(); });
    }
}

MSimpleVariableDef *MFileV4::add_svardef(MCompComponent &mcc,
                                         const std::string &varname,
                                         const std::string &varval,
                                         VariableAssignmentType assignment_type) {
    MSimpleVariableDef *var = new MSimpleVariableDef(varname, varval, assignment_type);
    mcc.add_component(var);
    return var;
}

MSimpleVariable *MFileV4::add_svar(MCompComponent &mcc, const std::string &varname) {
    MSimpleVariable *var = new MSimpleVariable(varname);
    mcc.add_component(var);
    return var;
}

void MFileV4::build_options_section() {
    VariableAssignmentType cond_eq = VariableAssignmentType::CONDITIONAL;

    auto &target = t.options_section;

    add_svardef(target, "CONFIG", t.config, cond_eq);

    target.add_component(MBlankLine());
    if (m_c && (m_cc || m_cpp)) {
        add_svardef(target, "CC", t.cc, cond_eq);
        add_svardef(target, "CXX", t.cxx, cond_eq);
        add_svardef(target, "CSTD", t.stdc, cond_eq);
        add_svardef(target, "CXXSTD", t.stdcxx, cond_eq);
    } else if (m_c) {
        add_svardef(target, "CC", t.cc, cond_eq);
        add_svardef(target, "STD", t.stdc, cond_eq);
    } else {
        add_svardef(target, "CXX", t.cxx, cond_eq);
        add_svardef(target, "STD", t.stdcxx, cond_eq);
    }
}

void MFileV4::build_obj_dir_config_file_cc_cxx_std() {
    // $(CC).$(STD)
    // $(CXX).$(STD)
    // $(CC).$(CXX).$(CSTD).$(CXXSTD)

    auto &target = t.obj_dir_config_file_cc_cxx_std;

    if (m_c && (m_cc || m_cpp)) {
        add_svar(target, "CC");
        target.add_component(".");
        add_svar(target, "CXX");
        target.add_component(".");
        add_svar(target, "CSTD");
        target.add_component(".");
        add_svar(target, "CXXSTD");
    } else if (m_c) {
        add_svar(target, "CC");
        target.add_component(".");
        add_svar(target, "STD");
    } else {
        add_svar(target, "CXX");
        target.add_component(".");
        add_svar(target, "STD");
    }
}

void MFileV4::build_c_cxx_flags() {
    // CFLAGS = -Wall -Wextra -std=$(STD) -I. -I./libs
    // CXXFLAGS = -Wall -Wextra -std=$(STD) -I. -I./libs
    // CFLAGS = -Wall -Wextra -std=$(CSTD) -I. -I./libs
    // CXXFLAGS = -Wall -Wextra -std=$(CXXSTD) -I. -I./libs
    auto &target = t.c_cxx_flags;

    std::vector<std::string> cflags = default_flags;
    std::vector<std::string> cxxflags = default_flags;
    if (m_c && (m_cc || m_cpp)) {
        cflags.push_back("-std=$(CSTD)");
        for (auto &inc : settings.include_dirs_) {
            cflags.push_back("-I" + inc);
        }
        cxxflags.push_back("-std=$(CXXSTD)");
        for (auto &inc : settings.include_dirs_) {
            cxxflags.push_back("-I" + inc);
        }

        add_svardef(target, "CFLAGS", vector_to_string(cflags, " "));
        add_svardef(target, "CXXFLAGS", vector_to_string(cxxflags, " "));
    } else if (m_c) {
        cflags.push_back("-std=$(STD)");
        for (auto &inc : settings.include_dirs_) {
            cflags.push_back("-I" + inc);
        }
        add_svardef(target, "CFLAGS", vector_to_string(cflags, " "));
    } else {
        cxxflags.push_back("-std=$(STD)");
        for (auto &inc : settings.include_dirs_) {
            cxxflags.push_back("-I" + inc);
        }
        add_svardef(target, "CXXFLAGS", vector_to_string(cxxflags, " "));
    }
}

void MFileV4::build_targets() {
    // Only one executable:
    //   TARGET = $(BIN_DIR)/$(PROJECT) <-- use $(PROJECT) or stem of the executable?
    // Multiple executables:
    //   TARGET1 = $(BIN_DIR)/foo
    //   TARGET2 = $(BIN_DIR)/bar
    if (m_executable.size() <= 1) {
        MVariableDef target{"TARGET"};
        target.add_component(make_comp_component(MSimpleVariable("BIN_DIR"), "/", MSimpleVariable("PROJECT")));
        t.targets.add_component(std::move(target));
    } else if (m_executable.size() > 1) {
        int index = 1;
        for (auto &exec : m_executable) {
            MVariableDef target{"TARGET" + std::to_string(index)};
            target.add_component(make_comp_component(MSimpleVariable("BIN_DIR"), "/", exec->stem()));
            t.targets.add_component(std::move(target));
            index++;
        }
    }
}

void MFileV4::build_c_cxx_flags_amend() {
    // debug amend:
    //   CFLAGS += -g -O0 -DDEBUG
    //   CXXFLAGS += -g -O0 -DDEBUG
    // release amend:
    //   CFLAGS += -flto=4 -O3 -march=native -DNDEBUG
    //   CXXFLAGS += -flto=4 -O3 -march=native -DNDEBUG

    auto &debug_target = t.c_cxx_flags_debug_amend;
    auto &release_target = t.c_cxx_flags_release_amend;

    const std::string debug_amend = "-g -O0 -DDEBUG";
    const std::string release_amend = "-flto=4 -O3 -march=native -DNDEBUG";

    if (m_c && (m_cc || m_cpp)) {
        add_svardef(debug_target, "CFLAGS", debug_amend, VariableAssignmentType::APPEND);
        add_svardef(debug_target, "CXXFLAGS", debug_amend, VariableAssignmentType::APPEND)->indent(1);
        add_svardef(release_target, "CFLAGS", release_amend, VariableAssignmentType::APPEND);
        add_svardef(release_target, "CXXFLAGS", release_amend, VariableAssignmentType::APPEND)->indent(1);
    } else if (m_c) {
        add_svardef(debug_target, "CFLAGS", debug_amend, VariableAssignmentType::APPEND);
        add_svardef(release_target, "CFLAGS", release_amend, VariableAssignmentType::APPEND);
    } else {
        add_svardef(debug_target, "CXXFLAGS", debug_amend, VariableAssignmentType::APPEND);
        add_svardef(release_target, "CXXFLAGS", release_amend, VariableAssignmentType::APPEND);
    }
}

static void find_all_sources_and_headers(std::vector<cfile> &files, cfile *file) {
    file->set_visited(true);
    for (auto include = file->includes().begin(); include != file->includes().end(); ++include) {
        if (!(*include)->visited()) {
            find_all_sources_and_headers(files, *include);
            if ((*include)->associate() && !(*include)->associate()->visited()) {
                find_all_sources_and_headers(files, (*include)->associate());
            }
        }
    }
}

void MFileV4::build_sources_section() {
    int index = 1;
    for (auto &exec : m_executable) {
        // TODO: wildcard sources
        std::string index_str = (index == 1 && m_executable.size() == 1) ? "" : std::to_string(index);

        MVariableDef sources{"SRCS" + index_str};

        find_all_sources_and_headers(cfiles_, exec);

        std::vector<cfile *> source_files;
        for (cfile &cf : cfiles_) {
            if (cf.visited() && cf.is_source()) {
                if (&cf != exec) {
                    source_files.push_back(&cf);
                }
                cf.set_visited(false);
            }
        }

        std::sort(source_files.begin(), source_files.end(), [](cfile *a, cfile *b) { return a->path() < b->path(); });

        sources.add_component(MFilename(fs::relative(exec->path(), settings.option_src_dir_)));

        // TODO: break the line if too long

        for (auto &src : source_files) {
            sources.add_component(MFilename(fs::relative(src->path(), settings.option_src_dir_)));
        }

        t.sources_section.add_component(std::move(sources));

        index++;
    }
}

void MFileV4::build_objects_section() {
    // OBJS1 = $(SRCS1:%.cpp=$(OBJ_DIR)/$(SRC_DIR)/%.o)
    // OBJS2 = $(SRCS2:%.cpp=$(OBJ_DIR)/$(SRC_DIR)/%.o)
    // DEPS = $(OBJS1:.o=.d) $(OBJS2:.o=.d)

    MVariableDef deps("DEPS");
    

    // TODO: break the line if too long

    for (size_t index = 1; index <= m_executable.size(); index++) {
        std::string index_str = (index == 1 && m_executable.size() == 1) ? "" : std::to_string(index);

        MVariableDef objects("OBJS" + index_str);
        objects.add_component(MSimpleVariable{"SRCS" + index_str + ":%.cpp=$(OBJ_DIR)/$(SRC_DIR)/%.o"});

        deps.add_component(MSimpleVariable{"OBJS" + index_str + ".o=.d"});

        t.objects_section.add_component(std::move(objects));
    }

    t.objects_section.add_component(std::move(deps));
}

void MFileV4::build_targets_section() {
    if (m_executable.size() == 1) {
        MRule target_rule{MSimpleVariable{"TARGET"}};
        target_rule.add_prerequisites(MSimpleVariable{"OBJS"}, MSimpleVariable{"CONFIG_FILE"});
        target_rule.add_recipe(MRecipe("mkdir -p $(@D)", MRecipePrefix::ECHO_OFF));
        target_rule.add_recipe(MRecipe("echo \"$(CXX) $(CXXFLAGS) ... -o $@ $(LDFLAGS)\"", MRecipePrefix::ECHO_OFF));
        target_rule.add_recipe(MRecipe("$(CXX) $(CXXFLAGS) $(OBJS) -o $@ $(LDFLAGS)", MRecipePrefix::ECHO_OFF));
        target_rule.add_recipe(MRecipe("$(call check_build_params)"));

        t.targets_section.add_component(std::move(target_rule));
    } else {
        // TODO: deal with multiple executables
    }
}
