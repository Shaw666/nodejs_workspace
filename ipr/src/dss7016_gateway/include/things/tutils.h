#ifndef TJS_UTILS_H
#define TJS_UTILS_H

#include <stdlib.h>

extern int tx_index(const char *dst, const char ch);
extern size_t tx_strcpy(char *dst, const char *src);
extern size_t tx_strncpy(char * dst, const char * src, size_t n);
extern size_t tx_strlen(const char *src);
extern int  tx_strcmp(const char *s1, const char *s2);
extern int tx_strncmp(const char *s1, const char *s2, size_t len);
extern short int tx_int_to_str(char *buf, long i);
extern short int tx_float_to_str(char *buf, double f, int n);
#endif
