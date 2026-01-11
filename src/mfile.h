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

#if __cplusplus < 202002L
namespace std {
// C++20 std::remove_cvref_t
template <typename _Tp> using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<_Tp>>;
} // namespace std
#endif

class MText : public MComponent {
  public:
    MText(const std::string &text = "") : m_text(text) {}
    MText(const char *text) : m_text(text) {}
    virtual ~MText() = default;
    virtual std::string to_string() const override { return m_text; }

  protected:
    std::string m_text;
};

template <typename T> constexpr bool is_convertible_to_string_v = std::is_convertible_v<T, std::string>;

// Compound component of a Makefile.
class MCompComponent : public MComponent {
  public:
    MCompComponent(const std::string &separator = "") : separator(separator) {}

    MCompComponent(const MCompComponent &other) = delete;
    MCompComponent &operator=(const MCompComponent &other) = delete;

    MCompComponent(MCompComponent &&other) { *this = std::move(other); }
    MCompComponent &operator=(MCompComponent &&other) {
        if (this != &other) {
            std::swap(separator, other.separator);
            std::swap(m_sub_components, other.m_sub_components);
        }
        return *this;
    }

    virtual ~MCompComponent() {
        for (auto &sub_component : m_sub_components) {
            delete sub_component;
        }
    }

    template <typename T> MCompComponent &add_component(T &&sub_component) {
        if constexpr (std::is_convertible_v<T, MComponent *>) {
            m_sub_components.push_back(sub_component);
        } else if constexpr (is_mcomponent_v<T>) {
            m_sub_components.push_back(new std::remove_cvref_t<T>(std::forward<T>(sub_component)));
        } else if constexpr (is_convertible_to_string_v<std::remove_cvref_t<T>>) {
            m_sub_components.push_back(new MText(sub_component));
        } else {
            static_assert(false, "Invalid type for MCompComponent");
        }
        return *this;
    }

    template <typename T> friend MCompComponent &operator<<(MCompComponent &mcc, T sub_component) {
        return mcc.add_component(sub_component);
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

enum class VariableAssignmentType {
    RECURSIVELY_EXPANDED, // =
    SIMPLY_EXPANDED, // := or ::=
    IMMEDIATELY_EVALUATED, // :::=
    APPEND, // +=
    CONDITIONAL, // ?=
};

constexpr const char *variable_assignment_type_to_string(VariableAssignmentType type) {
    switch (type) {
    case VariableAssignmentType::RECURSIVELY_EXPANDED:
        return "=";
    case VariableAssignmentType::SIMPLY_EXPANDED:
        return ":=";
    case VariableAssignmentType::IMMEDIATELY_EVALUATED:
        return ":::=";
    case VariableAssignmentType::APPEND:
        return "+=";
    case VariableAssignmentType::CONDITIONAL:
        return "?=";
    default:
        return "";
    }
}

class MSimpleVariable : public MComponent {
  public:
    MSimpleVariable(const std::string &varname) : m_varname(varname) {}

    virtual std::string to_string() const { return "$(" + m_varname + ")"; }

  protected:
    std::string m_varname;
};

class MSimpleVariableDef : public MComponent {
  public:
    MSimpleVariableDef(const std::string &varname,
                       const std::string &value,
                       VariableAssignmentType assignment_type = VariableAssignmentType::RECURSIVELY_EXPANDED)
        : m_varname(varname), m_value(value), m_assignment_type(assignment_type) {}

    virtual std::string to_string() const {
        return m_varname + " " + variable_assignment_type_to_string(m_assignment_type) + " " + m_value;
    }

  protected:
    std::string m_varname;
    std::string m_value;
    VariableAssignmentType m_assignment_type;
};

class MVariableDef : public MCompComponent {
  public:
    MVariableDef(const std::string &varname,
                 VariableAssignmentType assignment_type = VariableAssignmentType::RECURSIVELY_EXPANDED)
        : MCompComponent(""), m_varname(varname), m_assignment_type(assignment_type) {}

    template <typename T>
    MVariableDef(const std::string &varname,
                 T &&value,
                 VariableAssignmentType assignment_type = VariableAssignmentType::RECURSIVELY_EXPANDED)
        : MCompComponent(""), m_varname(varname), m_assignment_type(assignment_type) {
        add_component(std::forward<T>(value));
    }

    virtual std::string to_string() const {
        return m_varname + " " + variable_assignment_type_to_string(m_assignment_type) + " " +
               MCompComponent::to_string();
    }

  public:
    std::string m_varname;
    VariableAssignmentType m_assignment_type;
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
enum class MRecipePrefix {
    NONE = ' ', // ' '
    ECHO_OFF = '@', // @
    IGNORE_ERROR = '-', // -
    IGNORE_MAKE_OPTIONS = '+' // +
};

class MRecipe : public MCompComponent {
  public:
    MRecipe(const std::string &command = "", MRecipePrefix prefix = MRecipePrefix::NONE)
        : MCompComponent(" "), m_prefix(prefix) {
        if (!command.empty()) {
            add_component(new MText(command));
        }
    }

    MRecipe(MRecipePrefix prefix) : MCompComponent(" "), m_prefix(prefix) {}

    virtual std::string to_string() const {
        if (m_prefix == MRecipePrefix::NONE) {
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

    template <typename T> MRule &add_prerequisite(T &&prerequisite) {
        m_prerequisites.add_component(std::forward<T>(prerequisite));
        return *this;
    }

    template <typename T> MRule &add_recipe(T &&recipe) {
        m_recipes.add_component(std::forward<T>(recipe));
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

    template <typename T> MQuoted(T &&content) { add_component(std::forward<T>(content)); }

    virtual std::string to_string() const { return "\"" + MCompComponent::to_string() + "\""; }
};

class MFile {
  public:
    MFile(std::vector<cfile> &cfiles, Config &cfg, uint32_t flags) : m_cfiles(cfiles), m_cfg(cfg), m_flags(flags) {}

    virtual ~MFile() {}

    int output();

    virtual int build() = 0;

    virtual std::string to_string() const = 0;

  protected:
    std::vector<cfile> &m_cfiles; // all source files
    Config &m_cfg; // config
    uint32_t m_flags; // CLI flags
};

#endif //_AUTO_SCAN_MFILE_H_
