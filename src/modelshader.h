#ifndef MODEL_SHADER_HEADER
#define MODEL_SHADER_HEADER

#include "ldmath.h"
#include "shader.h"

typedef struct {
    Program program;
    int timeLocation;
    int mvpLocation;
    int diffuseLocation;
    int specularLocation;
    int normalLocation;
} ModelShader;

ModelShader * modelshader_init_program(
        ModelShader * ms,
        const Program * p);

ModelShader * modelshader_init_quick(
        ModelShader * ms,
        const char * source);

ModelShader * modelshader_init_vertfrag(
        ModelShader * ms,
        const char * vert_source,
        const char * frag_source);

ModelShader * modelshader_init_file(
        ModelShader * ms,
        const char * path);

ModelShader * modelshader_init_resource(
        ModelShader * ms,
        const char * resource);

void modelshader_deinit(ModelShader * ms);

#endif
