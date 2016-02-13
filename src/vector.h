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

#define VECGEN(TYPE, NAME) \
static Vector * vector_init_##NAME(Vector * v, unsigned initial_capacity) { \
    initial_capacity = initial_capacity > 0 ? initial_capacity : 1; \
    v->data = malloc(sizeof(TYPE) * initial_capacity); \
    if (v->data == NULL) \
        uerr("Could not allocate vector data."); \
    v->capacity = initial_capacity; \
    v->count = 0; \
    return v; \
} \
static void vector_deinit_##NAME(Vector * v) { \
    free(v->data); \
    v->data = NULL; \
} \
static void vector_resize_##NAME(Vector * v) { \
    v->capacity = v->count * 2; /* double capacity */ \
    v->data = realloc(v->data, sizeof(TYPE) * v->capacity); \
    if (v->data == NULL) \
        uerr("Resize failed."); \
} \
static void vector_trim_##NAME(Vector * v, unsigned extra_capacity) { \
    v->capacity = v->count + extra_capacity; \
    v->data = realloc(v->data, sizeof(TYPE) * v->capacity); \
    if (v->data == NULL) \
        uerr("Trim failed."); \
} \
static void vector_empty_##NAME(Vector * v) { \
    v->count = 0; \
} \
static void vector_reset_##NAME(Vector * v, unsigned capacity) { \
    v->count = 0; \
    realloc(v->data, sizeof(TYPE) * capacity); \
    v->capacity = capacity; \
} \
static TYPE * vector_ptr_##NAME(const Vector * v, unsigned index) { \
    if (index >= v->count) \
        uerr("Out of bounds."); \
    return ((TYPE *)v->data) + index; \
} \
static TYPE vector_get_##NAME(const Vector * v, unsigned index) { \
    if (index >= v->count) \
        uerr("Out of bounds."); \
    return ((TYPE *)v->data)[index]; \
} \
static TYPE vector_push_##NAME(Vector * v, TYPE value) { \
    if (v->count >= v->capacity) \
        vector_resize_##NAME(v); \
    return (((TYPE *)v->data)[v->count++] = value); \
} \
static void vector_set_##NAME(Vector * v, unsigned index, TYPE value) { \
    if (index >= v->count) \
        uerr("Out of bounds."); \
    ((TYPE *)v->data)[index] = value; \
} \
static TYPE vector_pop_##NAME(Vector * v) { \
    if (v->count == 0) \
        uerr("Underflow."); \
    return ((TYPE *)v->data)[--v->count]; \
} \
static TYPE vector_remove_##NAME(Vector * v, unsigned index) { \
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
static void vector_insert_##NAME(Vector * v, unsigned index, TYPE value) { \
    if (index >= v->count) \
        uerr("Out of bounds."); \
    char *src = (char *) v->data + (sizeof(TYPE) * index); \
    char *dest = src + sizeof(TYPE); \
    size_t num_bytes = sizeof(TYPE) * (v->count - index); \
    memmove(dest, src, num_bytes); \
    v->count++; \
    ((TYPE *)v->data)[index] = value; \
} \
static void vector_bag_remove_##NAME(Vector * v, unsigned index) { \
    if (index == v->count - 1) \
        vector_pop_##NAME(v); \
    else \
        vector_set_##NAME(v, index, vector_pop_##NAME(v)); \
} \
static unsigned vector_ptrdiff_##NAME(const Vector * v, void * ptr) { \
    return (TYPE *)ptr - (TYPE *)v->data; \
} \

#endif
