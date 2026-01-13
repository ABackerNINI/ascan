#ifndef _AUTO_SCAN_MFILE_H_
#define _AUTO_SCAN_MFILE_H_

#include "cfile.h"
#include "settings.h"
#include "traits.h" // IWYU pragma: keep
#include <cstdio>
#include <string>
#include <type_traits>
#include <vector>

// Component of a Makefile.
class MComponent {
  public:
    std::string indent() const { return std::string(indent_, '\t'); }
    void indent(int i) { indent_ = i; }
    void indent(int i) const { indent_ = i; }
    int get_indent() const { return indent_; }

    virtual ~MComponent() = default;
    virtual std::string to_string() const { return "MComponent"; }

  protected:
    // Indentation level '\t's of the component.
    // It is mutable so that in the const function to_string of a component, the indent of sub components can be
    // changed.
    mutable int indent_{0};
};

// Check if a type is a subclass of MComponent.
template <typename T> inline constexpr bool is_mcomponent_v = std::is_base_of_v<MComponent, std::remove_reference_t<T>>;

class MText : public MComponent {
  public:
    MText(const std::string &text = "") : text_(text) {}
    MText(const char *text) : text_(text) {}
    MText(const MComponent &text) : MText(text.to_string()) {}

    virtual ~MText() = default;
    virtual std::string to_string() const override { return indent() + text_; }

  protected:
    std::string text_;
};

// Compound component of a Makefile.
class MCompComponent : public MComponent {
  public:
    MCompComponent(const std::string &separator = "") : separator_(separator) {}

    MCompComponent(const MCompComponent &other) = delete;
    MCompComponent &operator=(const MCompComponent &other) = delete;

    MCompComponent(MCompComponent &&other) { *this = std::move(other); }
    MCompComponent &operator=(MCompComponent &&other) {
        if (this != &other) {
            std::swap(separator_, other.separator_);
            std::swap(sub_components_, other.sub_components_);
        }
        return *this;
    }

    virtual ~MCompComponent() {
        for (auto &sub_component : sub_components_) {
            delete sub_component;
        }
    }

    template <typename T> MCompComponent &add_component(T &&sub_component) {
        if constexpr (std::is_convertible_v<T, MComponent *>) {
            sub_components_.push_back(sub_component);
        } else if constexpr (is_mcomponent_v<T>) {
            sub_components_.push_back(new std::remove_cvref_t<T>(std::forward<T>(sub_component)));
        } else if constexpr (std::is_convertible_v<std::remove_cvref_t<T>, std::string>) {
            sub_components_.push_back(new MText(sub_component));
        } else {
            static_assert(false, "Invalid type for MCompComponent");
        }
        return *this;
    }

    template <typename... T> MCompComponent &add_components(T &&...sub_components) {
        (add_component(std::forward<T>(sub_components)), ...);
        return *this;
    }

    template <typename T> friend MCompComponent &operator<<(MCompComponent &mcc, T &&sub_component) {
        return mcc.add_component(std::forward<T>(sub_component));
    }

    void set_separator(const std::string &separator) { this->separator_ = separator; }

    virtual std::string to_string() const {
        std::string str;

        if (!sub_components_.empty()) {
            str += indent() + sub_components_[0]->to_string();
            for (size_t i = 1; i < sub_components_.size(); ++i) {
                str += separator_;
                if (!separator_.empty() && separator_.back() == '\n') {
                    str += indent();
                }
                str += sub_components_[i]->to_string();
            }
        }

        return str;
    }

  protected:
    std::string separator_;
    std::vector<MComponent *> sub_components_;
};

template <typename... T> MCompComponent make_comp_component(T &&...sub_components) {
    MCompComponent mcc;
    mcc.add_components(std::forward<T>(sub_components)...);
    return mcc;
}

class MComment : public MText {
  public:
    MComment(const std::string &comment = "") : MText(comment) {}

    MComment(const MComponent &comment) : MComment(comment.to_string()) {}

