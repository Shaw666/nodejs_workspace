#ifndef HTTP_UTIL_H
#define HTTP_UTIL_H

extern int encodeURIComponent(const char *input, char *output, int output_len);

extern int decodeURIComponent(const char *input, char *output, int output_len);

#endif
