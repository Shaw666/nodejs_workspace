#include <string.h>
#include "http_util.h"

int encodeURIComponent(const char *input, char *output, int output_len)
{
    const char no_encode[] = "-_.!~*'()abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    if (!input || !output) { return 0; };
    int len = 0;
    char *s = output;
    while (input[0] != 0 && len < output_len) {
        char ch = input[0];
        input++;
        if (strchr(no_encode, ch)) {
            output[0] = ch;
            output++;
            len++;
            output[0] = 0;
        } else {
            if (len > output_len - 3) {
                return -1;
            }
            char ch_low  = ch & 0x0F;
            char ch_high = (ch & 0xF0) >> 4;
            if (ch_low < 10)  {
                ch_low   = ch_low + '0';
            } else {
                ch_low   = ch_low  - 10 + 'A';
            };
            if (ch_high < 10)  {
                ch_high   = ch_high + '0';
            } else {
                ch_high   = ch_high  - 10 + 'A';
            };
            output[0] = '%';
            output[1] = ch_high;
            output[2] = ch_low;

            output  = output + 3;
            len = len + 3;
        }
    }
    output[0] = 0;
    return len;
}

int decodeURIComponent(const char *input, char *output, int output_len)
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


