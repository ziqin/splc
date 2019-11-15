#ifndef ARRAY_H
#define ARRAY_H

#include <assert.h>
#include <stdlib.h>

#define USE_ARRAY(T)                                                            \
                                                                                \
struct T##_array_t {                                                            \
    T * addr;                                                                   \
    unsigned length;                                                            \
    unsigned capacity;                                                          \
};                                                                              \
                                                                                \
static struct T##_array_t create_array_##T(int length, int capacity) {          \
    assert(capacity > 0);                                                       \
    struct T##_array_t arr;                                                     \
    arr.addr = malloc(capacity * sizeof(T));                                    \
    if (arr.addr == NULL) {                                                     \
        arr.length = arr.capacity = 0;                                          \
    } else {                                                                    \
        arr.length = length;                                                    \
        arr.capacity = capacity;                                                \
    }                                                                           \
    return arr;                                                                 \
}                                                                               \
                                                                                \
static void delete_array_##T(struct T##_array_t * arr) {                        \
    assert(arr != NULL);                                                        \
    if (arr->addr != NULL) {                                                    \
        free(arr->addr);                                                        \
    }                                                                           \
    arr->length = arr->capacity = 0;                                            \
}                                                                               \
                                                                                \
/* amortized time complexity: O(1) */                                           \
static void append_##T(struct T##_array_t * arr, T val) {                       \
    assert(arr != NULL);                                                        \
    if (arr->length >= arr->capacity) {                                         \
        T * new_addr = realloc(arr->addr, sizeof(T) * (arr->capacity * 2));     \
        assert(new_addr != NULL);                                               \
        arr->addr = new_addr;                                                   \
        arr->capacity = arr->capacity * 2;                                      \
    }                                                                           \
    arr->addr[arr->length++] = val;                                             \
}                                                                               \
                                                                                \
static T pop_##T(struct T##_array_t * arr) {                                    \
    assert(arr != NULL && arr->length > 0);                                     \
    T ans = arr->addr[--(arr->length)];                                         \
    if (arr->length < arr->capacity / 2) {                                      \
        arr->capacity /= 2;                                                     \
        arr->addr = realloc(arr->addr, sizeof(T) * arr->capacity);              \
        assert(arr->addr != NULL);                                              \
    }                                                                           \
    return ans;                                                                 \
}                                                                               \
                                                                                \
typedef struct T##_array_t T##_array_t

#define array_t(T) T##_array_t
#define create_array(T, length, capacity) create_array_##T((length), (capacity))
#define delete_array(T, arr) delete_array_##T((arr))
#define append(T, arr, val) append_##T((arr), (val))
#define pop(T, arr) pop_##T((arr))
#define at(arr, index) ((arr).addr[index])

#endif
