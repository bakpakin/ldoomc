#ifndef QUICKDRAW_HEADER
#define QUICKDRAW_HEADER

#include <stdlib.h>
#include "geom.h"

#define QD_LINES 1
#define QD_LINESTRIP 2
#define QD_LINELOOP 4
#define QD_TRIANGLES 8
#define QD_TRIANGLESTRIP 16
#define QD_TRIANGLEFAN 32
#define QD_POINTS 64

void qd_init();

void qd_deinit();

void qd_matrix(const float m[16]);

void qd_rgbav(float c[4]);

void qd_rgba(float r, float g, float b, float a);

void qd_rgbv(float c[3]);

void qd_rgb(float r, float g, float b);

void qd_begin();

void qd_point(float x, float y);

void qd_pointv(float p[2]);

void qd_points(size_t n, float * xys);

void qd_pointvs(int count, float ** ps);

void qd_circle(float x, float y, float r, int segs, unsigned type);

void qd_rect(float x, float y, float w, float h, unsigned type);

void qd_poly(const poly * p, unsigned type);

void qd_end();

void qd_draw(unsigned type);

#endif
