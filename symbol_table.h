#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "utils/dict.h"

typedef struct symbol_table_t {
    dict_t dict;
    struct symbol_table_t * parent;
} symbol_table_t;

#endif
