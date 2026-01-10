#include "mfilev4.h"

#include "common.h"
#include "fs.h"
#include "options.h"
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
        t.proj_name = m_executable[0]->name();
    } else if (m_executable.size() > 1) {
        // TODO: proj_name = dir name of the project root
        t.proj_name = "ascan_project";
    }

    build_options_section();
    build_targets();
    build_sources_section();
    build_targets_section();

    fs::path tempfile = "../template/v4/template.mk";
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
        // return 1;
    }

    if (temp.find("__ASCAN::") != std::string::npos) {
        print_error("Failed to replace all strings in template\n");
        return 2;
    }

    std::cout << "targets_section: " << t.targets_section.to_string() << std::endl;

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
         [](const cfile *a, const cfile *b) { return a->name() < b->name(); });
}

void MFileV4::build_options_section() {}

void MFileV4::build_targets() {}

void MFileV4::build_sources_section() {}

void MFileV4::build_targets_section() {}
