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

// Component of a Makefile.
class MComponent {
  public:
    MComponent() {}
    virtual ~MComponent() = default;
    virtual std::string to_string() const { return "MComponent"; }
};

// Check if a type is a subclass of MComponent.
template <typename T> struct is_mcomponent : std::is_base_of<MComponent, std::remove_reference_t<T>> {};

template <typename _Tp> inline constexpr bool is_mcomponent_v = is_mcomponent<_Tp>::value;

class MText : public MComponent {
  public:
    MText(const std::string &text = "") : m_text(text) {}
    MText(const char *text) : m_text(text) {}
    virtual ~MText() = default;
    virtual std::string to_string() const override { return m_text; }

  protected:
    std::string m_text;
};

// Compound component of a Makefile.
class MCompComponent : public MComponent {
  public:
    MCompComponent(const std::string &separator = "") : separator(separator) {}

    virtual ~MCompComponent() {
        for (auto &sub_component : m_sub_components) {
            delete sub_component;
        }
    }

    template <typename T> MCompComponent &add_sub_component(T &&sub_component) {
        if constexpr (std::is_pointer_v<std::remove_reference_t<T>> &&
                      is_mcomponent_v<std::remove_pointer_t<std::remove_reference_t<T>>>) {
            m_sub_components.push_back(sub_component);
        } else if constexpr (is_mcomponent_v<T>) {
            m_sub_components.push_back(new std::remove_cv_t<std::remove_reference_t<T>>(sub_component));
        } else if constexpr (std::is_same_v<const char *, std::remove_cv_t<std::remove_reference_t<T>>>) {
            m_sub_components.push_back(new MText(sub_component));
        } else if constexpr (std::is_same_v<std::string, std::remove_cv_t<std::remove_reference_t<T>>>) {
            m_sub_components.push_back(new MText(sub_component));
        } else {
            static_assert(false, "Invalid type for MCompComponent");
        }
        return *this;
    }

    template <typename T> friend MCompComponent &operator<<(MCompComponent &mcc, T sub_component) {
        return mcc.add_sub_component(sub_component);
    }

    void set_separator(const std::string &separator) { this->separator = separator; }

    virtual std::string to_string() const {
        std::string str;

        if (!m_sub_components.empty()) {
            str += m_sub_components[0]->to_string();
            for (size_t i = 1; i < m_sub_components.size(); ++i) {
                str += separator + m_sub_components[i]->to_string();
            }
        }

        return str;
    }

  protected:
    std::string separator;
    std::vector<MComponent *> m_sub_components;
};

class MComment : public MText {
  public:
    MComment(const std::string &comment = "") : MText(comment) {}

    friend MComment &operator<<(MComment &mc, const std::string &comment) {
        mc.m_text += comment;
        return mc;
    }

    friend MComment &operator<<(MComment &mc, const char *comment) {
        mc.m_text += comment;
        return mc;
    }

    virtual std::string to_string() const { return "# " + m_text; }
};

class MFilename : public MComponent {
  public:
    MFilename(const std::string &filename) : m_filename(filename) {}

    virtual std::string to_string() const {
        for (auto &c : m_filename) {
            if (std::isspace(c)) {
                return "\"" + m_filename + "\"";
            }
        }
        return m_filename;
    }

  protected:
    std::string m_filename;
};

class MSimpleVariable : public MComponent {
  public:
    MSimpleVariable(const std::string &varname) : m_varname(varname) {}

    virtual std::string to_string() const { return "$(" + m_varname + ")"; }

  protected:
    std::string m_varname;
};

class MSimpleVariableDef : public MComponent {
  public:
    MSimpleVariableDef(const std::string &varname, const std::string &value) : m_varname(varname), m_value(value) {}

    virtual std::string to_string() const { return m_varname + " = " + m_value; }

  protected:
    std::string m_varname;
    std::string m_value;
};

class MVariableDef : public MCompComponent {
  public:
    MVariableDef(const std::string &varname) : MCompComponent(" "), m_varname(varname) {}

    virtual std::string to_string() const { return m_varname + " = " + MCompComponent::to_string(); }

  public:
    std::string m_varname;
};

class MBlankLine : public MComponent {
  public:
    MBlankLine() {}

    virtual std::string to_string() const { return ""; }
};

// Prefix of a command.
//   '@': turn off echo.
//   '-': ignore error, make will exit when error occurs.
//   '+': ignore make's -n -t -q options.
enum MRecipePrefix {
    M_COMMAND_PREFIX_NONE = ' ',
    M_COMMAND_PREFIX_ECHO_OFF = '@',
    M_COMMAND_PREFIX_IGNORE_ERROR = '-',
    M_COMMAND_PREFIX_IGNORE_MAKE_OPTIONS = '+'
};

class MRecipe : public MCompComponent {
  public:
    MRecipe(const std::string &command = "", MRecipePrefix prefix = M_COMMAND_PREFIX_NONE)
        : MCompComponent(" "), m_prefix(prefix) {
        if (!command.empty()) {
            add_sub_component(new MText(command));
        }
    }

    MRecipe(MRecipePrefix prefix) : MCompComponent(" "), m_prefix(prefix) {}

    virtual std::string to_string() const {
        if (m_prefix == M_COMMAND_PREFIX_NONE) {
            return std::string("\t") + MCompComponent::to_string();
        }
        return std::string("\t") + char(m_prefix) + MCompComponent::to_string();
    }

    void set_prefix(MRecipePrefix prefix) { m_prefix = prefix; }

  protected:
    MRecipePrefix m_prefix;
};

class MRule : public MComponent {
  public:
    MRule(const std::string &target) : m_target(target) {}
    MRule(const MComponent &target) : m_target(target.to_string()) {}

    virtual ~MRule() {}

    template <typename T> MRule &add_prerequisite(T prerequisite) {
        m_prerequisites.add_sub_component(prerequisite);
        return *this;
    }

    template <typename T> MRule &add_recipe(T recipe) {
        m_recipes.add_sub_component(recipe);
        return *this;
    }

    virtual std::string to_string() const {
        std::string dependencies = m_prerequisites.to_string();

        std::string commands = m_recipes.to_string();

        return m_target + (dependencies.empty() ? ":" : ": " + dependencies) +
               (commands.empty() ? "" : "\n" + commands);
    }

  protected:
    std::string m_target;
    MCompComponent m_prerequisites{" "};
    MCompComponent m_recipes{"\n"};
};

class MQuoted : public MCompComponent {
  public:
    MQuoted(const std::string &content) { m_sub_components.push_back(new MText(content)); }

    template <typename T> MQuoted(const T &content) { m_sub_components.push_back(new T(content)); }

    virtual std::string to_string() const { return "\"" + MCompComponent::to_string() + "\""; }
};

class MFile {
  public:
    MFile(std::vector<cfile> &cfiles, Config &cfg, uint32_t flags) : m_cfiles(cfiles), m_cfg(cfg), m_flags(flags) {}

    virtual ~MFile() {}

    int output();

    virtual int build() = 0;

    void add_component(MComponent *component) { m_components.add_sub_component(component); }

  protected:
    std::vector<cfile> &m_cfiles;
    Config &m_cfg;
    uint32_t m_flags;

    MCompComponent m_components{"\n"};
};

#endif //_AUTO_SCAN_MFILE_H_
