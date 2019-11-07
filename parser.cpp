#include <cassert>
#include <string.h>
#include <iostream>

#include "parser.h"

enum PARSER_STATE {
    STA_INIT,
    STA_IDENTIFIER,
    STA_SYM,
    STA_NUMBER,
    STA_COMMENT1,
    STA_COMMENT2_1,
    STA_COMMENT2_2,
    STA_SLASH,
    STA_QUOTE,
    STA_QUOTE_BACK_SLASH,
    STA_EOF
};

enum INCLUDE_MAIN_STATE {
    IMS_INIT,
    IMS_POUND,  // #
    IMS_INCLUDE,
    IMS_MAIN_RETURN_TYPE,
    IMS_MAIN,
    IMS_MAIN_LEFT_PAREN,  // (
    IMS_MAIN_VOID,        // void
    IMS_MAIN_ARG_1_TYPE,
    IMS_MAIN_ARG_1,
    IMS_MAIN_ARG_1_COMMA,
    IMS_MAIN_ARG_2_TYPE,
    IMS_MAIN_ARG_2_ASTERISK_1,
    IMS_MAIN_ARG_2_ASTERISK_2,
    IMS_MAIN_ARG_2_1,                // char *argv[]
    IMS_MAIN_ARG_2_2,                // char **argv
    IMS_MAIN_ARG_2_LEFT_BRACKET_1,   // [
    IMS_MAIN_ARG_2_RIGHT_BRACKET_2,  // ]
    IMS_MAIN_RIGHT_PAREN             // )
};

static size_t erase_last_slash(const char *str, size_t length);
static string make_path(const char *p1, const char *p2);

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

