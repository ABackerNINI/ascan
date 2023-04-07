#include "config.h"
#include "common.h"
#include "debug.h"
#include "parser.h"
#include <cassert>
#include <iostream>
#include <string.h>

using namespace std;

Config::Config()
    : output(CONFIG_DEFAULT_OUTPUT), k_bin(CONFIG_BIN), k_obj(CONFIG_OBJ),
      k_obj_bd(CONFIG_OBJ_BD) {
    configs[CONFIG_CC] = CONFIG_DEFAULT_V_CC;
    configs[CONFIG_CXX] = CONFIG_DEFAULT_V_CXX;
    configs[CONFIG_CFLAGS] = CONFIG_DEFAULT_V_CFLAGS;
    configs[CONFIG_CXXFLAGS] = CONFIG_DEFAULT_V_CXXFLAGS;
    configs[CONFIG_LFLAGS] = CONFIG_DEFAULT_V_LFLAGS;
    configs[CONFIG_BD] = CONFIG_DEFAULT_V_BD;

    // These are variable names in the Makefile, thus can NOT contain spaces
    check_space(CONFIG_CC);
    check_space(CONFIG_CXX);
    check_space(CONFIG_CFLAGS);
    check_space(CONFIG_CXXFLAGS);
    check_space(CONFIG_LFLAGS);

    // These may show in the clean section of the Makefile, thus can NOT contain
    // spaces
    check_space(CONFIG_FILENAME);
    check_space(CONFIG_DEPENDENCIES_FILENAME);
    check_space(CONFIG_DEFAULT_V_BD);
    check_space(CONFIG_BD);
    check_space(CONFIG_BIN);
    check_space(CONFIG_OBJ);
    check_space(CONFIG_OBJ_BD);

    // These may show in the clean section of the Makefile, thus can NOT start
    // with '/'
    check_slash(CONFIG_FILENAME);
    check_slash(CONFIG_DEPENDENCIES_FILENAME);
    check_slash(CONFIG_DEFAULT_V_BD);
    check_slash(CONFIG_BD);
    check_slash(CONFIG_BIN);
    check_slash(CONFIG_OBJ);
    check_slash(CONFIG_OBJ_BD);
}

const std::string &
Config::get_config_value(const std::string &config_name) const {
    auto config = configs.find(config_name);
    if (config == configs.end()) {
        print_error("no such config: \"%s\"", config_name.c_str());
        exit(EXIT_FAILURE);
    }
    return config->second;
}

void Config::set_config(const std::string &config_name,
                        const std::string &config_value) {
    configs[config_name] = config_value;
}

string Config::make_bin(int i) const {
    return make(k_bin, i);
}

string Config::make_obj(int i) const {
    return make(k_obj, i);
}

string Config::make_obj_bd(int i) const {
    return make(k_obj_bd, i);
}

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

void Config::check_space(const std::string &config) const {
    if (contain_space(config)) {
        cerr << "The configuration value \"" << config
             << "\" contains space, which is not allowed!" << endl;
        exit(1);
    }
}

void Config::check_slash(const std::string &config) const {
    if (config.find('/') != config.npos) {
        cerr << "The configuration value \"" << config
             << "\" contains '/', which is not allowed!" << endl;
        exit(1);
    }
}

