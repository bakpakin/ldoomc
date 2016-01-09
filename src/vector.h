#ifndef VECTOR_HEADER
#define VECTOR_HEADER

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "util.h"

typedef struct {
    unsigned capacity;
    unsigned count;
    void * data;
} Vector;

// This is why templates exist in c++

#define VECTOR_GENERATE(MODIFIERS, TYPE, NAME) \
MODIFIERS Vector * vector_init_##NAME(Vector * v, unsigned initial_capacity) { \
    initial_capacity = initial_capacity > 0 ? initial_capacity : 1; \
    v->data = malloc(sizeof(TYPE) * initial_capacity); \
    if (v->data == NULL) \
        uerr("Could not allocate vector data."); \
    v->capacity = initial_capacity; \
    v->count = 0; \
    return v; \
} \
MODIFIERS void vector_deinit_##NAME(Vector * v) { \
    free(v->data); \
    v->data = NULL; \
} \
MODIFIERS void vector_resize_##NAME(Vector * v) { \
    v->capacity = v->count * 2; /* double capacity */ \
    v->data = realloc(v->data, sizeof(TYPE) * v->capacity); \
    if (v->data == NULL) \
        uerr("Resize failed."); \
} \
MODIFIERS void vector_trim_##NAME(Vector * v, unsigned extra_capacity) { \
    v->capacity = v->count + extra_capacity; \
    v->data = realloc(v->data, sizeof(TYPE) * v->capacity); \
    if (v->data == NULL) \
        uerr("Trim failed."); \
} \
MODIFIERS void vector_empty_##NAME(Vector * v) { \
    v->count = 0; \
} \
MODIFIERS void vector_reset_##NAME(Vector * v, unsigned capacity) { \
    v->count = 0; \
    realloc(v->data, sizeof(TYPE) * capacity); \
    v->capacity = capacity; \
} \
MODIFIERS TYPE * vector_ptr_##NAME(Vector * v, unsigned index) { \
    if (index >= v->count) \
        uerr("Out of bounds."); \
    return ((TYPE *)v->data) + index; \
} \
MODIFIERS TYPE vector_get_##NAME(Vector * v, unsigned index) { \
    if (index >= v->count) \
        uerr("Out of bounds."); \
    return ((TYPE *)v->data)[index]; \
} \
MODIFIERS TYPE vector_push_##NAME(Vector * v, TYPE value) { \
    if (v->count >= v->capacity) \
        vector_resize_##NAME(v); \
    return (((TYPE *)v->data)[v->count++] = value); \
} \
MODIFIERS void vector_set_##NAME(Vector * v, unsigned index, TYPE value) { \
    if (index >= v->count) \
        uerr("Out of bounds."); \
    ((TYPE *)v->data)[index] = value; \
} \
MODIFIERS TYPE vector_pop_##NAME(Vector * v) { \
    if (v->count == 0) \
        uerr("Underflow."); \
    return ((TYPE *)v->data)[--v->count]; \
} \
MODIFIERS TYPE vector_remove_##NAME(Vector * v, unsigned index) { \
    if (index >= v->count) \
        uerr("Out of bounds."); \
    TYPE ret = ((TYPE *)v->data)[index]; \
    char *dest = (char *) v->data + (sizeof(TYPE) * index); \
    char *src = dest + sizeof(TYPE); \
    size_t num_bytes = sizeof(TYPE) * (v->count - index - 1); \
    memmove(dest, src, num_bytes); \
    v->count--; \
    return ret; \
} \
MODIFIERS void vector_insert_##NAME(Vector * v, unsigned index, TYPE value) { \
    if (index >= v->count) \
        uerr("Out of bounds."); \
    char *src = (char *) v->data + (sizeof(TYPE) * index); \
    char *dest = src + sizeof(TYPE); \
    size_t num_bytes = sizeof(TYPE) * (v->count - index); \
    memmove(dest, src, num_bytes); \
    v->count++; \
    ((TYPE *)v->data)[index] = value; \
} \
MODIFIERS void vector_bag_remove_##NAME(Vector * v, unsigned index) { \
    if (index == v->count - 1) \
        vector_pop_##NAME(v); \
    else \
        vector_set_##NAME(v, index, vector_pop_##NAME(v)); \
} \

#define VECTOR_DECLARE(MODIFIERS, TYPE, NAME) \
MODIFIERS Vector * vector_init_##NAME(Vector * v, unsigned initial_capacity); \
MODIFIERS void vector_deinit_##NAME(Vector * v); \
MODIFIERS void vector_resize_##NAME(Vector * v); \
MODIFIERS void vector_trim_##NAME(Vector * v, unsigned extra_capacity); \
MODIFIERS void vector_empty_##NAME(Vector * v); \
MODIFIERS void vector_reset_##NAME(Vector * v, unsigned capacity); \
MODIFIERS TYPE * vector_ptr_##NAME(Vector * v, unsigned index); \
MODIFIERS TYPE vector_get_##NAME(Vector * v, unsigned index); \
MODIFIERS TYPE vector_push_##NAME(Vector * v, TYPE value); \
MODIFIERS void vector_set_##NAME(Vector * v, unsigned index, TYPE value); \
MODIFIERS TYPE vector_pop_##NAME(Vector * v); \
MODIFIERS TYPE vector_remove_##NAME(Vector * v, unsigned index); \
MODIFIERS void vector_insert_##NAME(Vector * v, unsigned index, TYPE value); \
MODIFIERS void vector_bag_remove_##NAME(Vector * v, unsigned index); \

#define VECTOR_DECLARE_GENERATE(TYPE, NAME) VECTOR_DECLARE( , TYPE, NAME) \
VECTOR_GENERATE( , TYPE, NAME)

#define VECTOR_STATIC_GENERATE(TYPE, NAME) VECTOR_DECLARE(static, TYPE, NAME) \
VECTOR_GENERATE(static, TYPE, NAME)

#endif