    friend MComment &operator<<(MComment &mc, const std::string &comment) {
        mc.text_ += comment;
        return mc;
    }

    friend MComment &operator<<(MComment &mc, const char *comment) {
        mc.text_ += comment;
        return mc;
    }

    virtual std::string to_string() const { return indent() + "# " + text_; }
};

class MFilename : public MComponent {
  public:
    MFilename(const std::string &filename) : filename_(filename) {}

    MFilename(const MComponent &filename) : MFilename(filename.to_string()) {}

    virtual std::string to_string() const {
        for (auto &c : filename_) {
            if (std::isspace(c)) {
                return "\"" + filename_ + "\"";
            }
        }
        return indent() + filename_;
    }

  protected:
    std::string filename_;
};

enum class VariableAssignmentType {
    RECURSIVELY_EXPANDED, // '='
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
    MSimpleVariable(const std::string &varname) : varname_(varname) {}

    MSimpleVariable(const MComponent &varname) : MSimpleVariable(varname.to_string()) {}

    virtual std::string to_string() const { return indent() + "$(" + varname_ + ")"; }

  protected:
    std::string varname_;
};

class MSimpleVariableDef : public MComponent {
  public:
    MSimpleVariableDef(const std::string &varname,
                       const std::string &value,
                       VariableAssignmentType assignment_type = VariableAssignmentType::RECURSIVELY_EXPANDED)
        : varname_(varname), value_(value), assignment_type_(assignment_type) {}

    MSimpleVariableDef(const MComponent &varname,
                       const MComponent &value,
                       VariableAssignmentType assignment_type = VariableAssignmentType::RECURSIVELY_EXPANDED)
        : MSimpleVariableDef(varname.to_string(), value.to_string(), assignment_type) {}

    virtual std::string to_string() const {
        return indent() + varname_ + " " + variable_assignment_type_to_string(assignment_type_) + " " + value_;
    }

  protected:
    std::string varname_;
    std::string value_;
    VariableAssignmentType assignment_type_;
};

class MVariableDef : public MComponent {
  public:
    MVariableDef(const std::string &varname,
                 VariableAssignmentType assignment_type = VariableAssignmentType::RECURSIVELY_EXPANDED)
        : varname_(varname), assignment_type_(assignment_type) {}

    MVariableDef(const MComponent &varname,
                 VariableAssignmentType assignment_type = VariableAssignmentType::RECURSIVELY_EXPANDED)
        : MVariableDef(varname.to_string(), assignment_type) {}

    template <typename T>
    MVariableDef(const std::string &varname,
                 T &&value,
                 VariableAssignmentType assignment_type = VariableAssignmentType::RECURSIVELY_EXPANDED)
        : MVariableDef(varname, assignment_type) {
        value_.add_component(std::forward<T>(value));
    }

    template <typename T>
    MVariableDef(const MComponent &varname,
                 T &&value,
                 VariableAssignmentType assignment_type = VariableAssignmentType::RECURSIVELY_EXPANDED)
        : MVariableDef(varname.to_string(), std::forward<T>(value), assignment_type) {}

    template <typename T> MVariableDef &add_component(T &&val_component) {
        value_.add_component(std::forward<T>(val_component));
        return *this;
    }

    template <typename... T> MVariableDef &add_components(T &&...val_components) {
        (value_.add_component(std::forward<T>(val_components)), ...);
        return *this;
    }

    void set_separator(const std::string &separator) { value_.set_separator(separator); }

    virtual std::string to_string() const {
        return indent() + varname_ + " " + variable_assignment_type_to_string(assignment_type_) + " " +
               value_.to_string();
    }

  public:
    std::string varname_;
    VariableAssignmentType assignment_type_;
    MCompComponent value_{" "};
};

class MBlankLine : public MComponent {
  public:
    MBlankLine() {}

