#ifndef _AUTO_SCAN_MFILE_H_
#define _AUTO_SCAN_MFILE_H_

#include "align.h"
#include "cfile.h"
#include "config.h"
#include "debug.h"
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

class mfile {
  public:
    mfile(std::vector<cfile> &cfiles, Config &cfg, uint32_t flags);
    int output();

  private:
    void prepare();
    void output_build_details();
    void output_targets();
    void output_compile_to_objects();
    void output_executable_details();
    void output_mode_control();
    void output_clean_up();
    void output_phony();

    // Output dependencies using gcc -MM.
    void output_mm_dependencies();

    void output_mk_build_if_option_b();

    void output_part();

    void output_gitignore();

  private:
    std::vector<cfile> &m_cfiles;
    Config &m_cfg;
    // bool m_flag_a;
    uint32_t m_flags;
    std::ofstream m_fout;
    std::vector<cfile *> m_executable;
    std::vector<std::string> m_binaries; // files to be added to gitignore
    Align m_align;

    bool m_c;
    bool m_cc;
    bool m_cpp;
    std::string m_build_path; // = "$(BUILD)/" iff OPTION_B is set
};

// Component of a Makefile.
class MComponent {
  public:
    MComponent(const std::string &name = "MComponent") : m_name(name) {}
    virtual ~MComponent() = default;
    virtual std::string to_string(const std::string &delim = "") const { return m_name; }

  protected:
    std::string m_name;
};

// Compound component of a Makefile.
class MCompComponent : public MComponent {
  public:
    MCompComponent(const std::string &name = "MCompComponent") : MComponent(name) {}

    virtual ~MCompComponent() {
        for (auto &sub_component : m_sub_components) {
            delete sub_component;
        }
    }

    void add_sub_component(MComponent *sub_component) { m_sub_components.push_back(sub_component); }

    MCompComponent &operator<<(MComponent *sub_component) {
        add_sub_component(sub_component);
        return *this;
    }

    virtual std::string to_string(const std::string &delim = " ") const {
        std::string str;
        for (const auto &sub_component : m_sub_components) {
            str += sub_component->to_string() + delim;
        }
        return str;
    }

  protected:
    std::vector<MComponent *> m_sub_components;
};

class MComment : public MComponent {
  public:
    MComment(const std::string &comment = "") : MComponent(comment) {}

    MComment &operator<<(const std::string &comment) {
        m_name += comment;
        return *this;
    }

    virtual std::string to_string(const std::string &delim = "") const {
        (void)delim;

        return "# " + m_name;
    }
};

class MFilename : public MComponent {
  public:
    MFilename(const std::string &name) : MComponent(name) {}

    virtual std::string to_string(const std::string &delim = "") const {
        (void)delim;

        for (auto &c : m_name) {
            if (std::isspace(c)) {
                return "\"" + m_name + "\"";
            }
        }
        return m_name;
    }
};

class MVariable : public MComponent {
  public:
    MVariable(const std::string &name) : MComponent(name) {}

    virtual std::string to_string(const std::string &delim = "") const {
        (void)delim;

        return "$(" + m_name + ")";
    }
};

class MSimpleVariableDef : public MComponent {
  public:
    MSimpleVariableDef(const std::string &name, const std::string &value) : MComponent(name), m_value(value) {}

    virtual std::string to_string(const std::string &delim = "") const {
        (void)delim;

        return m_name + " = " + m_value;
    }

  protected:
    std::string m_value;
};

class MVariableDef : public MCompComponent {
  public:
    MVariableDef(const std::string &name) : MCompComponent(name) {}

    virtual std::string to_string(const std::string &delim = "") const {
        (void)delim;

        return m_name + " = " + MCompComponent::to_string();
    }
};

class MBlankLine : public MComponent {
  public:
    MBlankLine() : MComponent("MBlankLine") {}

    virtual std::string to_string(const std::string &delim = "") const {
        (void)delim;
        return "\n";
    }
};

//   '@': turn off echo.
//   '-': ignore error, make will exit when error occurs.
//   '+': ignore make's -n -t -q options.
enum MCommandPrefix {
    M_COMMAND_PREFIX_NONE = ' ',
    M_COMMAND_PREFIX_ECHO_OFF = '@',
    M_COMMAND_PREFIX_IGNORE_ERROR = '-',
    M_COMMAND_PREFIX_IGNORE_MAKE_OPTIONS = '+'
};

class MCommand : public MCompComponent {
  public:
    MCommand(const std::string &name = "MCommand") : MCompComponent(name), m_prefix(M_COMMAND_PREFIX_NONE) {}

    virtual std::string to_string(const std::string &delim = "") const {
        (void)delim;

        return std::string("\t") + char(m_prefix) + MCompComponent::to_string(delim);
    }

    void set_prefix(MCommandPrefix prefix) { m_prefix = prefix; }

  protected:
    MCommandPrefix m_prefix;
};

class MRule : public MComponent {
  public:
    MRule(const std::string &name = "MRule") : MComponent(name) {}

    virtual std::string to_string(const std::string &delim = "") const {
        (void)delim;

        std::string dependencies;
        for (const auto &dependency : m_dependencies) {
            dependencies += dependency->to_string() + " ";
        }

        std::string commands;
        for (const auto &command : m_commands) {
            commands += command->to_string() + "\n";
        }

        return m_name + ": " + dependencies + "\n" + commands;
    }

  protected:
    std::vector<MComponent *> m_dependencies;
    std::vector<MComponent *> m_commands;
};

class MFile {
  public:
    MFile() {}

    ~MFile() {
        for (auto &component : m_components) {
            delete component;
        }
    }

    void add_component(MComponent *component) { m_components.push_back(component); }

    std::string to_string() const {
        std::string str;
        for (const auto &component : m_components) {
            str += component->to_string() + "\n";
        }
        return str;
    }

    MFile &operator<<(MComponent *component) {
        add_component(component);
        return *this;
    }

  protected:
    std::vector<MComponent *> m_components;
};

#endif //_AUTO_SCAN_MFILE_H_
