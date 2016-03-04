#include "sky.h"
#include "shader.h"
#include "opengl.h"
#include "ldmath.h"
#include "mesh.h"

static Program skyshader;

static vec3 sunDirection;
static vec3 lookDirection;
static GLint sunUniform;
static GLint lookUniform;

static Mesh quad;

static GLfloat points[8] = {
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f
};

static GLushort els[4] = { 0, 1, 2, 3 };

void sky_init() {
    program_init_resource(&skyshader, "sky.glsl");
    sunUniform = glGetUniformLocation(skyshader.id, "sundir");
    lookUniform = glGetUniformLocation(skyshader.id, "direction");

    mesh_init_nocopy(&quad,
        MESHTYPE_2D,
        GL_STATIC_DRAW,
        8,
        points,
        4,
        els);
}

void sky_deinit() {
    program_deinit(&skyshader);
    //mesh_deinit(&quad);
}

void sky_render() {
    glUseProgram(skyshader.id);
    glUniform3fv(sunUniform, 1, sunDirection);
    glUniform3fv(sunUniform, 1, lookDirection);
    mesh_draw(&quad);
}
