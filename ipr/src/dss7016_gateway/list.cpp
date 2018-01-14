#include "list.h"

List *list_create() {
    return (List *)calloc(1, sizeof(List));
}

void list_destroy(List *list) {
    LIST_FOREACH(list, first, next, cur) {
        if (cur->prev) {
            free(cur->prev);
        }
    }

    free(list->last);
    free(list);
}

void list_clear(List *list) {
    LIST_FOREACH(list, first, next, cur) {
        if (cur->value) {
            free(cur->value);
        }
    }
}

void list_clear_destroy(List *list) {
    list_clear(list);
    list_destroy(list);
}

void list_push(List *list, void *value) {
    List_Node *node = (List_Node *)calloc(1, sizeof(List_Node));
    if (node == NULL) {
        return;
    }

    node->value = value;

    if (list->last == NULL) {
        list->first = node;
        list->last = node;
    } else {
        list->last->next = node;
        node->prev = list->last;
        list->last = node;
    }

    list->count++;
}

void *list_pop(List *list) {
    List_Node *node = list->last;
    return node != NULL? list_remove(list, node):NULL;
}

void list_unshift(List *list, void *value) {
    List_Node *node = (List_Node *)calloc(1, sizeof(List_Node));
    if (node == NULL) {
        return;
    }

    node->value = value;

    if (list->first == NULL) {
        list->first = node;
        list->last = node;
    } else {
        node->next = list->first;
        list->first->prev = node;
        list->first = node;
    }

    list->count++;
}

void *list_shift(List *list) {
    List_Node *node = list->first;
    return node != NULL? list_remove(list, node):NULL;
}

void *list_remove(List *list, List_Node *node) {
    void *result = NULL;

    if ((list->first == NULL) && (list->last == NULL)) {
        return NULL;
    }
    
    if (node == NULL) {
        return NULL;
    }

    if (node == list->first && node == list->last) {
        list->first = NULL;
        list->last = NULL;
    } else if (node == list->first) {
        list->first = node->next;
        if (list->first == NULL) {
            return NULL;
        }
        list->first->prev = NULL;
    } else if (node == list->last) {
        list->last = node->prev;
        if (list->last == NULL) {
            return NULL;
        }
        list->last->next = NULL;
    } else {
        List_Node *after = node->next;
        List_Node *before = node->prev;
        after->prev = before;
        before->next = after;
    }

    list->count--;
    result = node->value;
    free(node);

    return result;
}
