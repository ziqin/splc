#ifndef TYPE_H
#define TYPE_H

#include <stdbool.h>
#include "utils/array.h"


enum type_category { PRIMITIVE, ARRAY, STRUCTURE, FUNCTION };

enum primitive_type { INT, FLOAT, CHAR, AUTO };

struct type_t;

typedef struct {
    struct type_t * base;
    unsigned size;
} array_info;

typedef struct {
    struct type_t * type;
    char * name;
} field_info;

USE_ARRAY(field_info);
typedef struct {
    array_t(field_info) fields;
} struct_info;

typedef struct type_t * type_t_ptr;
USE_ARRAY(type_t_ptr);
typedef struct {
    struct type_t * returned;
    array_t(type_t_ptr) parameters;
} function_info;

typedef struct type_t {
    enum type_category category;
    union {
        enum primitive_type primitive;
        array_info * array;
        struct_info * structure;
        function_info * function;
    };
} type_t;

type_t * create_primitive_type(enum primitive_type primitive);
type_t * create_array_type(type_t * base, unsigned size);
type_t * create_struct_type(unsigned field_num);
type_t * create_function_type(type_t * returned, unsigned para_num);
void delete_type(type_t * type);
bool type_equal(const type_t * a, const type_t * b);

#endif
