/*******************************************
  JNode lib <sdkver>
  copyright (c) shenzhen conwin tech. ltd.
  2015-4-1     gxy
*********************************************/
#ifndef TOBJECT_H
#define TOBJECT_H

//#define T_DEBUG
#ifdef AVR
#include <avr/pgmspace.h>
#define MEMORY_LOCATION PROGMEM
#else
#define MEMORY_LOCATION 
#endif
#include <stdlib.h>
#include <inttypes.h>


#define DELI_PATH '.'

typedef enum {
    BY_NAME = 0,
    BY_PATH,
} Name_mode;


typedef enum {
    T_ANY_VALUE = -1,
    T_UNDEFINE = 0,
    T_NULL_VALUE,
    T_BOOL,
    T_INT,
    T_FLOAT,
    T_STRING,
    T_ARRAY,
    T_OBJECT
} OBJECT_TYPE;

typedef enum {
    T_FALSE = 0,
    T_TRUE
} T_BOOLEAN;

static char STR_INTERNAL_ERROR[] = "internal error";
static char STR_UNDEFINED[]      = "undefined";
static char STR_NULL[]           = "null";
static char STR_UNNAMED[]        = "unnamed";
static char STR_TYPE_BOOL[]      = "boolean";
static char STR_TYPE_INT[]       = "int";
static char STR_TYPE_FLOAT[]     = "float";
static char STR_TYPE_STRING[]    = "string";
static char STR_TYPE_ARRAY[]     = "array";
static char STR_TYPE_OBJECT[]    = "object";
static char STR_TRUE[]           = "true";
static char STR_FALSE[]          = "false";

class TObject;
/*sdk*/TObject *obj_add(TObject *o1, TObject *o2);
/*sdk*/TObject *obj_times(TObject *o1, TObject *o2);
/*sdk*/TObject *obj_mod(TObject *o1, TObject *o2);
/*sdk*/TObject *obj_div(TObject *o1, TObject *o2);
/*sdk*/TObject *obj_sub(TObject *o1, TObject *o2);
/*sdk*/TObject *obj_neg          (TObject *o1);
/*sdk*/TObject *obj_bool_eq      (TObject *o1, TObject *o2);
/*sdk*/TObject *obj_bool_ne      (TObject *o1, TObject *o2);
/*sdk*/TObject *obj_bool_gt      (TObject *o1, TObject *o2);
/*sdk*/TObject *obj_bool_lt      (TObject *o1, TObject *o2);
/*sdk*/TObject *obj_bool_ge      (TObject *o1, TObject *o2);
/*sdk*/TObject *obj_bool_le      (TObject *o1, TObject *o2);
/*sdk*/TObject *obj_bool_and     (TObject *o1, TObject *o2);
/*sdk*/TObject *obj_bool_or      (TObject *o1, TObject *o2);

typedef void (*TObject_callback)(TObject *value, void *context);

typedef enum {
/*sdk*/T_FLAGS_NONE      = 0x00,
/*sdk*/T_FLAGS_READONLY  = 0x01,
/*sdk*/T_FLAGS_CHANGED   = 0x02,
/*sdk*/T_FLAGS_NEED_PACK = 0x04,
/*sdk*/T_FLAGS_REALTIME  = 0x08
} T_FLAGS;

typedef enum {
/*sdk*/T_ON_BEFORE_CHANGE = 0,
/*sdk*/T_ON_AFTER_CHANGE
} T_VAR_EVENT;

typedef struct {
/*sdk*/T_VAR_EVENT event;
/*sdk*/TObject_callback callback;
/*sdk*/void *context;
} T_CALLBACK;


typedef struct {
/*sdk*/TObject *value;
/*sdk*/char *name;
} MemberRecord;


typedef union {
/*sdk*/char *str;
/*sdk*/long i;
/*sdk*/float f;
/*sdk*/T_BOOLEAN  b;
/*sdk*/MemberRecord *members;
} TObjectData;


