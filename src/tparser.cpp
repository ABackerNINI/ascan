#include "tparser.h"
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

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

class Compound;
class PlainText;
class Block;
class Statement;
class IfStmt;
class ShellStmt;
class Expression;
class ConditionalExpression;
class LogicalOrExpression;
class LogicalAndExpression;
class InclusiveOrExpression;
class ExclusiveOrExpression;
class AndExpression;
class EqualityExpression;
class RelationalExpression;
class ShiftExpression;
class AdditiveExpression;
class MultiplicativeExpression;
class UnaryExpression;
class PostfixExpression;
class PrimaryExpression;
class Literal;
class Variable;
class CmdArgs;
class Identifier;
class Number;

std::string next_token(std::istream &in) {
}

class Compound : public Template {
  public:
    Compound() {}

    virtual bool parse(std::istream &in) override {}

    virtual bool execute(AscanInfo &ainfo) override {}

    virtual ~Compound() {}

  private:
    std::vector<Template *> plain_texts_or_blocks;
};

class PlainText : public Template {
  public:
    PlainText() {}

    virtual bool parse(std::istream &in) override {}

    virtual bool execute(AscanInfo &ainfo) override {}

    virtual ~PlainText() {}

  private:
    std::string text;
};

class Block : public Template {
  public:
    Block() {}

    virtual bool parse(std::istream &in) override {}

    virtual bool execute(AscanInfo &ainfo) override {}

    virtual ~Block() {}

  private:
    std::vector<Statement *> statements;
};

class Statement : public Template {
  public:
    Statement() {}

    virtual bool parse(std::istream &in) override {}

    virtual bool execute(AscanInfo &ainfo) override {}

    virtual ~Statement() {}

  private:
    IfStmt *if_stmt = nullptr;
    ShellStmt *shell_stmt = nullptr;
};

class IfStmt : public Template {
  public:
    IfStmt() {}

    virtual bool parse(std::istream &in) override {}

    virtual bool execute(AscanInfo &ainfo) override {}

    virtual ~IfStmt() {}

  private:
    Expression *expression = nullptr;
    Compound *compound = nullptr;
    std::vector<Expression *> elif_expressions;
    std::vector<Compound *> elif_compounds;
    Compound *else_compound = nullptr;
};

class ShellStmt : public Template {
  public:
    ShellStmt() {}

    virtual bool parse(std::istream &in) override {}

    virtual bool execute(AscanInfo &ainfo) override {}

    virtual ~ShellStmt() {}

  private:
    std::string shell_command;
    std::vector<Template *> shell_args;
};

class Expression : public Template {
  public:
    Expression() {}

    virtual bool parse(std::istream &in) override {}

    virtual bool execute(AscanInfo &ainfo) override {}

    virtual std::string evaluate(AscanInfo &ainfo) = 0;
    virtual ~Expression() {}
};

class ConditionalExpression : public Expression {
  public:
    ConditionalExpression() {}

    virtual bool parse(std::istream &in) override {}

    virtual bool execute(AscanInfo &ainfo) override {}

    virtual std::string evaluate(AscanInfo &ainfo) override {}

    virtual ~ConditionalExpression() {}

  private:
    LogicalOrExpression *logical_or_expression = nullptr;
    Expression *expression1 = nullptr;
    Expression *expression2 = nullptr;
};

class LogicalOrExpression : public Expression {
  public:
    LogicalOrExpression() {}

    virtual bool parse(std::istream &in) override {}

    virtual bool execute(AscanInfo &ainfo) override {}

    virtual std::string evaluate(AscanInfo &ainfo) override {}

    virtual ~LogicalOrExpression() {}

  private:
    LogicalOrExpression *logical_or_expression = nullptr;
    LogicalAndExpression *logical_and_expressions;
};

class LogicalAndExpression : public Expression {
  public:
    LogicalAndExpression() {}

    virtual bool parse(std::istream &in) override {}

    virtual bool execute(AscanInfo &ainfo) override {}

    virtual std::string evaluate(AscanInfo &ainfo) override {}

    virtual ~LogicalAndExpression() {}

  private:
    LogicalAndExpression *logical_and_expression = nullptr;
    InclusiveOrExpression *inclusive_or_expression = nullptr;
};

class InclusiveOrExpression : public Expression {
  public:
    InclusiveOrExpression() {}

    virtual bool parse(std::istream &in) override {}

    virtual bool execute(AscanInfo &ainfo) override {}

    virtual std::string evaluate(AscanInfo &ainfo) override {}

    virtual ~InclusiveOrExpression() {}

  private:
    InclusiveOrExpression *inclusive_or_expression = nullptr;
    ExclusiveOrExpression *exclusive_or_expression = nullptr;
};

class ExclusiveOrExpression : public Expression {
  public:
    ExclusiveOrExpression() {}

    virtual bool parse(std::istream &in) override {}

    virtual bool execute(AscanInfo &ainfo) override {}

    virtual std::string evaluate(AscanInfo &ainfo) override {}

    virtual ~ExclusiveOrExpression() {}

  private:
    ExclusiveOrExpression *exclusive_or_expression = nullptr;
    AndExpression *and_expression = nullptr;
};

class AndExpression : public Expression {
  public:
    AndExpression() {}

    virtual bool parse(std::istream &in) override {}

    virtual bool execute(AscanInfo &ainfo) override {}

    virtual std::string evaluate(AscanInfo &ainfo) override {}

    virtual ~AndExpression() {}

  private:
    AndExpression *and_expression = nullptr;
    EqualityExpression *equality_expression = nullptr;
};