vector<string> scan_includes_and_main_func(const char *filename,
                                           bool *main_func) {
    //! INCLUDES:
    // # include "header.h"
    // should ignore comments (// | /**/)

    //! MAIN FUNC:
    // return type maybe (int | void)
    // function name should be (main)
    // arguments list maybe (NULL | void | int,char **argv | int,char*argv[])
    // should ignore comments (// | /**/)

    vector<string> includes;

    *main_func = false;

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Can't open source file \"%s\" @%s line %d\n", filename,
                __func__, __LINE__);
        return includes;
    }

    INCLUDE_MAIN_STATE state = IMS_INIT;
    PARSER_TOKEN_TYPE type;

    char buff[BUFSIZ];
    while ((type = next_token(fp, buff, BUFSIZ)) != TYPE_EOF) {
        switch (state) {
            case IMS_INIT:
                if (type == TYPE_SYM) {
                    if (strcmp(buff, "#") == 0) {
                        state = IMS_POUND;
                    }
                } else if (type == TYPE_IDENTIFIER) {
                    if (strcmp(buff, "int") == 0 || strcmp(buff, "void") == 0) {
                        state = IMS_MAIN_RETURN_TYPE;
                    }
                }
                break;
            case IMS_POUND:
                if (type == TYPE_IDENTIFIER) {
                    if (strcmp(buff, "include") == 0) {
                        state = IMS_INCLUDE;
                        break;
                    }
                }
                state = IMS_INIT;
                break;
            case IMS_INCLUDE:
                if (type == TYPE_STRING) {
                    printf("\t|%s|\n", buff);
                    includes.push_back(make_path(filename, buff));
                }
                state = IMS_INIT;
                break;
            case IMS_MAIN_RETURN_TYPE:
                if (type == TYPE_IDENTIFIER) {
                    if (strcmp(buff, "main") == 0) {
                        state = IMS_MAIN;
                        break;
                    }
                }
                state = IMS_INIT;
                break;
            case IMS_MAIN:
                if (type == TYPE_SYM) {
                    if (strcmp(buff, "(") == 0) {
                        state = IMS_MAIN_LEFT_PAREN;
                        break;
                    }
                }
                state = IMS_INIT;
                break;
            case IMS_MAIN_LEFT_PAREN:
                if (type == TYPE_SYM) {
                    if (strcmp(buff, ")") == 0) {
                        state = IMS_MAIN_RIGHT_PAREN;
                        break;
                    }
                } else if (type == TYPE_IDENTIFIER) {
                    if (strcmp(buff, "int") == 0) {
                        state = IMS_MAIN_ARG_1_TYPE;
                        break;
                    } else if (strcmp(buff, "void") == 0) {
                        state = IMS_MAIN_VOID;
                        break;
                    }
                }
                state = IMS_INIT;
                break;
            case IMS_MAIN_VOID:
                if (strcmp(buff, ")") == 0) {
                    state = IMS_MAIN_RIGHT_PAREN;
                    break;
                }
                state = IMS_INIT;
                break;
            case IMS_MAIN_ARG_1_TYPE:
                if (type == TYPE_IDENTIFIER) {
                    state = IMS_MAIN_ARG_1;
                    break;
                }
                state = IMS_INIT;
                break;
            case IMS_MAIN_ARG_1:
                if (type == TYPE_SYM) {
                    if (strcmp(buff, ",") == 0) {
                        state = IMS_MAIN_ARG_1_COMMA;
                        break;
                    }
                }
                state = IMS_INIT;
                break;
            case IMS_MAIN_ARG_1_COMMA:
                if (type == TYPE_IDENTIFIER) {
                    if (strcmp(buff, "char") == 0) {
                        state = IMS_MAIN_ARG_2_TYPE;
                        break;
                    }
                }
                state = IMS_INIT;
                break;
            case IMS_MAIN_ARG_2_TYPE:
                if (type == TYPE_SYM) {
                    if (strcmp(buff, "*") == 0) {
                        state = IMS_MAIN_ARG_2_ASTERISK_1;
                        break;
                    }
                }
                state = IMS_INIT;
                break;
            case IMS_MAIN_ARG_2_ASTERISK_1:
                if (type == TYPE_SYM) {
                    if (strcmp(buff, "*") == 0) {
                        state = IMS_MAIN_ARG_2_ASTERISK_2;
                        break;
                    }
                } else if (type == TYPE_IDENTIFIER) {
                    state = IMS_MAIN_ARG_2_1;
                    break;
                }
                state = IMS_INIT;
                break;
            case IMS_MAIN_ARG_2_ASTERISK_2:
                if (type == TYPE_IDENTIFIER) {
                    state = IMS_MAIN_ARG_2_2;
                    break;
                }
                state = IMS_INIT;
                break;
            case IMS_MAIN_ARG_2_1:
                if (type == TYPE_SYM) {
                    if (strcmp(buff, "[") == 0) {
                        state = IMS_MAIN_ARG_2_LEFT_BRACKET_1;
                        break;
                    }
                }
                state = IMS_INIT;
                break;
            case IMS_MAIN_ARG_2_2:
                if (type == TYPE_SYM) {
                    if (strcmp(buff, ")") == 0) {
                        state = IMS_MAIN_RIGHT_PAREN;
                        break;
                    }
                }
                state = IMS_INIT;
                break;
            case IMS_MAIN_ARG_2_LEFT_BRACKET_1:
                if (type == TYPE_SYM) {
                    if (strcmp(buff, "]") == 0) {
                        state = IMS_MAIN_ARG_2_RIGHT_BRACKET_2;
                        break;
                    }
                }
                state = IMS_INIT;
                break;
            case IMS_MAIN_ARG_2_RIGHT_BRACKET_2:
                if (type == TYPE_SYM) {
                    if (strcmp(buff, ")") == 0) {
                        state = IMS_MAIN_RIGHT_PAREN;
                        break;
                    }
                }
                state = IMS_INIT;
                break;
            case IMS_MAIN_RIGHT_PAREN:
                break;
        }

        if (state == IMS_MAIN_RIGHT_PAREN) {
            *main_func = true;  // find main func
            state = IMS_INIT;
        }
    }

    fclose(fp);

    return includes;
}

