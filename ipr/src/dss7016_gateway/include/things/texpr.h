#ifndef TEXPR_H
#define TEXPR_H

#include "tobject.h"

#define TOKENS_MAX 200

typedef enum {
    T_TOKEN_VALUE = 0,
    T_TOKEN_OPERATOR,
    T_TOKEN_NAME,
    T_TOKEN_COMMENT,
    T_TOKEN_LEFT_BRACKET,
    T_TOKEN_RIGHT_BRACKET
} T_TOKEN_TYPE;


typedef enum {
    OP_NONE = -1,
    OP_L_BRACKET = 0,
    OP_R_BRACKET,
    OP_DOT,     // '.'  : : this.calc_dot},
    OP_POWER,   // '^'  : : this.calc_power},
    OP_TIMES,   // '*'  : : this.calc_times},
    OP_MOD,     // '%'  : : this.calc_div_int},
    OP_DIV,     // '/'  : : this.calc_div},
    OP_PLUS,    // '+'  : : this.calc_plus},
    OP_SUB,     // '-'  : : this.calc_sub},
    OP_NEG,     // '-n' : : this.calc_neg},
    OP_BOOL_EQ, // '==' : : this.calc_bool_eq},
    OP_BOOL_NE, // '!=' : : this.calc_bool_ne},
    OP_BOOL_GT, // '>'  : : this.calc_bool_gt},
    OP_BOOL_LT, // '<'  : : this.calc_bool_lt},
    OP_BOOL_GE, // '>=' : : this.calc_bool_ge},
    OP_BOOL_LE, // '<=' : : this.calc_bool_le},
    OP_BOOL_AND,// '&&' : : this.calc_bool_and},
    OP_BOOL_OR, // '||' : : this.calc_bool_or},
} T_OPERATOR;

typedef struct {
    T_TOKEN_TYPE type;
    TObject *value;
    char *name;
    T_OPERATOR op;
} T_TOKEN;

typedef T_TOKEN *(Oper_calc)(T_TOKEN *params[], int n, void *context);

typedef enum {
    T_OPER_LTR = 0,
    T_OPER_RTL
} T_OPER_DIR;


typedef struct {
    int priority;
    T_OPER_DIR dir;
    int oper_nums;
    Oper_calc *calc;
} T_OPER_REC;

extern T_TOKEN *calc_dot          (T_TOKEN *params[], int n, void *context);
extern T_TOKEN *calc_power        (T_TOKEN *params[], int n, void *context);
extern T_TOKEN *calc_times        (T_TOKEN *params[], int n, void *context);
extern T_TOKEN *calc_mod          (T_TOKEN *params[], int n, void *context);
extern T_TOKEN *calc_div          (T_TOKEN *params[], int n, void *context);
extern T_TOKEN *calc_plus         (T_TOKEN *params[], int n, void *context);
extern T_TOKEN *calc_sub          (T_TOKEN *params[], int n, void *context);
extern T_TOKEN *calc_neg          (T_TOKEN *params[], int n, void *context);
extern T_TOKEN *calc_bool_eq      (T_TOKEN *params[], int n, void *context);
extern T_TOKEN *calc_bool_ne      (T_TOKEN *params[], int n, void *context);
extern T_TOKEN *calc_bool_gt      (T_TOKEN *params[], int n, void *context);
extern T_TOKEN *calc_bool_lt      (T_TOKEN *params[], int n, void *context);
extern T_TOKEN *calc_bool_ge      (T_TOKEN *params[], int n, void *context);
extern T_TOKEN *calc_bool_le      (T_TOKEN *params[], int n, void *context);
extern T_TOKEN *calc_string_match (T_TOKEN *params[], int n, void *context);
extern T_TOKEN *calc_bool_and     (T_TOKEN *params[], int n, void *context);
extern T_TOKEN *calc_bool_or      (T_TOKEN *params[], int n, void *context);
extern T_TOKEN *calc_array_in     (T_TOKEN *params[], int n, void *context);

static T_OPER_REC  operators[] = {
    /* '(' */ {1, T_OPER_LTR, 0, NULL              },
    /* ')' */ {1, T_OPER_LTR, 0, NULL              },
    /* '.' */ {1, T_OPER_LTR, 2, calc_dot          },
    /* '^' */ {3, T_OPER_RTL, 2, calc_power        },
    /* '*' */ {3, T_OPER_LTR, 2, calc_times        },
    /* '%' */ {3, T_OPER_LTR, 2, calc_mod          },
    /* '/' */ {3, T_OPER_LTR, 2, calc_div          },
    /* '+' */ {4, T_OPER_LTR, 2, calc_plus         },
    /* '-' */ {4, T_OPER_LTR, 2, calc_sub          },
    /* '-n'*/ {4, T_OPER_RTL, 1, calc_neg          },
    /* '=='*/ {7, T_OPER_LTR, 2, calc_bool_eq      },
    /* '!='*/ {1, T_OPER_LTR, 2, calc_bool_ne      },
    /* '>' */ {6, T_OPER_LTR, 2, calc_bool_gt      },
    /* '<' */ {6, T_OPER_LTR, 2, calc_bool_lt      },
    /* '>='*/ {6, T_OPER_LTR, 2, calc_bool_ge      },
    /* '<='*/ {6, T_OPER_LTR, 2, calc_bool_le      },
    /* '&&'*/ {11,T_OPER_LTR, 2, calc_bool_and     },
    /* '||'*/ {12,T_OPER_LTR, 2, calc_bool_or      },
};

class TExpr {
 private:
    T_TOKEN *tokens[TOKENS_MAX];  // values moved to rpn queue, dont free this
    int token_len;

    T_TOKEN *rpn[TOKENS_MAX];
    int rpn_len;

    int is_name_char(char ch);
    int is_operator_prefix(char ch);
    void pass1(char **code);
    void pass2();
    TObject *pass3();
    T_TOKEN *parse_token_string(char **code);
    T_TOKEN *parse_token_number(char **code);
    T_TOKEN *parse_token_name(char **code);
    T_TOKEN *parse_token_operator(char **code);
    void free_token(T_TOKEN **tokens);
    void pack();
    T_TOKEN *clone_token(T_TOKEN *token);
 public:
    TObject *context;

    TExpr(char *code);
    ~TExpr();
    TObject *eval(TObject *context);
};

#endif