    // TODO: fix this
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

class MRecipe : public MComponent {
  public:
    MRecipe(const std::string &command = "", MRecipePrefix prefix = MRecipePrefix::NONE) : prefix_(prefix) {
        if (!command.empty()) {
            cmd_.add_component(new MText(command));
        }
    }

    MRecipe(const MComponent &command, MRecipePrefix prefix = MRecipePrefix::NONE)
        : MRecipe(command.to_string(), prefix) {}

    MRecipe(MRecipePrefix prefix) : prefix_(prefix) {}

    virtual std::string to_string() const {
        if (prefix_ == MRecipePrefix::NONE) {
            return indent() + std::string("\t") + cmd_.to_string();
        }
        return indent() + std::string("\t") + char(prefix_) + cmd_.to_string();
    }

    void set_prefix(MRecipePrefix prefix) { prefix_ = prefix; }

    void set_separator(const std::string &separator) { cmd_.set_separator(separator); }

    template <typename T> MRecipe &add_component(T &&cmd_component) {
        cmd_.add_component(std::forward<T>(cmd_component));
        return *this;
    }

    template <typename... T> MRecipe &add_components(T &&...cmd_components) {
        (cmd_.add_component(std::forward<T>(cmd_components)), ...);
        return *this;
    }

    template <typename T> friend MRecipe &operator<<(MRecipe &mcc, T &&sub_component) {
        mcc.cmd_.add_component(std::forward<T>(sub_component));
        return mcc;
    }

  protected:
    MRecipePrefix prefix_;
    MCompComponent cmd_{" "};
};

class MRule : public MComponent {
  public:
    MRule(const std::string &target) : target_(target) {}

    MRule(const MComponent &target) : MRule(target.to_string()) {}

    template <typename T> MRule &add_prerequisite(T &&prerequisite) {
        prerequisites_.add_component(std::forward<T>(prerequisite));
        return *this;
    }

    template <typename... T> MRule &add_prerequisites(T &&...prerequisite_components) {
        (prerequisites_.add_component(std::forward<T>(prerequisite_components)), ...);
        return *this;
    }

    template <typename T> MRule &add_recipe(T &&recipe) {
        recipes_.add_component(std::forward<T>(recipe));
        return *this;
    }

    template <typename... T> MRule &add_recipes(T &&...recipe_components) {
        (recipes_.add_component(std::forward<T>(recipe_components)), ...);
        return *this;
    }

    virtual std::string to_string() const {
        std::string dependencies = prerequisites_.to_string();

        recipes_.indent(get_indent());
        std::string commands = recipes_.to_string();

        return indent() + target_ + (dependencies.empty() ? ":" : ": " + dependencies) +
               (commands.empty() ? "" : "\n" + commands);
    }

  protected:
    std::string target_;
    MCompComponent prerequisites_{" "};
    MCompComponent recipes_{"\n"};
};

class MQuoted : public MComponent {
  public:
    MQuoted(const std::string &content) { content_.add_components(content); }

    MQuoted(const MComponent &content) : MQuoted(content.to_string()) {}

    template <typename T> MQuoted(T &&content) { content_.add_component(std::forward<T>(content)); }

    template <typename T> MQuoted &add_content(T &&content_component) {
        content_.add_component(std::forward<T>(content_component));
        return *this;
    }

    template <typename... T> MQuoted &add_contents(T &&...content_components) {
        (content_.add_component(std::forward<T>(content_components)), ...);
        return *this;
    }

    virtual std::string to_string() const { return indent() + "\"" + content_.to_string() + "\""; }

  protected:
    MCompComponent content_{""};
};

class MFile {
  public:
    MFile(const Settings &settings, std::vector<cfile> &cfiles) : settings(settings), cfiles_(cfiles) {}

    virtual ~MFile() {}

    int output();

    virtual int build() = 0;

    virtual std::string to_string() const = 0;

  protected:
    const Settings &settings;
    std::vector<cfile> &cfiles_; // all source files
};

#endif //_AUTO_SCAN_MFILE_H_
