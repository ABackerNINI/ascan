#include "tlexer.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

enum LowLevelTokenType { LTOK_ERROR = 0, LTOK_PLAIN_TEXT, LTOK_ASCAN_BLOCK, LTOK_EOF };

static const char *LowLevelTokenNames[] = {"ERROR", "PLAIN_TEXT", "ASCAN_BLOCK", "EOF"};

class LowLevelToken {
  public:
    int lineno;
    int colno;
    LowLevelTokenType type;
    std::string lx;

    LowLevelToken(int lineno, int colno, LowLevelTokenType type, const std::string &lx = "")
        : lineno(lineno), colno(colno), type(type), lx(lx) {}
};

class LowLevelLexer {
  public:
    LowLevelLexer(std::istream &in) : in(in) {}

    bool parse() {
        bool eof = false;
        lineno   = 0;
        while (!eof) {
            LowLevelToken token = next_token();

            if (token.type == LTOK_EOF) {
                eof = true;
                break;
            }

            // Merge same type tokens
            if (!tokens.empty() && tokens.back().type == token.type) {
                tokens.back().lx += token.lx;
                continue;
            }

            tokens.push_back(token);
        }
        return true;
    }

    void print_tokens() {
        for (auto &token : tokens) {
            std::cout << "LOW LEVEL TOKEN BEGIN: " << token.lineno << ":" << token.colno
                      << " Type: " << LowLevelTokenNames[token.type] << std::endl;
            std::cout << token.lx << std::endl;
            std::cout << "LOW LEVEL TOKEN END\n" << std::endl;
        }
    }

    const LowLevelToken &get_token() { return tokens.at(index++); }

    const LowLevelToken &peek_token() const { return tokens.at(index); }

  private:
    size_t count(const std::string &str, const std::string &substr) {
        size_t count = 0;
        size_t pos   = 0;
        while ((pos = str.find(substr, pos)) != std::string::npos) {
            count++;
            pos += substr.length();
        }
        return count;
    }

    LowLevelToken next_token() {
        if (line.empty()) {
            lineno++;
            if (!std::getline(in, line)) {
                LowLevelToken token(lineno, 1, LTOK_EOF);
                line.clear();
                return token;
            }
            line += "\n";

            size_t pos = line.find("__ASCAN_BEGIN__");
            if (pos == std::string::npos) {
                LowLevelToken token(lineno, 1, LTOK_PLAIN_TEXT, line);
                line.clear();
                return token;
            }

            LowLevelToken token(lineno, 1, LTOK_PLAIN_TEXT, line.substr(0, pos));
            line.erase(0, pos);
            colno = pos + 1;
            return token;
        } else {
            std::stringstream ss(line);
            std::string lx;
            while (ss >> lx) {
                if (lx == "__ASCAN_BEGIN__") {
                    ascan_begin_count++;
                } else if (lx == "__ASCAN_END__") {
                    ascan_begin_count--;
                    if (ascan_begin_count == 0) {
                        ssize_t pos = ss.tellg();
                        pos         = pos == -1 ? line.size() : pos;
                        LowLevelToken token(lineno, colno, LTOK_ASCAN_BLOCK, line.substr(0, pos));
                        line.erase(0, pos);
                        colno += ss.tellg();
                        return token;
                    }
                }
            }

            LowLevelToken token = LowLevelToken(lineno, colno,
                                                ascan_begin_count > 0 ? LTOK_ASCAN_BLOCK : LTOK_PLAIN_TEXT, line);
            line.clear();
            if (std::getline(in, line)) {
                line += "\n";
            }
            return token;
        }
    }

  private:
    std::istream &in;
    std::vector<LowLevelToken> tokens;
    int index  = 0;
    int lineno = 0;
    int colno  = 1;
    std::string line;
    int ascan_begin_count = 0;
};

Token TLexer::next_token() {
    // Delimiters: spaces
}

bool TLexer::parse() {
    LowLevelLexer llexer(in);
    if (!llexer.parse()) {
        return false;
    }
    llexer.print_tokens();
    return true;
}

int main(int argc, char **argv) {
    std::ifstream in("template/v2/template.mk");
    if (!in.is_open()) {
        std::cerr << "Failed to open file" << std::endl;
        return 1;
    }

    TLexer lexer(in);
    lexer.parse();
    return 0;
}
