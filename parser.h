#ifndef _AUTO_SCAN_PARSER_H_
#define _AUTO_SCAN_PARSER_H_

#include <string>
#include <vector>

enum PARSER_TOKEN_TYPE {
    TYPE_IDENTIFIER, // '[_a-zA-Z][_0-9a-zA-Z]*'
    TYPE_SYM,        // '[]()<>{},;:+-*/#',etc
    TYPE_NEW_LINE,   // '\n'
    TYPE_NUMBER,     // '[0-9]+'
    TYPE_COMMENT1,   // '//*\n'
    TYPE_COMMENT2,   // '/***/'
    TYPE_STRING,     // '"*"'
    TYPE_REACH_MAX_BUFFER_SIZE,
    TYPE_EOF // EOF
};

// Scan includes and check main function in c/c++ file.
// Includes: #include "header.h"
// Main func: (int|void) main((NULL|void|int p1,char **p2|int p1, char *p2[]))
//
// Return an array containing included file names.
std::vector<std::string> scan_includes_and_main_func(const char *filename,
                                           bool *main_func);

// Simple lex parser, get next token from file.
// Note: do NOT parse multiple files at the same time.
//
// Return token type.
enum PARSER_TOKEN_TYPE next_token(FILE *fp, char *buff, size_t buff_size);

#endif //_AUTO_SCAN_PARSER_H_
