#ifndef CWLIB_H
#define CWLIB_H

#include <time.h>
#include <stdlib.h>


#ifdef AVR
char* index(const char* s, int c);
char* rindex(const char* s, const char c);  // changed to strrchr
#endif

void tick_to_time_str(time_t *time, char *buf, int len);
time_t str_to_time_tick(char *time);

extern char* get_token_first(char* buf, const char deli);
extern char* get_token_next(char* buf, const char deli);
extern char* get_token_rest(char* buf);

#endif