class EqualityExpression : public Expression {
  public:
    EqualityExpression() {}

    virtual bool parse(std::istream &in) override {}

    virtual bool execute(AscanInfo &ainfo) override {}

    virtual std::string evaluate(AscanInfo &ainfo) override {}

    virtual ~EqualityExpression() {}

  private:
    EqualityExpression *equality_expression = nullptr;
    RelationalExpression *relational_expression = nullptr;
};

class RelationalExpression : public Expression {
  public:
    RelationalExpression() {}

    virtual bool parse(std::istream &in) override {}

    virtual bool execute(AscanInfo &ainfo) override {}

    virtual std::string evaluate(AscanInfo &ainfo) override {}

    virtual ~RelationalExpression() {}

  private:
    RelationalExpression *relational_expression = nullptr;
    ShiftExpression *shift_expression = nullptr;
};

class ShiftExpression : public Expression {
  public:
    ShiftExpression() {}

    virtual bool parse(std::istream &in) override {}

    virtual bool execute(AscanInfo &ainfo) override {}

    virtual std::string evaluate(AscanInfo &ainfo) override {}

    virtual ~ShiftExpression() {}

  private:
    ShiftExpression *shift_expression = nullptr;
    AdditiveExpression *additive_expression = nullptr;
};

class AdditiveExpression : public Expression {
  public:
    AdditiveExpression() {}

    virtual bool parse(std::istream &in) override {}

    virtual bool execute(AscanInfo &ainfo) override {}

    virtual std::string evaluate(AscanInfo &ainfo) override {}

    virtual ~AdditiveExpression() {}

  private:
    AdditiveExpression *additive_expression = nullptr;
    MultiplicativeExpression *multiplicative_expression = nullptr;
};

class MultiplicativeExpression : public Expression {
  public:
    MultiplicativeExpression() {}

    virtual bool parse(std::istream &in) override {}

    virtual bool execute(AscanInfo &ainfo) override {}

    virtual std::string evaluate(AscanInfo &ainfo) override {}

    virtual ~MultiplicativeExpression() {}

  private:
    MultiplicativeExpression *multiplicative_expression = nullptr;
    UnaryExpression *unary_expression = nullptr;
};

class UnaryExpression : public Expression {
  public:
    UnaryExpression() {}

    virtual bool parse(std::istream &in) override {}

    virtual bool execute(AscanInfo &ainfo) override {}

    virtual std::string evaluate(AscanInfo &ainfo) override {}

    virtual ~UnaryExpression() {}

  private:
    std::string op;
    PostfixExpression *postfix_expression = nullptr;
};

class PostfixExpression : public Expression {
  public:
    PostfixExpression() {}

    virtual bool parse(std::istream &in) override {}

    virtual bool execute(AscanInfo &ainfo) override {}

    virtual std::string evaluate(AscanInfo &ainfo) override {}

    virtual ~PostfixExpression() {}

  private:
    PrimaryExpression *primary_expression = nullptr;
};

class PrimaryExpression : public Expression {
  public:
    PrimaryExpression() {}

    virtual bool parse(std::istream &in) override {}

    virtual bool execute(AscanInfo &ainfo) override {}

    virtual std::string evaluate(AscanInfo &ainfo) override {}

    virtual ~PrimaryExpression() {}

  private:
    Literal *literal = nullptr;
    Variable *variable = nullptr;
    CmdArgs *cmd_args = nullptr;
    Expression *expression = nullptr;
};

class Literal : public Expression {
  public:
    Literal() {}

    virtual bool parse(std::istream &in) override {}

    virtual bool execute(AscanInfo &ainfo) override {}

    virtual std::string evaluate(AscanInfo &ainfo) override {}

    virtual ~Literal() {}

  private:
    Number *number = nullptr;
    std::string string;
};

class Variable : public Expression {
  public:
    Variable() {}

    virtual bool parse(std::istream &in) override {}

    virtual bool execute(AscanInfo &ainfo) override {}

    virtual std::string evaluate(AscanInfo &ainfo) override {}

    virtual ~Variable() {}

  private:
    Identifier *identifier = nullptr;
};

class CmdArgs : public Expression {
  public:
    CmdArgs() {}

    virtual bool parse(std::istream &in) override {}

    virtual bool execute(AscanInfo &ainfo) override {}

    virtual std::string evaluate(AscanInfo &ainfo) override {}

    virtual ~CmdArgs() {}

  private:
    Identifier *identifier = nullptr;
};

class Identifier : public Expression {
  public:
    Identifier() {}

    virtual bool parse(std::istream &in) override {}

    virtual bool execute(AscanInfo &ainfo) override {}

    virtual std::string evaluate(AscanInfo &ainfo) override {}

    virtual ~Identifier() {}

  private:
    std::string id;
};

class Number : public Expression {
  public:
    Number() {}

    virtual bool parse(std::istream &in) override {}

    virtual bool execute(AscanInfo &ainfo) override {}

    virtual std::string evaluate(AscanInfo &ainfo) override {}

    virtual ~Number() {}

  private:
    int num;
};

bool TemplateParser::parse() {
}

bool TemplateParser::execute(AscanInfo) {
}

int main(int argc, char **argv) {
    std::ifstream in("template/v2/template.mk");
    if (!in.is_open()) {
        std::cerr << "Failed to open file" << std::endl;
        return 1;
    }

    TemplateParser tparser(in);
    tparser.parse();

    AscanInfo ainfo;
    tparser.execute(ainfo);

    return 0;
}