enum PARSER_TOKEN_TYPE next_token(FILE *fp, char *buff, size_t buff_size) {
    static char c = '\0';
    enum PARSER_STATE state = STA_INIT;
    PARSER_TOKEN_TYPE type = TYPE_EOF;

    size_t len = 0;

    if (c != '\0') {
        buff[len++] = c;
    }
    while (!feof(fp)) {
        if (c == '\0') {
            c = fgetc(fp);
            if (c == EOF) {
                c = '\0';
                // type = TYPE_EOF;
                // break;
            }
            if (state < STA_COMMENT1 || state > STA_COMMENT2_2) {
                buff[len++] = c;
                if (len == buff_size - 1) {
                    fprintf(stderr, "Reach maximum buffer size\n");
                    type = TYPE_REACH_MAX_BUFFER_SIZE;
                    break;
                }
            }
        }

        switch (state) {
            case STA_INIT:
                if (isalpha(c) || c == '_') {
                    state = STA_IDENTIFIER;
                } else if (isspace(c)) {
                    // still STA_INIT
                    --len;
                } else if (isdigit(c)) {
                    state = STA_NUMBER;
                } else if (c == '/') {
                    state = STA_SLASH;
                } else if (c == '\"') {
                    state = STA_QUOTE;
                    --len;
                } else {
                    state = STA_SYM;
                }
                c = '\0';
                break;
            case STA_IDENTIFIER:
                if (!isalpha(c) && !isdigit(c) && c != '_') {
                    type = TYPE_IDENTIFIER;
                    goto END;
                } else {
                    c = '\0';
                }
                break;
            case STA_SYM:
                type = TYPE_SYM;
                goto END;
            case STA_SLASH:
                if (c == '/') {
                    state = STA_COMMENT1;
                    len = 0;
                } else if (c == '*') {
                    state = STA_COMMENT2_1;
                    len = 0;
                } else {
                    type = TYPE_SYM;  // divide symble
                    goto END;
                }
                c = '\0';
                break;
            case STA_QUOTE:
                if (c == '\"') {
                    type = TYPE_STRING;
                    c = '\0';
                    --len;
                    goto END;
                } else if (c == '\\') {
                    state = STA_QUOTE_BACK_SLASH;
                }
                c = '\0';
                break;
            case STA_QUOTE_BACK_SLASH:
                state = STA_QUOTE;
                c = '\0';
                break;
            case STA_NUMBER:
                if (!isdigit(c)) {
                    type = TYPE_NUMBER;
                    goto END;
                } else {
                    c = '\0';
                }
                break;
            case STA_COMMENT1:
                if (c == '\n') {
                    state = STA_INIT;
                }
                c = '\0';
                break;
            case STA_COMMENT2_1:
                if (c == '*') {
                    state = STA_COMMENT2_2;
                }
                c = '\0';
                break;
            case STA_COMMENT2_2:
                if (c == '/') {
                    state = STA_INIT;
                } else if (c == '*') {
                    // still STA_COMMENT2_2
                } else {
                    state = STA_COMMENT2_1;
                }
                c = '\0';
                break;
            default:
                assert(false);
                break;
        }
    }

END:
    if (c != '\0') {
        --len;
    }

    buff[len] = '\0';

    return type;
}

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

static size_t erase_last_slash(const char *str, size_t length) {
    if (length > 0) {
        while (str[length] != '/') {
            if (--length == 0) {
                break;
            }
        }
    }
    return length;
}

static string make_path(const char *p1, const char *p2) {
    size_t n1 = strlen(p1);

    n1 = erase_last_slash(p1, n1);
    while (strncmp(p2, "../", 3) == 0) {
        p2 += 3;
        n1 = erase_last_slash(p1, n1);
    }
    return string(p1).substr(0, n1) + "/" + p2;
}
