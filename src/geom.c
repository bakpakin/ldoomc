#include "geom.h"
#include <float.h>
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

    vec2 p1, p2;

    float px = point[0];
    float py = point[1];

    for (unsigned i = 0, j = p->count - 1; i < p->count; j = i++) {
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

    for (unsigned i = 0; i < p->count; i++) {
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
    for (unsigned i = 0, j = p->count - 1; i < p->count; j = i++) {
        if (point_seg_dist2(point, p->points[i], p->points[j]) < r2) {
            return 1;
        }
    }
    return 0;

}

// Use SAT algorithm for now, possibly upgrade to GJK if performance becomes an issue. (Unlikely)
// In fact, for small polygon circle tests, SAT might be faster.
// Also, SAT allows ignoring interior edges for compound polygons.
int sat_circle_poly_edges(const vec2 ccenter, float radius, const poly * p, unsigned edgefield, vec2 displacement) {

    float min_sep2 = FLT_MAX; // Current minimum separation from circle center squared
    float r2 = radius * radius;

    vec2 sep; sep[0] = sep[1] = 0.0f;
    vec2 difference;
    vec2 itocenter, jtocenter;

    for (unsigned edge = 1, i = 0, j = p->count - 1; i < p->count; j = i++, edge *= 2) {

        if (!(edge & edgefield)) continue;

        // Check if the circle is in the edge region
        vec2_sub(difference, p->points[i], p->points[j]);
        vec2_sub(itocenter, ccenter, p->points[i]);
        vec2_sub(jtocenter, ccenter, p->points[j]);
        float doti = vec2_dot(itocenter, difference);
        float dotj = vec2_dot(jtocenter, difference);

        if (doti > 0 || dotj < 0) {

            float distance2 = vec2_len2(itocenter);
            if (distance2 < min_sep2) {
                sep[0] = difference[1]; sep[1] = -difference[0];
                min_sep2 = distance2;
            }

        } else {

            float crossl = vec2_crossl(difference, itocenter);
            float distance2 = crossl * crossl / vec2_len2(difference);

            if (crossl > 0 && distance2 > r2) return 0;

            if (distance2 < min_sep2) {
                vec2_assign(sep, itocenter);
                min_sep2 = distance2;
            }

        }
    }

    vec2_scale(displacement, sep, (radius - sqrt(min_sep2)) / vec2_len(sep));

    return 1;
}

int sat_circle_poly(const vec2 center, float radius, const poly * p, vec2 displacement) {
    return sat_circle_poly_edges(center, radius, p, ~((unsigned)0), displacement);
}

int sat_circle_circle(const vec2 a, float ra, const vec2 b, float rb, vec2 displacement) {
    float r = ra + rb;
    float r2 = r * r;
    vec2 diff;
    vec2_sub(diff, b, a);
    if (vec2_len2(diff) > r2)
        return 0;
    float dist = vec2_len(diff);
    vec2_scale(displacement, diff, (dist - r) / dist);
    return 1;
}
