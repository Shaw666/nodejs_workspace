#ifndef TVAREX_H
#define TVAREX_H

#include <stdlib.h>
#include "tobject.h"
#include "things.h"

struct VAREX_REC {
    char *addr;
    TObject *var;
};

class VarEx {
 private:
    Things *_t;
    VAREX_REC *_vars;
    int _var_cnt;

 public:
    VarEx(Things *t);
    ~VarEx();
    void push_vars(int delta);
};

#endif
