#include "quickdraw.h"
#include "glfw.h"
#include "shader.h"
#include "platform.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

#define DM_STROKE 0
#define DM_FILL 1
#define DM_STROKEFILL 2
static int drawing = 0;

static float color[4] = {1.0, 1.0, 1.0, 1.0};

static float * pbuffer = NULL;
static unsigned pbuffer_len = 0;
static unsigned pbuffer_capacity = 0;

static Program program;

static const char *vsource = "#version 330 core\n"
"uniform vec4 color;\n"
"uniform mat4 matrix;\n"
"layout(location = 0) in vec2 p;\n"
"void main() { gl_Position = matrix * vec4(p, 0.0, 1.0); }";
static const char *fsource = "#version 330 core\n"
"uniform vec4 color;\n"
"uniform mat4 matrix;\n"
"out vec4 c;\n"
"void main() { c = color; }";
static GLint ucolor_loc;
static GLint umatrix_loc;

static GLuint VAO;
static GLuint VBO;

static inline void ensure_capacity(unsigned cap) {
    if (pbuffer_capacity < cap) {
        pbuffer_capacity = cap;
        pbuffer = realloc(pbuffer, sizeof(float) * cap);
    }
}

void qd_init() {
    static int initialized = 0;
    if (initialized) return;
    program_init_vertfrag(&program, vsource, fsource);
    ucolor_loc = glGetUniformLocation(program.id, "color");
    umatrix_loc = glGetUniformLocation(program.id, "matrix");

    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);
    glBindVertexArray(0);

    pbuffer = malloc(10 * sizeof(float));
    pbuffer_capacity = 10;

    initialized = 1;
}

void qd_deinit() {
    program_deinit(&program);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    if (pbuffer)
        free(pbuffer);
}

void qd_rgbav(float c[4]) {
    memcpy(&color, c, 4 * sizeof(float));
}

void qd_rgba(float r, float g, float b, float a) {
    color[0] = r;
    color[1] = g;
    color[2] = b;
    color[3] = a;
}

void qd_rgbv(float c[3]) {
    memcpy(&color, c, 3 * sizeof(float));
    color[3] = 1.0f;
}

void qd_rgb(float r, float g, float b) {
    color[0] = r;
    color[1] = g;
    color[2] = b;
    color[3] = 1.0f;
}

void qd_begin() {
    if (drawing) return;
    pbuffer_len = 0;
    drawing = 1;
}

void qd_point(float x, float y) {
    if (!drawing) return;
    ensure_capacity(pbuffer_len + 2);
    pbuffer[pbuffer_len] = x;
    pbuffer[pbuffer_len + 1] = y;
    pbuffer_len += 2;
}

void qd_pointv(float p[2]) {
    if (!drawing) return;
    qd_point(p[0], p[1]);
}

void qd_points(size_t n, float * xys) {
    if (!drawing) return;
    ensure_capacity(pbuffer_len + n);
    memcpy(pbuffer + pbuffer_len, xys, n);
    pbuffer_len += n;
}

void qd_pointvs(int count, float ** ps) {
    if (!drawing) return;
    ensure_capacity(pbuffer_len + 2 * count);
    for (int i = 0; i < count; i++) {
        pbuffer[pbuffer_len + 2 * i] = ps[i][0];
        pbuffer[pbuffer_len + 2 * i + 1] = ps[i][1];
    }
    pbuffer_len += 2 * count;
}

void qd_circle(float x, float y, float r, int segs, unsigned type) {
    if (drawing) return;
    qd_begin();
    double dang = 2 * M_PI / segs;
    double ang = 0.0;
    ensure_capacity(pbuffer_len + 2 * segs);
    for (int i = 0; i < segs; i++, ang += dang) {
        pbuffer[pbuffer_len + 2 * i] = x + r * sin(ang);
        pbuffer[pbuffer_len + 2 * i + 1] = y + r * cos(ang);
    }
    pbuffer_len += 2 * segs;
    qd_end();
    qd_draw(type);
}

void qd_rect(float x, float y, float w, float h, unsigned type) {
    if (drawing) return;
    ensure_capacity(8);
    pbuffer[0] = x;
    pbuffer[1] = y;
    pbuffer[2] = x;
    pbuffer[3] = y + h;
    pbuffer[4] = x + w;
    pbuffer[5] = y + h;
    pbuffer[6] = x + w;
    pbuffer[7] = y;
    pbuffer_len = 8;
    qd_draw(type);
}

void qd_poly(unsigned type, unsigned count, float * points) {
    if (drawing) return;
    // Temporarily set pbuffer
    float * oldpbuffer = pbuffer;
    unsigned oldsize = pbuffer_len;
    pbuffer = points;
    pbuffer_len = count;

    // Draw stuff
    qd_draw(type);

    // Set it back
    pbuffer = oldpbuffer;
    pbuffer_len = oldsize;
}

static GLenum get_gl_draw_type(unsigned t) {
    switch (t) {
    case QD_LINES: return GL_LINES;
    case QD_LINESTRIP: return GL_LINE_STRIP;
    case QD_LINELOOP: return GL_LINE_LOOP;
    case QD_TRIANGLES: return GL_TRIANGLES;
    case QD_TRIANGLESTRIP: return GL_TRIANGLE_STRIP;
    case QD_TRIANGLEFAN: return GL_TRIANGLE_FAN;
    case QD_POINTS: return GL_POINTS;
    default: return GL_TRIANGLE_FAN;
    }
}

void qd_end() {
    drawing = 0;
}

void qd_draw(unsigned type) {
    if (drawing || (type == QD_NONE)) return;
    glUseProgram(program.id);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * pbuffer_len, pbuffer, GL_DYNAMIC_DRAW);
    glUniform4fv(ucolor_loc, 1, color);
    glUniformMatrix4fv(umatrix_loc, 1, GL_FALSE, platform_screen_matrix());
    glDrawArrays(get_gl_draw_type(type), 0, pbuffer_len / 2);
}
