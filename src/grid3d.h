/*
 * Defines a 3d space-partioning grid type. Useful for
 * Rendering optimization, collision calculations, and
 * spatial querying.
 */
#ifndef GRID3D_HEADER
#define GRID3D_HEADER

#include "ldmath.h"
#include "vector.h"
#include "opool.h"

typedef struct  {
    unsigned char mark;
    aabb3 aabb;
} GHandle;

typedef struct {
    struct {
        unsigned x, y, z;
    } size;
    Flexpool aabbs;
    Vector * cells;

    // Iteration implementation
    Vector iter;
    unsigned char mark;
    GHandle * iter_handle1;
    unsigned iter_index;
} Grid;

Grid * grid_init(Grid * g, unsigned xsize, unsigned ysize, unsigned zsize);

void grid_deinit(Grid * g);

GHandle * grid_add(Grid * g, const aabb3 aabb);

void grid_remove(Grid * g, GHandle * handle);

void grid_update(Grid * g, GHandle * handle, const aabb3 dest);

void grid_bounds(Grid * g, aabb3 out);

// Iterators

struct grid_iter_pair {
    GHandle * a;
    GHandle * b;
};

int grid_has_next(Grid * g);

struct grid_iter_pair grid_iter_pairs(Grid * g, const aabb3 bounds);

struct grid_iter_pair grid_iter_pairs_next(Grid * g);

GHandle * grid_iter(Grid * g, const aabb3 bounds);

GHandle * grid_iter_next(Grid * g);

#endif
