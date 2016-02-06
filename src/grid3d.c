#include "grid3d.h"
#include "opool.h"
#include "ldmath.h"
#include "vector.h"

#define GRID3D_CELLSIZE 256

#define iter_3d(MIN, MAX) for (unsigned x = MIN.x; x <= MAX.x; x++) \
                          for (unsigned y = MIN.y; y <= MAX.y; y++) \
                          for (unsigned z = MIN.z; z <= MAX.z; z++)

VECTOR_STATIC_GENERATE(GHandle *, ptr);

struct cell_index {
    int x, y, z;
};

/*
 * Gets converts a floating point position vector into a cell coordinate, and
 * clamps the components to inside the cell grid. This way, AABBs outside the
 * the grid bounds will be added to the closest cells.
 */
static inline struct cell_index get_cindex(Grid * g, const vec3 pos) {
    struct cell_index ret = (struct cell_index) {
        ldm_floor(pos[0] / GRID3D_CELLSIZE) + g->size.x / 2,
        ldm_floor(pos[1] / GRID3D_CELLSIZE) + g->size.y / 2,
        ldm_floor(pos[2] / GRID3D_CELLSIZE) + g->size.z / 2
    };

    ret.x = ret.x < g->size.x ? ret.x : g->size.x - 1;
    ret.x = ret.x >= 0 ? ret.x : 0;

    ret.y = ret.y < g->size.y ? ret.y : g->size.y - 1;
    ret.y = ret.y >= 0 ? ret.y : 0;

    ret.z = ret.z < g->size.z ? ret.z : g->size.z - 1;
    ret.z = ret.z >= 0 ? ret.z : 0;

    return ret;
}

/*
 * Converts cell coordinates to array indices in the grid cell array.
 */
static inline unsigned to_index(Grid * g, unsigned x, unsigned y, unsigned z) {
    return g->size.x * g->size.y * z + g->size.x * y + x;
}

static inline unsigned s_to_index(Grid * g, struct cell_index * ci) {
    return to_index(g, ci->x, ci->y, ci->z);
}

Grid * grid_init(Grid * g, unsigned xsize, unsigned ysize, unsigned zsize) {
    flexpool_init(&g->aabbs, sizeof(GHandle), 512);
    g->cells = calloc(sizeof(Vector), xsize * ysize * zsize);
    g->size.x = xsize;
    g->size.y = ysize;
    g->size.z = zsize;
    g->mark = 0;

    const unsigned cellcount = g->size.x * g->size.y * g->size.z;
    for (int i = 0; i < cellcount; i++)
        vector_init_ptr(g->cells + i, 10);

    vector_init_ptr(&g->iter, 128);
    return g;
}

void grid_deinit(Grid * g) {
    const unsigned cellcount = g->size.x * g->size.y * g->size.z;
    for (int i = 0; i < cellcount; i++)
        vector_deinit_ptr(g->cells + i);
    vector_deinit_ptr(&g->iter);
    free(g->cells);
    flexpool_deinit(&g->aabbs);
}

GHandle * grid_add(Grid * g, const aabb3 aabb) {
    GHandle * ptr = flexpool_alloc(&g->aabbs);
    aabb3_assign(ptr->aabb, aabb);
    ptr->mark = !g->mark;

    struct cell_index minc = get_cindex(g, aabb[0]);
    struct cell_index maxc = get_cindex(g, aabb[1]);

    iter_3d(minc, maxc)
        vector_push_ptr(g->cells + to_index(g, x, y, z), ptr);

    return ptr;
}

static inline void remove_from_cell(Vector * v, GHandle * h) {
    for (int i = 0; i < v->count; i++)
        if (vector_get_ptr(v, i) == h) {
            vector_bag_remove_ptr(v,i);
            return;
        }
    uerr("Can't find handle in expected cell.");
}

void grid_remove(Grid * g, GHandle * handle) {
    struct cell_index minc = get_cindex(g, handle->aabb[0]);
    struct cell_index maxc = get_cindex(g, handle->aabb[1]);

    iter_3d(minc, maxc)
        remove_from_cell(g->cells + to_index(g, x, y, z), handle);

    flexpool_free(&g->aabbs, handle);
}

