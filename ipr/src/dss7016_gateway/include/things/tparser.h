/*******************************************
  JNode lib <sdkver>
  copyright (c) shenzhen conwin tech. ltd.
  2015-4-1     gxy
*********************************************/
#ifndef T_PARSER_H
#define T_PARSER_H

#include "tobject.h"

/*sdk*/#define STX  2    //0x02
/*sdk*/#define ETX  3    //0x03
/*sdk*/#define GS   29   //0x1D
/*sdk*/#define US   31   //0x1F
/*sdk*/#define CR   0x0D  
/*sdk*/#define LF   0x0A
/*sdk*/#define TAB  0x09
/*sdk*/
/*sdk*/typedef enum {
/*sdk*/    T_PARSE_SPACE = 0,
/*sdk*/    T_PARSE_OBJECT
/*sdk*/} T_PARSE_STATE;
/*sdk*/
/*sdk*//* extern TObject *parse(char* code); */
/*sdk*//* extern void parse_skip_space(char **code); */
/*sdk*//* extern char* parse_property_name(char **code); */
/*sdk*//* extern TObject *parse_boolean(char **code); */
/*sdk*//* extern TObject *parse_null(char **code); */
/*sdk*//* extern TObject *parse_string(char **code); */
/*sdk*//* extern TObject *parse_array(char **code); */
/*sdk*//* extern TObject *parse_number(char **code); */
/*sdk*//* extern TObject *parse_object(char **code); */
/*sdk*//* extern TObject *parse_value(char **code); */
/*sdk*//* extern TObject *parse_array(char **code); */
/*sdk*//* extern void parse_block(char **code); */
/*sdk*//* extern TObject *parse(char **code); */
/*sdk*/
extern TObject *parse_json(const char *code);
extern TObject *parse_json(char **code);
/*sdk*/extern void parse_skip_space(char **code);
/*sdk*/extern TObject *parse_value(char **code);

#endif
