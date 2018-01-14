#ifndef VSYNC_H
#define VSYNC_H

#include <stdlib.h>
#include "tobject.h"
#include "things.h"

class VSync {
 private:
    Things *_t;

 public:
    TObject *root;
    VSync(Things *t);
    ~VSync();
    TObject *sync_with(const char *tid);
    void unsync(const char *tid);
    TObject *get(const char *tid);
    TObject *update_with(const char *tid, char *path, char *json);
    TObject *set_with(char *tid, char *json);
    void sync();
    void pack();
    void process_var_message(T_POST *post);
};

#endif
