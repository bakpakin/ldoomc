#include "grid3d.h"
#include "ldmath.h"
#include "vector.h"

#define iter_3d(MIN, MAX) for (unsigned x = MIN.x; x <= MAX.x; x++) \
                          for (unsigned y = MIN.y; y <= MAX.y; y++) \
                          for (unsigned z = MIN.z; z <= MAX.z; z++)

#define ITER_BIT 2
#define ALIVE_BIT 1

typedef struct {
    unsigned flags;
    aabb3 aabb;
} GHandle;

VECTOR_STATIC_GENERATE(int, int);

struct cell_index {
    int x, y, z;
};

static inline int cindex_clamp(int x, int min, int max) {
    x = x < min ? min : x;
    return x > max ? max : x;
}

/*
 * Converts a floating point position vector into a cell coordinate, and
 * clamps the components to inside the cell grid. This way, AABBs outside the
 * the grid bounds will be added to the closest cells.
 */

static inline struct cell_index get_cindex(Grid * g, const vec3 pos) {
    struct cell_index ret = (struct cell_index) {
        ldm_floor(pos[0] / g->gridsize),
        ldm_floor(pos[1] / g->gridsize),
        ldm_floor(pos[2] / g->gridsize)
    };
    ret.x = cindex_clamp(ret.x, 0, g->size.x - 1);
    ret.y = cindex_clamp(ret.y, 0, g->size.y - 1);
    ret.z = cindex_clamp(ret.z, 0, g->size.z - 1);
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

/*
 * For debugging
 */
static inline void aabb3_print(const aabb3 x) {
    printf("<min: %f, %f, %f | max: %f, %f, %f>\n", x[0][0], x[0][1], x[0][2], x[1][0], x[1][1], x[1][2]);
}

Grid * grid_init(Grid * g, float gridsize, unsigned xsize, unsigned ysize, unsigned zsize) {
    g->gridsize = gridsize;
    g->aabbs = malloc(sizeof(GHandle) * 64);
    g->aabb_capacity = 64;
    g->first_free_index = 0;
    g->after_last_index = 0;
    g->aabb_count = 0;
    g->cells = malloc(sizeof(Vector) * xsize * ysize * zsize);
    g->size.x = xsize;
    g->size.y = ysize;
    g->size.z = zsize;
    g->mark = 0;
    g->first_free_index = 0;

    unsigned cellcount = g->size.x * g->size.y * g->size.z;
    for (int i = 0; i < cellcount; i++)
        vector_init_int(g->cells + i, 20);

    vector_init_int(&g->iter, 128);
    return g;
}

void grid_deinit(Grid * g) {
    unsigned cellcount = g->size.x * g->size.y * g->size.z;
    for (int i = 0; i < cellcount; i++)
        vector_deinit_int(g->cells + i);
    vector_deinit_int(&g->iter);
    free(g->cells);
    free(g->aabbs);
}

int grid_add(Grid * g, const aabb3 aabb) {

    int handle = g->first_free_index++;
    if (handle >= g->aabb_capacity) {
        g->aabb_capacity *= 2;
        g->aabbs = realloc(g->aabbs, sizeof(GHandle) * g->aabb_capacity);
    }
    if (handle >= g->after_last_index)
        g->after_last_index = handle + 1;

    GHandle * ptr = ((GHandle *)g->aabbs) + handle;
    aabb3_assign(ptr->aabb, aabb);
    ptr->flags = (ITER_BIT & g->mark) | ALIVE_BIT;

    struct cell_index minc = get_cindex(g, aabb[0]);
    struct cell_index maxc = get_cindex(g, aabb[1]);

    iter_3d(minc, maxc)
        vector_push_int(g->cells + to_index(g, x, y, z), handle);

    g->aabb_count++;
    return handle;
}

static inline void remove_from_cell(Vector * v, int handle) {
    for (int i = 0; i < v->count; i++)
        if (vector_get_int(v, i) == handle) {
            vector_bag_remove_int(v, i);
            return;
        }
    uerr("Can't find handle in expected cell.");
}

void grid_remove(Grid * g, int handle) {

    GHandle * ptr = (GHandle *)g->aabbs + handle;

    struct cell_index minc = get_cindex(g, ptr->aabb[0]);
    struct cell_index maxc = get_cindex(g, ptr->aabb[1]);

    iter_3d(minc, maxc)
        remove_from_cell(g->cells + to_index(g, x, y, z), handle);

    ptr->flags &= ~ALIVE_BIT;
    if (handle < g->first_free_index)
        g->first_free_index = handle;

    g->aabb_count--;
}

void grid_update(Grid * g, int handle, const aabb3 dest) {

    GHandle * ptr = (GHandle *)g->aabbs + handle;

    struct cell_index minc = get_cindex(g, ptr->aabb[0]);
    struct cell_index maxc = get_cindex(g, ptr->aabb[1]);
    struct cell_index mind = get_cindex(g, dest[0]);
    struct cell_index maxd = get_cindex(g, dest[1]);

    iter_3d(minc, maxc)
        if (x < mind.x || x > maxd.x || y < mind.y || y > maxd.y || z < mind.z || z > maxd.z)
            remove_from_cell(g->cells + to_index(g, x, y, z), handle);

    iter_3d(mind, maxd)
        if (x < minc.x || x > maxc.x || y < minc.y || y > maxc.y || z < minc.z || z > maxc.z)
            vector_push_int(g->cells + to_index(g, x, y, z), handle);
}

void grid_aabb(Grid * g, int handle, aabb3 out) {
    aabb3_assign(out, ((GHandle *)g->aabbs + handle)->aabb);
}

void grid_bounds(Grid * g, aabb3 out) {
    float xs = g->gridsize * g->size.x;
    float ys = g->gridsize * g->size.y;
    float zs = g->gridsize * g->size.z;
    vec3 min = {0, 0, 0};
    vec3 max = {xs, ys, zs};
    aabb3_fill(out, min, max);
}

static void iterate_cell(Grid * g, Vector * v, const aabb3 bounds) {
    for (unsigned i = 0; i < v->count; i++) {
        int ih = vector_get_int(v, i);
        GHandle * h = (GHandle *) g->aabbs + ih;
        if (~((h->flags ^ g->mark) & ITER_BIT) && aabb3_overlaps(h->aabb, bounds)) {
            vector_push_int(&g->iter, ih);
            h->flags ^= ITER_BIT;
        }
    }
}

static void iterate_cell_noflag(Grid * g, Vector * v, const aabb3 bounds) {
    for (unsigned i = 0; i < v->count; i++) {
        int ih = vector_get_int(v, i);
        GHandle * h = (GHandle *) g->aabbs + ih;
        if (aabb3_overlaps(h->aabb, bounds)) {
            vector_push_int(&g->iter, ih);
        }
    }
}

typedef void (*iter_fn) (Grid *, Vector *, const aabb3);

static inline void iterate_grid(iter_fn fn, Grid * g, const aabb3 bounds) {
    struct cell_index c1 = get_cindex(g, bounds[0]);
    struct cell_index c2 = get_cindex(g, bounds[1]);
    iter_3d(c1, c2)
        fn(g, g->cells + to_index(g, x, y, z), bounds);
}

// Iterator
int grid_iter_next(Grid * g, int * a) {
    if (g->iter.count > g->iter_index1) {
        *a = vector_get_int(&g->iter, g->iter_index1++);
        return 1;
    }
    return 0;
}

void grid_iter(Grid * g, const aabb3 bounds) {
    g->iter.count = 0;
    iterate_grid(iterate_cell, g, bounds);
    g->mark ^= ITER_BIT;
    g->iter_index1 = 0;
}

static void iterate_cell_for_pairs(Grid * g, Vector * v, const aabb3 bounds) {
    unsigned char mark = g->mark;
    for (unsigned i = 0; v->count; i++) {
        int ih = vector_get_int(v, i);
        GHandle * h = (GHandle *) g->aabbs + ih;
        if (~((h->flags ^ mark) & ITER_BIT) && aabb3_overlaps(bounds, h->aabb)) {
            h->flags ^= ITER_BIT;
            vector_push_int(&g->iter, ih);
            unsigned oldcount = g->iter.count;
            iterate_grid(iterate_cell_noflag, g, h->aabb);
            if (oldcount == g->iter.count)
                vector_pop_int(&g->iter);
            else
                vector_push_int(&g->iter, -1);
        }
    }
}

void grid_iter_pairs(Grid * g, const aabb3 bounds) {
    g->iter.count = 0;
    iterate_grid(iterate_cell_for_pairs, g, bounds);
    g->mark ^= ITER_BIT;
    g->iter_index1 = 0;
    g->iter_index2 = -1;
}

int grid_iter_pairs_next(Grid * g, int * a, int * b) {
    if (g->iter.count > g->iter_index1) {
        if (g->iter_index1 >= 0) {
            g->iter_index1 = vector_get_int(&g->iter, g->iter_index2++);
        }
        int next = vector_get_int(&g->iter, g->iter_index2++);
        if (next == -1) {
            g->iter_index1 = -1;
            return grid_iter_pairs_next(g, a, b);
        }
        *a = vector_get_int(&g->iter, g->iter_index1);
        *b = next;
        return 1;
    }
    return 0;
}

#undef iter_3d
