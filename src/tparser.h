#ifndef __TEMPLATE_PARSER_H__
#define __TEMPLATE_PARSER_H__

#include "tlexer.h"
#include <string>
#include <vector>

/*
# ASCAN TEMPLATE SYNTAX

template -> compound
compound -> (plain-text | block)*
plain-text -> .*
block -> "__ASCAN_BEGIN__" (block | statement)* "__ASCAN_END__"

statement -> if-stmt | shell-stmt | comment

if-stmt -> "if" expression ":" compound ["elif" expression ":" compound]* ["else" ":" compound] "fi"

shell-stmt -> shell-command (shell-args)* "\n"
shell-command -> .*
shell-args -> expression | plain-text

comment -> ^"#" .*

expression -> conditional-expression
conditional-expression -> logical-or-expression ["?" expression ":" expression]
logical-or-expression -> [logical-or-expression "||"] logical-and-expression
logical-and-expression -> [logical-and-expression "&&"] inclusive-or-expression
inclusive-or-expression -> [inclusive-or-expression "|"] exclusive-or-expression
exclusive-or-expression -> [exclusive-or-expression "^"] and-expression
and-expression -> [and-expression "&"] equality-expression
equality-expression -> [equality-expression ("==" | "!=")] relational-expression
relational-expression -> [relational-expression ("<" | ">" | "<=" | ">=")] shift-expression
shift-expression -> [shift-expression ("<<" | ">>")] additive-expression
additive-expression -> [additive-expression ("+" | "-")] multiplicative-expression
multiplicative-expression -> [multiplicative-expression ("*" | "/" | "%")] unary-expression
unary-expression -> [("+" | "-" | "!" | "~")] postfix-expression
postfix-expression -> primary-expression
primary-expression -> literal | variable | cmd-args | "(" expression ")"

literal -> number | string
variable -> "$" "(" identifier ")"
cmd-args -> "--" identifier

identifier -> [a-zA-Z_][a-zA-Z0-9_.]*
number -> [0-9]+
string -> '"' .* '"'
 */

class AscanInfo {
  public:
    AscanInfo() {}
    ~AscanInfo() {}

    // void output();
    // void add_output(const std::string &output);
    // void add_output(const char *output);
    // void add_output(int output);
    // void add_output(const std::vector<std::string> &output);
    // void add_output(const std::vector<char *> &output);
    // void add_output(const std::vector<int> &output);

  private:
    std::vector<std::string> outputs;
};

class Template {
  public:
    virtual bool parse(std::istream &in)   = 0;
    virtual bool execute(AscanInfo &ainfo) = 0;
    virtual ~Template() {}
};

class TemplateParser {
  public:
    TemplateParser(std::istream &in) : lexer(in) {}

    bool parse();
    bool execute(AscanInfo);

  private:
    TLexer lexer;
};

#endif // __TEMPLATE_PARSER_H__
