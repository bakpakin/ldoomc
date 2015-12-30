#include "geom.h"

#include <stdarg.h>

poly * poly_init(poly * p, int count, ...) {

    va_list l;
    va_start(l, count);

    for (int i = 0; i < count; i++) {
        float x = va_arg(l, double);
        float y = va_arg(l, double);
        p->points[i][0] = x;
        p->points[i][1] = y;
    }

    va_end(l);

    return p;

}

poly * poly_new(int count, ...) {

    poly * p = malloc(count * sizeof(vec2) + sizeof(poly));

    va_list l;
    va_start(l, count);

    for (int i = 0; i < count; i++) {
        float x = va_arg(l, double);
        float y = va_arg(l, double);
        p->points[i][0] = x;
        p->points[i][1] = y;
    }

    va_end(l);

    return p;
}

int poly_contains(const poly * p, vec2 point) {

    int odd = 0;

    int i, j;
    vec2 p1, p2;

    float px = point[0];
    float py = point[1];

    for (i = 0, j = p->count - 1; i < p->count; j = i++) {
         vec2_assign(p1, p->points[i]);
         vec2_assign(p2, p->points[j]);
         if ((p1[1] < py && p2[1] > py) || (p1[1] > py && p2[1] < py))
             if (p1[0] + (py - p1[1]) / (p2[1] - p1[1]) * (p2[0] - p1[0]) < px)
                 odd = !odd;
    }

    return odd;

}

void poly_aabb(const poly * p, aabb2 out) {

    vec2 mn = {FLT_MAX, FLT_MAX};
    vec2 mx = {FLT_MIN, FLT_MIN};

    for (int i = 0; i < p->count; i++) {
        vec2_min(mn, mn, p->points[i]);
        vec2_max(mx, mx, p->points[i]);
    }

    aabb2_fill(out, mn, mx);

}

float point_seg_dist2(const vec2 p, const vec2 p1, const vec2 p2) {

    vec2 d, dp;
    vec2_sub(d, p2, p1);
    vec2_sub(dp, p, p1);
    float dot = vec2_dot(dp, d);
    if (dot < 0)
        return vec2_len2(dp);
    float d2 = vec2_len2(d);
    if (dot < d2) {
        vec2_sub(d, p2, p);
        return vec2_len2(d);
    }
    float f = -dot / d2;
    vec2_addmul(d, dp, d, f);
    return vec2_len2(d);

}

int poly_contains_circle(const poly * p, const vec2 point, float r) {

    aabb2 bounds;
    poly_aabb(p, bounds);
    if (aabb2_contains(bounds, point))
        return 1;
    float r2 = r * r;
    int i, j;
    for (i = 0, j = p->count - 1; i < p->count; j = i++) {
        if (point_seg_dist2(point, p->points[i], p->points[j]) < r2) {
            return 1;
        }
    }
    return 0;

}
