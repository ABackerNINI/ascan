#ifndef __TEMPLATE_LEXER_H__
#define __TEMPLATE_LEXER_H__

#include <string>
#include <vector>

enum TokenType {
    TOK_ERROR = 0,
    TOK_RESERVED,
    TOK_OPERATOR,
    TOK_IDENTIFIER,
    TOK_NUMBER,
    TOK_STRING,
    TOK_SPACE,
    TOK_NEWLINE,
    TOK_PLAIN_TEXT,
    TOK_EOF
};

class Token {
  public:
    int lineno;
    int colno;
    TokenType type;
    std::string lx;
};

class TLexer {
  public:
    TLexer(std::istream &in) : in(in) {}

    bool parse();

    const Token &get_token() { return tokens.at(index++); }

    const Token &peek_token() const { return tokens.at(index); }

  private:
    Token next_token();

  private:
    std::istream &in;
    std::vector<Token> tokens;
    int index = 0;
};

#endif // __TEMPLATE_LEXER_H__
