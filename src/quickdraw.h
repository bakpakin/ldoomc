#ifndef QUICKDRAW_HEADER
#define QUICKDRAW_HEADER

#include <stdlib.h>

#define QD_LINES 1
#define QD_LINESTRIP 2
#define QD_LINELOOP 4
#define QD_TRIANGLES 8
#define QD_TRIANGLESTRIP 16
#define QD_TRIANGLEFAN 32
#define QD_POINTS 64

void qd_init();

void qd_deinit();

void qd_color(float c[4]);

void qd_begin();

void qd_point(float x, float y);

void qd_pointv(float p[2]);

void qd_points(size_t n, float * xys);

void qd_pointvs(int count, float ** ps);

void qd_circle(float x, float y, float r, int segs);

void qd_end();

void qd_draw(unsigned type);

#endif
