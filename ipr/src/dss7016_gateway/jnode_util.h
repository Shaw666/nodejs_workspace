#ifndef JNODE_UTIL_H
#define JNODE_UTIL_H

#include "things/tobject.h"
#include "things/cwlib.h"

extern TObject *load_json_file(const char *filename);
extern void     write_json_file(const char *filename, TObject *obj);
extern int      load_file(const char *filename, char *buf, int len);
extern void     write_object_to_file(FILE *f, TObject *obj, int pad, int delta, char *expand_with_prefix);
extern void update_json_file(const char *filename, const char *path, TObject *update);

extern char *full_path(const char *file);

extern char *ip_of(const char *iface);
extern int decode_url(const char *input, char *output, int output_len);

extern TObject *pair_to_object(char *buf, const char *d1, const char *d2, const int do_trim);

#endif
