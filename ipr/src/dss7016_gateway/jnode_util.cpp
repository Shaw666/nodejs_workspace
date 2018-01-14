#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <things/tobject.h>
#include <things/tparser.h>
#include "jnode_util.h"
#include "jnode_env.h"

int opt_ident             = 4;
int opt_quote_value       = 0;  // 1 - single   2 - double
int opt_uppercase_name    = 0;
int opt_update_only       = 0;
char opt_dot_replace_with = '.';

void write_object_to_file(FILE *f, TObject *obj, int pad, int delta, char *expand_with_prefix)
{
    int i, n;
    if (obj->type() == T_OBJECT) {
        if (expand_with_prefix == NULL) {
            for (i = 0; i < pad; i++) ; fprintf(f, "{");
            if (opt_ident > 0) {fprintf(f, "\n"); };
        }
        int cnt = obj->member_cnt();
        for (n = 0; n < cnt; n++) {
            if (expand_with_prefix == NULL) {
                for (i = 0; i < pad + delta; i++) { fprintf(f, " "); };
                if (opt_quote_value == 1) {
                    fprintf(f, "'");
                } else {
                    fprintf(f, "\"");
                }
                fprintf(f, "%s", obj->member_name(n));
                if (opt_quote_value == 1) {
                    fprintf(f, "':");
                } else {
                    fprintf(f, "\":");
                }
                write_object_to_file(f, obj->member(n), pad + delta, 
                                     delta, NULL);
                if (n < cnt - 1) { fprintf(f, ","); };
                if (opt_ident > 0) {fprintf(f, "\n"); };
            } else {
                char *s_name = obj->member_name(n);
                char *next_prefix =
                    (char*)malloc(strlen(expand_with_prefix)
                                  + 1
                                  + strlen(s_name) + 1);
                sprintf(next_prefix, "%s%c%s", expand_with_prefix, opt_dot_replace_with, s_name);
                write_object_to_file(f, obj->member(n), pad + delta,
                                     delta, next_prefix);
                free(next_prefix);
            }
        };
        if (expand_with_prefix == NULL) {
            for (i = 0; i < pad; i++) { fprintf(f, " "); }; fprintf(f, "}");
        }
    } else {
        if (expand_with_prefix != NULL) {
            if (opt_uppercase_name) {
                for (int i = 0; i < strlen(expand_with_prefix); i++) {
                    expand_with_prefix[i] = toupper(expand_with_prefix[i]);
                }
            }
            fprintf(f, "%s=", expand_with_prefix);
        }
        if (opt_quote_value || (obj->type() == T_STRING)) {
            if (opt_quote_value == 1) {
                fprintf(f, "'");
            } else if (opt_quote_value == 2) {
                fprintf(f, "\"");
            } else if (obj->type() == T_STRING) {
                fprintf(f, "\"");
            }
        };
        fprintf(f, "%s", obj->to_string());
        if (opt_quote_value || (obj->type() == T_STRING)) {
            if (opt_quote_value == 1) {
                fprintf(f, "'");
            } else if (opt_quote_value == 2) {
                fprintf(f, "\"");
            } else if (obj->type() == T_STRING) {
                fprintf(f, "\"");
            }
        };
        if (expand_with_prefix != NULL) {
            fprintf(f, "\n");
        }
    }
};

int load_file(const char *filename, char *buf, int len)
{
    int fd;
    fd = open(filename, O_RDONLY);
    if (fd < 0) { return -1; }
    int ret = read(fd, buf, len);
    close(fd);
    if (ret <= 0) {
        return -2;
    } else {
        return ret;
    };
}


TObject *load_json_file(const char *filename)
{
    int fd;
    fd = open(filename, O_RDONLY);
    if (fd < 0) { return NULL; }
    off_t size = lseek(fd, 0, SEEK_END);
    if (size < 0) { return NULL; };
    lseek(fd, 0, 0);
    char *buf = (char*)malloc(size + 1);
    memset(buf, 0, size);
    int ret = read(fd, buf, size);
    if (ret <= 0) {
        close(fd);
        free(buf);
        return NULL;
    };
    TObject *result = parse_json(buf);
    free(buf);
    close(fd);
    return result;
};

void write_json_file(const char *filename, TObject *obj)
{
    if (!obj || !filename) { return; };
    FILE *f = fopen(filename, "w+");
    write_object_to_file(f, obj, 0, 4, NULL);
    fclose(f);
};

void update_json_file(const char *filename, const char *path,
                      TObject *update)
{
    if (!filename || !update) { return; };
    TObject *json = NULL;
    if (path) {
        json = load_json_file(filename);
        if (!json) { return; };
        if (!json->has_member(path)) {
            json->new_member(path);
        }
        json->member(path)->assign(update);
    } else {
        json = update->clone();
    }
    write_json_file(filename, json);
    delete json;
};

char *full_path(const char *file)
{
    if (!file) { return NULL; };
    if (file[0] == '~') {
        char *home = getenv("HOME");
        char *ret = (char*)malloc(strlen(home) + strlen(file) + 1);
        strcpy(ret, home);
        strcat(ret, &file[1]);
        return ret;
    } if (file[0] != '/') {
        char wd[200];
        getcwd(wd, sizeof(wd));
        char *ret = (char*)malloc(strlen(wd) + strlen(file) + 2);
        strcpy(ret, wd);
        ret = strcat(ret, "/");
        ret = strcat(ret, file);
        return ret;
    } else {
        return strdup(file);
    }
};


char *__ip = NULL;
char *ip_of(const char *iface)
{
    if (!iface) { return NULL; };
    struct ifreq ifr;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) { return NULL; };
    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;
    /* I want IP address attached to "eth0" */
    strncpy(ifr.ifr_name, iface, IFNAMSIZ-1);
    int ret = ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);
    if (__ip) { free(__ip); __ip = NULL; };
    if (ret < 0) {
        return NULL;
    } else {
        __ip = strdup(inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)
                                ->sin_addr));
        return __ip;
    };
}

int decode_url(const char *input, char *output, int output_len)
{
    if (!input || !output) { return 0; };
    int len = 0;
    while (input[0] != 0 && len < output_len) {
        char ch = input[0];
        if (ch != '%') {
            output[0] = ch;
            input++; output++; len++;
        } else {
            char ch1 = input[1]; char ch2 = input[2];
            if (ch1 >= '0' && ch1 <= '9') {
                ch1 = ch1 - '0';
            } else if (ch1 >= 'A' && ch1 <= 'F') {
                ch1 = ch1 - 'A' + 10;
            }
            if (ch2 >= '0' && ch2 <= '9') {
                ch2 = ch2 - '0';
            } else if (ch2 >= 'A' && ch2 <= 'F') {
                ch2 = ch2 - 'A' + 10;
            }
            ch = ch1 * 0x10 + ch2;
            output[0] = ch;
            output++; len++;
            input = input + 3;
        }
    }
    output[0] = 0;
}

// TObject *pair_to_object(char *buf, const char *d1, const char *d2, const int do_trim)
// {
//     if (!buf) { return NULL; };
//     TObject *result = new TObject((char*)NULL, NULL);
//     char *token;
//     while ((token = strsep(&buf, d1)) != 0) {
//         char *name = strsep(&token, d2);
//         char *value = token;
//         if (do_trim) {
//             name = trim(name);  value = trim(value);
//         }
//         if (name && value) {
//             result->new_member(name)->set(value);
//         }
//     }
//     return result;
// };

