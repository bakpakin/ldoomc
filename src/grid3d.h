/*
 * Defines a 3d space-partioning grid type. Useful for
 * Rendering optimization, collision calculations, and
 * spatial querying.
 */
#ifndef GRID3D_HEADER
#define GRID3D_HEADER

#include "ldmath.h"
#include "vector.h"

typedef struct {
    struct {
        unsigned x, y, z;
    } size;
    void * aabbs;
    unsigned aabb_count;
    unsigned aabb_capacity;
    unsigned first_free_index;
    unsigned after_last_index;
    Vector * cells;

    // Iteration implementation
    Vector iter;
    unsigned char mark;
    int iter_index1;
    int iter_index2;
} Grid;

Grid * grid_init(Grid * g, unsigned xsize, unsigned ysize, unsigned zsize);

void grid_deinit(Grid * g);

int grid_add(Grid * g, const aabb3 aabb);

void grid_remove(Grid * g, int handle);

void grid_update(Grid * g, int handle, const aabb3 dest);

void grid_aabb(Grid * g, int handle, aabb3 out);

void grid_bounds(Grid * g, aabb3 out);

// Iterators

int grid_has_next(Grid * g);

void grid_iter_pairs(Grid * g, const aabb3 bounds);

void grid_iter_pairs_next(Grid * g, int * a, int * b);

void grid_iter(Grid * g, const aabb3 bounds);

void grid_iter_next(Grid * g, int * a);

#endif
