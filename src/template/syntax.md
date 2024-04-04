# ASCAN TEMPLATE SYNTAX

template -> compound
compound -> (plain-text | block)*
plain-text -> .*
block -> "__ASCAN_BEGIN__" statements "__ASCAN_END__"
statements -> (if-stmt | shell-stmt)*
if-stmt -> "if" expression ":" compound [else ":" compound] "fi"

unqualified-id -> variable
id-expression -> unqualified-id | qualified-id
literal -> number
primary-expression -> literal | id-expression | "(" expression ")"
postfix-expression -> primary-expression
unary-expression -> [("+" | "-" | "!" | "~")] postfix-expression
multiplicative-expression -> [multiplicative-expression ("*" | "/" | "%")] unary-expression
additive-expression -> [additive-expression ("+" | "-")] multiplicative-expression
shift-expression -> [shift-expression ("<<" | ">>")] additive-expression
compare-expression -> [compare-expression "<=>"] shift-expression
relational-expression -> [relational-expression ("<" | ">" | "<=" | ">=")] compare-expression
equality-expression -> [equality-expression ("==" | "!=")] relational-expression
and-expression -> [and-expression "&"] equality-expression
exclusive-or-expression -> [exclusive-or-expression "^"] and-expression
inclusive-or-expression -> [inclusive-or-expression "|"] exclusive-or-expression
logical-and-expression -> [logical-and-expression "&&"] inclusive-or-expression
logical-or-expression -> [logical-or-expression "||"] logical-and-expression
conditional-expression -> logical-or-expression ["?" expression ":" expression]
expression -> conditional-expression

variable -> "$" "(" identifier ")"
identifier -> [a-zA-Z_][a-zA-Z0-9_]*
number -> [0-9]+
shell-stmt -> "shell" shell-command shell-args
shell-command -> .*
shell-args -> (.* | variable)*

