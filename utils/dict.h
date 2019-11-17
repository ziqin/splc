#ifndef UTILS_DICT_H
#define UTILS_DICT_H

typedef struct avl_node_t {
    char * key;
    void * val;
    struct avl_node_t * left, * right;
    int height;
} avl_node_t;

typedef struct dict_t {
    struct avl_node_t * root;
    unsigned size;
} dict_t;

dict_t create_dict(void);
void clear_dict(dict_t * dict);
struct avl_node_t * dict_add(dict_t * dict, const char * key, void * value);
struct avl_node_t * dict_get(dict_t dict, const char * key);
void dict_set(dict_t * dict, const char * key, void * value);
void dict_delete(dict_t * dict, const char * key);
void dict_for_each(dict_t * tree, void (*action)(void *));

#endif
