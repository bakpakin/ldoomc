#ifndef GEOM_HEADER
#define GEOM_HEADER

#include "ldmath.h"

#include <stdlib.h>

/*
 * Polygons should be convex and wound from the positive x-axis to the positive y-axis. (CCW)
 * Ploygons that don't follow this assumption make work for some functions, and not for others.
 */
typedef struct {
    unsigned count;
    vec2 points[];
} poly;

poly * poly_init(poly * p, int count, ...);

poly * poly_new(int count, ...);

int poly_contains(const poly * p, vec2 point);

void poly_aabb(const poly * p, aabb2 out);

float point_seg_dist2(const vec2 p, const vec2 p1, const vec2 p2);

int poly_contains_circle(const poly * p, const vec2 point, float r);

int sat_circle_poly(const vec2 center, float radius, const poly * p, vec2 displacement);

int sat_circle_poly_edges(const vec2 center, float radius, const poly * p, unsigned edgefield, vec2 displacement);

int sat_circle_circle(const vec2 a, float ra, const vec2 b, float rb, vec2 displacement);

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
