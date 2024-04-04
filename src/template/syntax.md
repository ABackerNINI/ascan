# ASCAN TEMPLATE SYNTAX

template -> compound
compound -> (plain-text | block)*
plain-text -> .*
block -> "__ASCAN_BEGIN__" statements "__ASCAN_END__"

statements -> (if-stmt | shell-stmt)*

if-stmt -> "if" expression ":" compound ["elif" expression ":" compound] ["else" ":" compound] "fi"

shell-stmt -> shell-command (shell-args)*
shell-command -> .*
shell-args -> .* | variable

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

literal -> number
variable -> "$" "(" identifier ")"
cmd-args -> "--" identifier

identifier -> [a-zA-Z_][a-zA-Z0-9_.]*
number -> [0-9]+