void grid_update(Grid * g, GHandle * handle, const aabb3 dest) {
    struct cell_index minc = get_cindex(g, handle->aabb[0]);
    struct cell_index maxc = get_cindex(g, handle->aabb[1]);
    struct cell_index mind = get_cindex(g, dest[0]);
    struct cell_index maxd = get_cindex(g, dest[1]);

    iter_3d(minc, maxc)
        if (x < mind.x || x > maxd.x || y < mind.y || y > maxd.y || z < mind.z || z > maxd.z)
            remove_from_cell(g->cells + to_index(g, x, y, z), handle);

    iter_3d(mind, maxd)
        if (x < minc.x || x > maxc.x || y < minc.y || y > maxc.y || z < minc.z || z > maxc.z)
            vector_push_ptr(g->cells + to_index(g, x, y, z), handle);
}

void grid_bounds(Grid * g, aabb3 out) {
    float xs = GRID3D_CELLSIZE * g->size.x / 2;
    float ys = GRID3D_CELLSIZE * g->size.y / 2;
    float zs = GRID3D_CELLSIZE * g->size.z / 2;
    vec3 min = {-xs, -ys, -zs};
    vec3 max = {xs, ys, zs};
    aabb3_fill(out, min, max);
}

static void iterate_cell(Grid * g, Vector * v, const aabb3 bounds) {
    for (unsigned i = 0; i < v->count; i++) {
        GHandle * h = vector_get_ptr(v, i);
        if (h->mark != g->mark && aabb3_overlaps(bounds, h->aabb)) {
            vector_push_ptr(&g->iter, h);
            h->mark = g->mark;
        }
    }
}

typedef void (*iter_fn) (Grid *, Vector *, const aabb3);

static inline void iterate_aabb(iter_fn fn, Grid * g, const aabb3 bounds) {
    struct cell_index c1 = get_cindex(g, bounds[0]);
    struct cell_index c2 = get_cindex(g, bounds[1]);
    iter_3d(c1, c2)
        fn(g, g->cells + to_index(g, x, y, z), bounds);
}

// Iterators
GHandle * grid_iter_next(Grid * g) {
    if (g->iter.count > g->iter_index) {
        return vector_get_ptr(&g->iter, g->iter_index++);
    } else {
        return NULL;
    }
}

GHandle * grid_iter(Grid * g, const aabb3 bounds) {
    g->iter.count = 0;
    iterate_aabb(iterate_cell, g, bounds);
    g->iter_index = 0;
    return grid_iter_next(g);
}

static void iterate_cell_for_pairs(Grid * g, Vector * v, const aabb3 bounds) {
    unsigned char mark = g->mark;
    for (unsigned i = 0; v->count; i++) {
        GHandle * h = vector_get_ptr(v, i);
        if (h->mark != mark && aabb3_overlaps(bounds, h->aabb)) {
            h->mark = mark;
            vector_push_ptr(&g->iter, h);
            unsigned oldcount = g->iter.count;
            iterate_aabb(iterate_cell, g, h->aabb);
            if (oldcount == g->iter.count)
                vector_pop_ptr(&g->iter);
            else
                vector_push_ptr(&g->iter, h);
        }
    }
}

struct grid_iter_pair grid_iter_pairs(Grid * g, const aabb3 bounds) {
    g->iter.count = 0;
    iterate_aabb(iterate_cell_for_pairs, g, bounds);
    g->iter_handle1 = NULL;
    g->iter_index = 0;
    return grid_iter_pairs_next(g);
}

struct grid_iter_pair grid_iter_pairs_next(Grid * g) {
    if (g->iter.count > g->iter_index) {
        if (!g->iter_handle1) {
            g->iter_handle1 = vector_get_ptr(&g->iter, g->iter_index++);
        }
        GHandle * next = vector_get_ptr(&g->iter, g->iter_index++);
        if (next == g->iter_handle1) {
            g->iter_handle1 = NULL;
            return grid_iter_pairs_next(g);
        }
        struct grid_iter_pair tmp = {
            g->iter_handle1, next
        };
        return tmp;
    } else {
        return (struct grid_iter_pair) { NULL, NULL };
    }
}

int grid_has_next(Grid * g) {
    return g->iter.count > 0;
}

#undef iter_3d