// enum CONFIG_STATE { CS_COMPILER, CS_EXECUTABLE, CS_IGNORE, CS_UNKNOWN };
//
// enum CONFIG_TYPE {
//     CT_TAG,
//     CT_KV,
//     CT_K,
//     CT_NOT_TAG,
//     CT_EMPTY,
//     CT_ERROR_TAG_MISSING_RIGHT_BRACKET,
//     CT_ERROR_KV
// };
//
// // skip ' ' and '\t'
// static char *skip_blank(char *s) {
//     while (*s && isblank(*s)) {
//         ++s;
//     }
//     return s;
// }
//
// CONFIG_TYPE parse_tag(char *s, char **tag, int *tag_len) {
//     s = skip_blank(s);
//     if (*s == '\n') {
//         return CT_EMPTY;
//     }
//     if (*s == '[') {
//         s = skip_blank(s + 1);
//         *tag = s;
//         while (*s && !isspace(*s) && *s != ']') {
//             ++s;
//         }
//         *tag_len = s - *tag;
//         s = skip_blank(s);
//         if (*s == ']') {
//             return CT_TAG;
//         } else {
//             return CT_ERROR_TAG_MISSING_RIGHT_BRACKET;
//         }
//     }
//     return CT_NOT_TAG;
// }
//
// CONFIG_TYPE parse_kv(char *s, char **k, int *k_len, char **v, int *v_len) {
//     s = skip_blank(s);
//     if (*s == '\n') {
//         return CT_EMPTY;
//     }
//     *k = s;
//     while (*s && !isspace(*s) && *s != '=') {
//         ++s;
//     }
//     *k_len = s - *k;
//
//     while (*s && *s != '=') {
//         ++s;
//     };
//
//     if (*s == '=') {
//         s = skip_blank(s + 1);
//         *v = s;
//         char *p; // last non-space character
//         while (*s) {
//             if (!isspace(*s)) {
//                 p = s;
//             }
//             ++s;
//         }
//         *v_len = p - *v + 1;
//         return CT_KV;
//     } else {
//         return *s ? CT_ERROR_KV : CT_K;
//     }
// }
//
// int read_config(const char *filename, Config *cfg) {
//     FILE *fp = fopen(filename, "r");
//     if (fp == NULL) {
//         fprintf(stderr, "Can't open file \"%s\" @%s line %d\n", filename,
//                 __func__, __LINE__);
//         exit(EXIT_FAILURE);
//     }
//
//     int ret = 0, line_num = 1;
//     char buff[BUFSIZ], *p;
//     CONFIG_STATE state = CS_UNKNOWN;
//     while (!feof(fp)) {
//         if ((p = fgets(buff, BUFSIZ, fp)) == NULL) {
//             break;
//         }
//
//         char *tag;
//         int tag_len;
//         CONFIG_TYPE ctype = parse_tag(p, &tag, &tag_len);
//
//         if (ctype == CT_TAG) {
//             tag[tag_len] = '\0';
//             if (strncmp(tag, "compiler", 8) == 0) {
//                 state = CS_COMPILER;
//             } else if (strncmp(tag, "executable", 10) == 0) {
//                 state = CS_EXECUTABLE;
//             } else if (strncmp(tag, "ignore", 6) == 0) {
//                 state = CS_IGNORE;
//             } else {
//                 fprintf(stderr,
//                         "Unknown tag \"%s\" in \"ascan.conf\" at line %d\n",
//                         tag, line_num);
//                 state = CS_UNKNOWN;
//             }
//         } else if (ctype == CT_ERROR_TAG_MISSING_RIGHT_BRACKET) {
//             fprintf(stderr, "\']\' is expected in \"ascan.conf\" at line
//             %d\n",
//                     line_num);
//         } else if (ctype == CT_NOT_TAG) {
//             char *k, *v;
//             int k_len, v_len;
//             ctype = parse_kv(p, &k, &k_len, &v, &v_len);
//
//             if (ctype == CT_KV) {
//                 k[k_len] = '\0';
//                 v[v_len] = '\0';
//                 if (state == CS_COMPILER) {
//                     if (strcmp(k, "cc") == 0) {
//                         cfg->v_cc = v;
//                     } else if (strcmp(k, "cxx") == 0) {
//                         cfg->v_cxx = v;
//                     } else if (strcmp(k, "cflag") == 0) {
//                         cfg->v_cflags = v;
//                     } else if (strcmp(k, "cxxflag") == 0) {
//                         cfg->v_cxxflags = v;
//                     } else {
//                         fprintf(
//                             stderr,
//                             "Unknown key \"%s\" in \"ascan.conf\" at line
//                             %d\n", k, line_num);
//                     }
//                 } else if (state == CS_EXECUTABLE) {
//                     cfg->executable[string(k)] = string(v);
//                 } else if (state == CS_IGNORE) {
//                     fprintf(stderr,
//                             "Wrong ignore line in \"ascan.conf\" at line
//                             %d\n", line_num);
//                 }
//             } else if (ctype == CT_K) {
//                 k[k_len] = '\0';
//                 if (state == CS_IGNORE) {
//                     cfg->ignores.push_back(string(k));
//                 } else {
//                     fprintf(stderr,
//                             "Key-value line is expected in \"ascan.conf\" at
//                             " "line %d\n", line_num);
//                 }
//             } else if (ctype == CT_ERROR_KV) {
//                 fprintf(stderr,
//                         "Wrong key-value line in \"ascan.conf\" at line
//                         %d\n", line_num);
//             }
//         }
//
//         ++line_num;
//     }
//
//     fclose(fp);
//
//     return ret;
// }
