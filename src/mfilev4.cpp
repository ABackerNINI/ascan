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
    prepare();

    if (m_executable.size() == 1) {
        t.proj_name = m_executable[0]->stem();
    } else if (m_executable.size() > 1) {
        // TODO: proj_name = dir name of the project root
        t.proj_name = "ascan_project";
    }

    t.src_dir = settings.option_src_dir_;
    t.bld_dir = settings.option_build_dir_;
    t.bin_dir = settings.option_bin_dir_;

    build_options_section();
    build_targets();
    build_sources_section();
    build_targets_section();

    fs::path tempfile = "templates/v4/template.mk";
    std::string temp = read_file(tempfile);

    std::vector<std::pair<std::string, std::string>> replaces;
    replaces.push_back({"__ASCAN::PROJECT_NAME__", t.proj_name});
    replaces.push_back({"__ASCAN::OPTIONS_SECTION__", t.options_section.to_string()});
    replaces.push_back({"__ASCAN::SRC_DIR__", t.src_dir});
    replaces.push_back({"__ASCAN::BLD_DIR__", t.bld_dir});
    replaces.push_back({"__ASCAN::BIN_DIR__", t.bin_dir});
    replaces.push_back({"__ASCAN::TARGETS__", t.targets.to_string()});
    replaces.push_back({"__ASCAN::SOURCES_SECTION__", t.sources_section.to_string()});
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

void MFileV4::prepare() {
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

    // Find all executables
    for (auto &cfile : m_cfiles) {
        if (cfile.have_main_func() && cfile.is_source()) {
            m_executable.push_back(&cfile);
        }
    }

    // Sort executables by name
    sort(m_executable.begin(), m_executable.end(),
         [](const cfile *a, const cfile *b) { return a->stem() < b->stem(); });
}

void MFileV4::build_options_section() {
    // TODO: add options for the following three fields
    MSimpleVariableDef config{"CONFIG", "debug", VariableAssignmentType::CONDITIONAL};
    MSimpleVariableDef cxx{"CXX", "g++", VariableAssignmentType::CONDITIONAL};
    MSimpleVariableDef std{"STD", "c++17", VariableAssignmentType::CONDITIONAL};

    t.options_section.add_component(config);
    t.options_section.add_component(MBlankLine());
    t.options_section.add_component(cxx);
    t.options_section.add_component(std);
}

void MFileV4::build_targets() {
    if (m_executable.size() == 1) {
        MVariableDef target{"TARGET"};
        target.add_component(MSimpleVariable("BIN_DIR"));
        target.add_component("/");
        target.add_component(MSimpleVariable("PROJECT"));
        t.targets.add_component(std::move(target));
    } else if (m_executable.size() > 1) {
        // TODO: implement multiple targets
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
    if (m_executable.size() == 1) {
        // TODO: wildcard sources

        MVariableDef sources{"SRCS", VariableAssignmentType::RECURSIVELY_EXPANDED};
        sources.set_separator(" ");
        auto &exec = m_executable[0];
        find_all_sources_and_headers(m_cfiles, exec);

        std::vector<cfile *> source_files;
        for (cfile &cf : m_cfiles) {
            if (cf.visited() && cf.is_source()) {
                if (&cf != exec) {
                    source_files.push_back(&cf);
                }
                cf.set_visited(false);
            }
        }

        std::sort(source_files.begin(), source_files.end(), [](cfile *a, cfile *b) { return a->path() < b->path(); });

        sources.add_component(new MFilename(fs::relative(exec->path(), settings.option_src_dir_)));

        for (auto &src : source_files) {
            sources.add_component(new MFilename(fs::relative(src->path(), settings.option_src_dir_)));
        }

        t.sources_section.add_component(std::move(sources));
    } else {
        // TODO: deal with multiple executables
    }
}

void MFileV4::build_targets_section() {
    if (m_executable.size() == 1) {
        MRule target_rule{MSimpleVariable{"TARGET"}};
        target_rule.add_prerequisite(MSimpleVariable{"OBJS"});
        target_rule.add_prerequisite(MSimpleVariable{"MODE_FILE"});
        target_rule.add_recipe(MRecipe("mkdir -p $(@D)", MRecipePrefix::ECHO_OFF));
        target_rule.add_recipe(MRecipe("$(CXX) $(CXXFLAGS) $(OBJS) -o $@ $(LDFLAGS)"));
        target_rule.add_recipe(MRecipe("$(call check_build_params)"));

        t.targets_section.add_component(std::move(target_rule));
    } else {
        // TODO: deal with multiple executables
    }
}
