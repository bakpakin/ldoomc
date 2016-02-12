#ifndef GEOM_HEADER
#define GEOM_HEADER

#include "ldmath.h"

#include <stdlib.h>

/*
 * Polygons should be convex and wound from the positive x-axis to the positive y-axis. (CCW)
 * Ploygons that don't follow this assumption make work for some functions, and not for others.
 */
typedef struct {
    int count;
    vec2 points[];
} poly;

static size_t poly_size(const poly * p) {
    return sizeof(poly) + sizeof(vec2) * p->count;
}

poly * poly_init(poly * p, int count, ...);

poly * poly_new(int count, ...);

int poly_contains(const poly * p, vec2 point);

void poly_aabb(const poly * p, aabb2 out);

float point_seg_dist2(const vec2 p, const vec2 p1, const vec2 p2);

int poly_contains_circle(const poly * p, const vec2 point, float r);

int sat_circle_poly(const vec2 center, float radius, const poly * p, vec2 displacement);

int sat_circle_poly_edges(const vec2 center, float radius, const poly * p, unsigned edgefield, vec2 displacement);

int sat_circle_circle(const vec2 a, float ra, const vec2 b, float rb, vec2 displacement);

#define def_polyn(n) \
typedef struct { \
    int count; \
    vec2 points[n]; \
} poly##n; \
static int poly##n##_contains(poly##n * p, vec2 point) { \
    return poly_contains((poly *) p, point); \
}

def_polyn(3);

typedef poly3 tri2d;

def_polyn(4);
def_polyn(5);
def_polyn(6);

static void poly3_init(poly3 * p,
        float x1, float y1,
        float x2, float y2,
        float x3, float y3) {
    poly_init((poly *) p, 3, x1, y1, x2, y2, x3, y3);
}

static void poly4_init(poly4 * p,
        float x1, float y1,
        float x2, float y2,
        float x3, float y3,
        float x4, float y4) {
    poly_init((poly *) p, 4, x1, y1, x2, y2, x3, y3, x4, y4);
}

static void poly5_init(poly5 * p,
        float x1, float y1,
        float x2, float y2,
        float x3, float y3,
        float x4, float y4,
        float x5, float y5) {
    poly_init((poly *) p, 5, x1, y1, x2, y2, x3, y3, x4, y4, x5, y5);
}

static void poly6_init(poly6 * p,
        float x1, float y1,
        float x2, float y2,
        float x3, float y3,
        float x4, float y4,
        float x5, float y5,
        float x6, float y6) {
    poly_init((poly *) p, 6, x1, y1, x2, y2, x3, y3, x4, y4, x5, y5, x6, y6);
}

#undef def_polyn

#define BODY_PRISM 1
#define BODY_CYLINDER 1

typedef struct {
    unsigned type;
    vec3 position;
    float height;
    union {
        poly * polygon;
        float radius;
    } base;
} Body;

#endif
