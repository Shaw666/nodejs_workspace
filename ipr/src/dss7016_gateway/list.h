#ifndef _LIST_H
#define _LIST_H

#include <stdlib.h>

struct List_Node;

typedef struct List_Node {
    struct List_Node *next;
    struct List_Node *prev;
    void *value;
} List_Node;

typedef struct List {
    int count;
    List_Node *first;
    List_Node *last;
} List;

List *list_create();
void  list_destroy(List *list);
void  list_clear(List *list);
void  list_clear_destroy(List *list);

#define LIST_COUNT(A) ((A)->count)
#define LIST_FIRST(A) ((A)->first != NULL ? (A)->first->value : NULL)
#define List_LAST(A)  ((A)->last != NULL ? (A)->last->value : NULL)

void list_push(List *list, void *value);
void *list_pop(List *list);

void list_unshift(List *list, void *value);
void *list_shift(List *list);

void *list_remove(List *list, List_Node *node);

#define LIST_FOREACH(L, S, M, V) List_Node *_node = NULL;\
                                 List_Node *V = NULL;\
                                     for(V = _node = L->S; _node != NULL; V = _node = _node->M)

#endif
