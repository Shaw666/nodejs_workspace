#ifndef TENV_H
#define TENV_H

#include "tobject.h"
#include "things.h"
#include "texpr.h"

/**
   @brief  管理环境变量的对象
 */

class TEnv {
 public:
    TEnv(Things *t, TObject *sys);
    ~TEnv();

    /**
       @brief select this object
     */
    void select(char *tid);
    
    
};

#endif


