#include <assert.h>
#include <stdlib.h>
#include "symbol_table.h"
#include "type.h"


symbol_table_t * create_symbol_table(symbol_table_t * parent) {
    symbol_table_t * table = (symbol_table_t *) malloc(sizeof(symbol_table_t));
    table->dict = create_dict();
    table->parent = parent;
    return table;
}

static void delete_type_adapter(void * type) {
    delete_type((type_t *) type);
}

void delete_symbol_table(symbol_table_t * table) {
    symbol_table_t * parent_table = table->parent;
    dict_for_each(&(table->dict), delete_type_adapter);
    free(table);
    if (parent_table != NULL) delete_symbol_table(parent_table);
}

void add_symbol(symbol_table_t * table, const char * identifier, type_t * type) {
    assert(type != NULL);
    dict_add(&(table->dict), identifier, type);
}

type_t * get_symbol_type(symbol_table_t * table, const char * identifier) {
    for (; table != NULL; table = table->parent) {
        struct avl_node_t * node = dict_get(table->dict, identifier);
        if (node) return (type_t *) node->val;
    }
    return NULL;
}
