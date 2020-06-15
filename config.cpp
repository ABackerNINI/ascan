#include "config.h"

#include <string.h>
#include <iostream>
#include <cassert>

#include "parser.h"

enum CONFIG_STATE { CS_COMPILER, CS_EXECUTABLE, CS_IGNORE, CS_UNKNOWN };

enum CONFIG_TYPE {
    CT_TAG,
    CT_KV,
    CT_K,
    CT_NOT_TAG,
    CT_EMPTY,
    CT_ERROR_TAG_MISSING_RIGHT_BRACKET,
    CT_ERROR_KV
};

Config::Config()
    : k_cc(CONFIG_DEFAULT_K_CC),
      k_cxx(CONFIG_DEFAULT_K_CXX),
      k_cflags(CONFIG_DEFAULT_K_CFLAGS),
      k_cxxflags(CONFIG_DEFAULT_K_CXXFLAGS),
      k_bd(CONFIG_DEFAULT_K_BD),
      k_bin(CONFIG_DEFAULT_K_BIN),
      k_obj(CONFIG_DEFAULT_K_OBJ),
      k_obj_bd(CONFIG_DEFAULT_K_OBJ_BD),
      v_cc(CONFIG_DEFAULT_V_CC),
      v_cxx(CONFIG_DEFAULT_V_CXX),
      v_cflag(CONFIG_DEFAULT_V_CFLAG),
      v_cxxflag(CONFIG_DEFAULT_V_CXXFLAG),
      v_bd(CONFIG_DEFAULT_V_BD) {}

string Config::make(const string &s, int i) const {
    char tmp[BUFSIZ];
    if (i == -1) {
        // do not show %d
        string k_bin2 = s;
        auto it = k_bin2.find("%d");
        assert(it != k_bin2.npos);
        k_bin2.erase(it, 2);
        sprintf(tmp, k_bin2.c_str(), i);
    } else {
        sprintf(tmp, s.c_str(), i);
    }
    return string(tmp);
}

string Config::make_bin(int i) const { return make(k_bin, i); }

string Config::make_obj(int i) const { return make(k_obj, i); }

string Config::make_obj_bd(int i) const { return make(k_obj_bd, i); }

// skip ' ' and '\t'
static char *skip_blank(char *s) {
    while (*s && isblank(*s)) {
        ++s;
    }
    return s;
}

CONFIG_TYPE parse_tag(char *s, char **tag, int *tag_len) {
    s = skip_blank(s);
    if (*s == '\n') {
        return CT_EMPTY;
    }
    if (*s == '[') {
        s = skip_blank(s + 1);
        *tag = s;
        while (*s && !isspace(*s) && *s != ']') {
            ++s;
        }
        *tag_len = s - *tag;
        s = skip_blank(s);
        if (*s == ']') {
            return CT_TAG;
        } else {
            return CT_ERROR_TAG_MISSING_RIGHT_BRACKET;
        }
    }
    return CT_NOT_TAG;
}

CONFIG_TYPE parse_kv(char *s, char **k, int *k_len, char **v, int *v_len) {
    s = skip_blank(s);
    if (*s == '\n') {
        return CT_EMPTY;
    }
    *k = s;
    while (*s && !isspace(*s) && *s != '=') {
        ++s;
    }
    *k_len = s - *k;

    while (*s && *s != '=') {
        ++s;
    };

    if (*s == '=') {
        s = skip_blank(s + 1);
        *v = s;
        char *p;  // last non-space character
        while (*s) {
            if (!isspace(*s)) {
                p = s;
            }
            ++s;
        }
        *v_len = p - *v + 1;
        return CT_KV;
    } else {
        return *s ? CT_ERROR_KV : CT_K;
    }
}

int read_config(const char *filename, Config *cfg) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Can't open file \"%s\" @%s line %d\n", filename,
                __func__, __LINE__);
        exit(EXIT_FAILURE);
    }

    int ret = 0, line_num = 1;
    char buff[BUFSIZ], *p;
    CONFIG_STATE state = CS_UNKNOWN;
    while (!feof(fp)) {
        if ((p = fgets(buff, BUFSIZ, fp)) == NULL) {
            break;
        }

        char *tag;
        int tag_len;
        CONFIG_TYPE ctype = parse_tag(p, &tag, &tag_len);

        if (ctype == CT_TAG) {
            tag[tag_len] = '\0';
            if (strncmp(tag, "compiler", 8) == 0) {
                state = CS_COMPILER;
            } else if (strncmp(tag, "executable", 10) == 0) {
                state = CS_EXECUTABLE;
            } else if (strncmp(tag, "ignore", 6) == 0) {
                state = CS_IGNORE;
            } else {
                fprintf(stderr,
                        "Unknown tag \"%s\" in \"ascan.conf\" at line %d\n",
                        tag, line_num);
                state = CS_UNKNOWN;
            }
        } else if (ctype == CT_ERROR_TAG_MISSING_RIGHT_BRACKET) {
            fprintf(stderr, "\']\' is expected in \"ascan.conf\" at line %d\n",
                    line_num);
        } else if (ctype == CT_NOT_TAG) {
            char *k, *v;
            int k_len, v_len;
            ctype = parse_kv(p, &k, &k_len, &v, &v_len);

            if (ctype == CT_KV) {
                k[k_len] = '\0';
                v[v_len] = '\0';
                if (state == CS_COMPILER) {
                    if (strcmp(k, "cc") == 0) {
                        cfg->v_cc = v;
                    } else if (strcmp(k, "cxx") == 0) {
                        cfg->v_cxx = v;
                    } else if (strcmp(k, "cflag") == 0) {
                        cfg->v_cflag = v;
                    } else if (strcmp(k, "cxxflag") == 0) {
                        cfg->v_cxxflag = v;
                    } else {
                        fprintf(
                            stderr,
                            "Unknown key \"%s\" in \"ascan.conf\" at line %d\n",
                            k, line_num);
                    }
                } else if (state == CS_EXECUTABLE) {
                    cfg->executable[string(k)] = string(v);
                } else if (state == CS_IGNORE) {
                    fprintf(stderr,
                            "Wrong ignore line in \"ascan.conf\" at line %d\n",
                            line_num);
                }
            } else if (ctype == CT_K) {
                k[k_len] = '\0';
                if (state == CS_IGNORE) {
                    cfg->ignores.push_back(string(k));
                } else {
                    fprintf(stderr,
                            "Key-value line is expected in \"ascan.conf\" at "
                            "line %d\n",
                            line_num);
                }
            } else if (ctype == CT_ERROR_KV) {
                fprintf(stderr,
                        "Wrong key-value line in \"ascan.conf\" at line %d\n",
                        line_num);
            }
        }

        ++line_num;
    }

    fclose(fp);

    return ret;
}