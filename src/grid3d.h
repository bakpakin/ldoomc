#ifndef GRID3D_HEADER
#define GRID3D_HEADER

#include "ldmath.h"

struct grid3d;
typedef struct grid3d grid3d;

struct grid3d_aabb3_handle {
    unsigned char mark;
    aabb3 aabb;
};
typedef struct grid3d_aabb3_handle grid3d_handle;

grid3d * grid3d_new(unsigned xsize, unsigned ysize, unsigned zsize);

void grid3d_delete(grid3d * g);

grid3d_handle * grid3d_add(grid3d * g, const aabb3 aabb);

void grid3d_remove(grid3d * g, grid3d_handle * handle);

void grid3d_update(grid3d * g, grid3d_handle * handle, const aabb3 dest);

void grid3d_bounds(grid3d * g, aabb3 out);

// Iterators

struct grid3d_iter_pair {
    grid3d_handle * a;
    grid3d_handle * b;
};

int grid3d_has_next(grid3d * g);

struct grid3d_iter_pair grid3d_iter_pairs(grid3d * g, const aabb3 bounds);

struct grid3d_iter_pair grid3d_iter_pairs_next(grid3d * g);

grid3d_handle * grid3d_iter(grid3d * g, const aabb3 bounds);

grid3d_handle * grid3d_iter_next(grid3d * g);

#endif