/*sdk*/
class TObject {
/*sdk*/ protected:
/*sdk*/    int          _ref_cnt;
/*sdk*/    OBJECT_TYPE  _type;
/*sdk*/    char        *_value_string;
/*sdk*/    int          _member_array_len;
/*sdk*/    TObjectData  _value;
/*sdk*/    uint8_t      _flags;
/*sdk*/    
/*sdk*/    void release();
/*sdk*/    size_t str_len_with_escape(char *str);
/*sdk*/    size_t calc_to_string_length(int escape, T_FLAGS flags);
/*sdk*/    void _init();
/*sdk*/    void _free_string(char **str);
/*sdk*/    void _call_before_change();
/*sdk*/    void _call_after_change();
/*sdk*/#ifdef T_DEBUG   
/*sdk*/    void _update_memory_usage(int size);
/*sdk*/#endif
/*sdk*/ protected:
/*sdk*/    int _callback_cnt;
/*sdk*/    T_CALLBACK *_event_callbacks;
/*sdk*/#ifdef T_DEBUG
/*sdk*/    int memory_alloc;
/*sdk*/    int memory_release;
/*sdk*/    int memory_usage;
/*sdk*/#endif
/*sdk*/
/*sdk*/    void ref(TObject *obj);
/*sdk*/    void unref(TObject *obj);
/*sdk*/
/*sdk*/    int need_quote();
/*sdk*/
/*sdk*/    int  put_value_string_to_buf(char *buf, size_t len,
/*sdk*/                                 bool escape, T_FLAGS flags);
/*sdk*/    void convert_value_to_string(T_FLAGS flags);
/*sdk*/    size_t size();
/*sdk*/    void on(T_VAR_EVENT event, TObject_callback callback, void *context);
/*sdk*/    TObject *get_by_path(const char *path);
/*sdk*/    TObject *get_by_path(const char *path, Name_mode mode);
/*sdk*/    TObject *assign_by_path(const char *path, TObject *value);
/*sdk*/    int flags_match(T_FLAGS flags);
/*sdk*/    void flags_set(T_FLAGS flags);
/*sdk*/    void flags_clear(T_FLAGS flags);
/*sdk*/    TObject *get_member_value(int index);
/*sdk*/    TObject *create_value(const char *path, Name_mode mode);
/*sdk*/    int find_member_first_level(const char *path, Name_mode mode);
/*sdk*/    uint8_t flags();
 public:

    TObject();
    TObject(const char *value);
    TObject(const char *value, T_FLAGS flags);
    TObject(const T_BOOLEAN value);
    TObject(const T_BOOLEAN value, T_FLAGS flags);
    TObject(const int value);
    TObject(const int value, T_FLAGS flags);
    TObject(const double value);
    TObject(const double value, T_FLAGS flags);
    TObject(const char *member_name, TObject  *value);
    TObject(const char *member_name, TObject  *value, T_FLAGS flags);
    ~TObject();
    TObject *set();
    TObject *set(const char *value);
    TObject *set(const char *value, T_FLAGS flags);
    TObject *set(const T_BOOLEAN  value);
    TObject *set(const T_BOOLEAN  value, T_FLAGS flags);
    TObject *set(const int value);
    TObject *set(const int value, T_FLAGS flags);
    TObject *set(const long  value);
    TObject *set(const long  value, T_FLAGS flags);
    TObject *set(const double  value);
    TObject *set(const double  value, T_FLAGS flags);
    TObject *set(const char *member_name, TObject *value);
    TObject *set(const char *member_name, TObject *value, T_FLAGS flags);

    void pack();
    void del(const char *member_name);
    void del(const int index);

    TObject *push(TObject *value);
    TObject *push(char *value);

    char* type_name();
    char* to_string(const char *path);
    char* to_string();
    char* to_string(T_FLAGS flags);
    char to_char();
    long  to_int();
    T_BOOLEAN to_bool();
    double to_float();

    void commit();
    int changed();
    void to_int(int *i);

    int member_cnt();
    TObject *update_with(TObject *delta, int auto_insert);
    TObject *update_with(TObject *delta, int auto_insert,
                         Name_mode mode);
    TObject *clone();
    TObject *clone(char *jnames);

    TObject *assign(TObject *value);
    OBJECT_TYPE type();

    int member_count();
    TObject *member(const char *name);
    TObject *member(const char *path, Name_mode mode);
    char *member_name(int index);
    TObject *member(int index);

    TObject *new_member(const char *name);
    TObject *new_member(const char *path, Name_mode mode);
    int has_member(const char *name);
    int has_member(const char *name, OBJECT_TYPE type);
    int has_member(const char *path, OBJECT_TYPE type, Name_mode mode);
};
#endif
