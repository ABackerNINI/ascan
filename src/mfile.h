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
#include <type_traits>
#include <vector>

// class mfile {
//   public:
//     mfile(std::vector<cfile> &cfiles, Config &cfg, uint32_t flags);
//     int output();
//
//   private:
//     void prepare();
//     void output_build_details();
//     void output_targets();
//     void output_compile_to_objects();
//     void output_executable_details();
//     void output_mode_control();
//     void output_clean_up();
//     void output_phony();
//
//     // Output dependencies using gcc -MM.
//     void output_mm_dependencies();
//
//     void output_mk_build_if_option_b();
//
//     void output_part();
//
//     void output_gitignore();
//
//   private:
//     std::vector<cfile> &m_cfiles;
//     Config &m_cfg;
//     // bool m_flag_a;
//     uint32_t m_flags;
//     std::ofstream m_fout;
//     std::vector<cfile *> m_executable;
//     std::vector<std::string> m_binaries; // files to be added to gitignore
//     Align m_align;
//
//     bool m_c;
//     bool m_cc;
//     bool m_cpp;
//     std::string m_build_path; // = "$(BUILD)/" iff OPTION_B is set
// };

// Component of a Makefile.
class MComponent {
  public:
    MComponent(const std::string &name = "MComponent") : m_name(name) {}
    virtual ~MComponent() = default;
    virtual std::string to_string() const { return m_name; }

  protected:
    std::string m_name;
};

// Check if a type is a subclass of MComponent.
template <typename T> struct is_mcomponent : std::is_base_of<MComponent, T> {};

// Compound component of a Makefile.
class MCompComponent : public MComponent {
  public:
    MCompComponent(const std::string &name = "MCompComponent", const std::string &separator = "")
        : MComponent(name), separator(separator) {}

    virtual ~MCompComponent() {
        for (auto &sub_component : m_sub_components) {
            delete sub_component;
        }
    }

    void add_sub_component(MComponent *sub_component) { m_sub_components.push_back(sub_component); }

    template <typename T> friend MCompComponent &operator<<(MCompComponent &mcc, T sub_component) {
        if constexpr (std::is_pointer_v<T> && is_mcomponent<std::remove_pointer_t<T>>::value) {
            mcc.add_sub_component(sub_component);
        } else if constexpr (is_mcomponent<T>::value) {
            mcc.add_sub_component(new T(sub_component));
        } else if constexpr (std::is_same_v<T, const char *>) {
            mcc.add_sub_component(new MComponent(sub_component));
        } else if constexpr (std::is_same_v<T, std::string>) {
            mcc.add_sub_component(new MComponent(sub_component));
        } else {
            static_assert(is_mcomponent<T>::value, "T must be a subclass of MComponent");
        }

        return mcc;
    }

    void set_separator(const std::string &separator) { this->separator = separator; }

    virtual std::string to_string() const {
        std::string str;
        for (const auto &sub_component : m_sub_components) {
            str += sub_component->to_string() + separator;
        }
        return str;
    }

  protected:
    std::string separator;
    std::vector<MComponent *> m_sub_components;
};

class MComment : public MComponent {
  public:
    MComment(const std::string &comment = "") : MComponent(comment) {}

    friend MComment &operator<<(MComment &mc, const std::string &comment) {
        mc.m_name += comment;
        return mc;
    }

    friend MComment &operator<<(MComment &mc, const char *comment) {
        mc.m_name += comment;
        return mc;
    }

    virtual std::string to_string() const { return "# " + m_name; }
};

class MFilename : public MComponent {
  public:
    MFilename(const std::string &name) : MComponent(name) {}

    virtual std::string to_string() const {
        for (auto &c : m_name) {
            if (std::isspace(c)) {
                return "\"" + m_name + "\"";
            }
        }
        return m_name;
    }
};

class MSimpleVariable : public MComponent {
  public:
    MSimpleVariable(const std::string &name) : MComponent(name) {}

    virtual std::string to_string() const { return "$(" + m_name + ")"; }
};

class MSimpleVariableDef : public MComponent {
  public:
    MSimpleVariableDef(const std::string &name, const std::string &value) : MComponent(name), m_value(value) {}

    virtual std::string to_string() const { return m_name + " = " + m_value; }

  protected:
    std::string m_value;
};

class MVariableDef : public MCompComponent {
  public:
    MVariableDef(const std::string &name) : MCompComponent(name, " ") {}

    virtual std::string to_string() const { return m_name + " = " + MCompComponent::to_string(); }
};

class MBlankLine : public MComponent {
  public:
    MBlankLine() : MComponent("MBlankLine") {}

    virtual std::string to_string() const { return ""; }
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
    MCommand(const std::string &command = "", MCommandPrefix prefix = M_COMMAND_PREFIX_NONE)
        : MCompComponent("MCommand", " "), m_prefix(prefix) {
        if (!command.empty()) {
            add_sub_component(new MComponent(command));
        }
    }

    MCommand(MCommandPrefix prefix) : MCompComponent("MCommand", " "), m_prefix(prefix) {}

    virtual std::string to_string() const {
        if (m_prefix == M_COMMAND_PREFIX_NONE) {
            return std::string("\t") + MCompComponent::to_string();
        }
        return std::string("\t") + char(m_prefix) + MCompComponent::to_string();
    }

    void set_prefix(MCommandPrefix prefix) { m_prefix = prefix; }

  protected:
    MCommandPrefix m_prefix;
};

class MRule : public MComponent {
  public:
    MRule(const std::string &name = "MRule") : MComponent(name) {}
    MRule(const MComponent &name) : MComponent(name.to_string()) {}

    virtual ~MRule() {
        for (auto &dependency : m_dependencies) {
            delete dependency;
        }
        for (auto &command : m_commands) {
            delete command;
        }
    }

    void add_dependency(MComponent *dependency) { m_dependencies.push_back(dependency); }

    void add_command(MComponent *command) { m_commands.push_back(command); }

    virtual std::string to_string() const {
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
    MFile(std::vector<cfile> &cfiles, Config &cfg, uint32_t flags);

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

    template <typename T> friend MFile &operator<<(MFile &mfile, T component) {
        if constexpr (std::is_pointer_v<T> && is_mcomponent<std::remove_pointer_t<T>>::value) {
            mfile.add_component(component);
        } else if constexpr (is_mcomponent<T>::value) {
            mfile.add_component(new T(component));
        } else if constexpr (std::is_same_v<T, const char *>) {
            mfile.add_component(new MComponent(component));
        } else if constexpr (std::is_same_v<T, std::string>) {
            mfile.add_component(new MComponent(component));
        } else {
            static_assert(is_mcomponent<T>::value, "T must be a subclass of MComponent");
        }

        return mfile;
    }

    int output();

  protected:
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

    void add_mkdir_build_cmd_if_option_b(MRule *rule);

    void output_part();

    void output_gitignore();

  protected:
    std::vector<MComponent *> m_components;

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

#endif //_AUTO_SCAN_MFILE_H_
