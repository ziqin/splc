#include <assert.h>
#include <string.h>
#include "type.h"


type_t * create_primitive_type(enum primitive_type primitive) {
    type_t * type = (type_t *) malloc(sizeof(type_t));
    type->category = PRIMITIVE;
    type->primitive = primitive;
    return type;
}

type_t * create_array_type(type_t * base, unsigned size) {
    type_t * type = (type_t *) malloc(sizeof(type_t) + sizeof(array_info));
    type->category = ARRAY;
    type->array = (array_info *) (type + 1);
    type->array->base = base;
    type->array->size = size;
    return type;
}

// caller need to append fields by itself
type_t * create_struct_type(unsigned field_num) {
    type_t * type = (type_t *) malloc(sizeof(type_t) + sizeof(struct_info));
    type->category = STRUCTURE;
    type->structure = (struct_info *) (type + 1);
    type->structure->fields = create_array(field_info, field_num, field_num);
    return type;
}

// caller need to append parameters by itself
type_t * create_function_type(type_t * returned, unsigned para_num) {
    type_t * type = (type_t *) malloc(sizeof(type_t) + sizeof(function_info));
    type->category = FUNCTION;
    type->function = (function_info *) (type + 1);
    type->function->returned = returned;
    type->function->parameters = create_array(type_t_ptr, para_num, para_num);
    return type;
}

void delete_type(type_t * type) {
    switch (type->category) {
    case PRIMITIVE:
    case ARRAY: // it's not array's duty to delete the type passed to it
        free(type);
        break;
    case STRUCTURE: // it's not structure's duty to delete field types
        delete_array(field_info, &(type->structure->fields));
        free(type);
        break;
    case FUNCTION: // it's not function's duty to delete types of return value or parameters
        delete_array(type_t_ptr, &(type->function->parameters));
        free(type);
        break;
    }
}

bool type_equal(const type_t * a, const type_t * b) {
    assert(a != NULL);
    assert(b != NULL);
    if (a == b) return true;
    if (a->category != b->category) return false;
    switch (a->category) {
    case PRIMITIVE:
        return a->primitive == b->primitive;
    case ARRAY:
        return (a->array->size == b->array->size) && type_equal(a->array->base, b->array->base);
    case STRUCTURE: {
        array_t(field_info) a_fields = a->structure->fields, b_fields = b->structure->fields;
        if (a_fields.length != b_fields.length) return false;
        for (unsigned i = 0; i < a_fields.length; ++i) {
            if (!type_equal(a_fields.addr[i].type, b_fields.addr[i].type))
                return false;
        }
        return true;
    }
    case FUNCTION: {
        if (!type_equal(a->function->returned, b->function->returned)) return false;
        array_t(type_t_ptr) a_paras = a->function->parameters, b_paras = b->function->parameters;
        for (unsigned i = 0; i < a_paras.length; ++i) {
            if (!type_equal(a_paras.addr[i], b_paras.addr[i]))
                return false;
        }
        return true;
    }
    default:
        return false;
    }
}
