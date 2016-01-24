#include "modelshader.h"
#include "glfw.h"

static ModelShader * modelshader_init(ModelShader * ms) {
    ms->mvpLocation = glGetUniformLocation(ms->program.id, "mvp");
    ms->timeLocation = glGetUniformLocation(ms->program.id, "time");
    ms->diffuseLocation = glGetUniformLocation(ms->program.id, "diffuse");
    ms->specularLocation = glGetUniformLocation(ms->program.id, "specular");
    ms->normalLocation = glGetUniformLocation(ms->program.id, "normal");
    return ms;
}

ModelShader * modelshader_init_program(
        ModelShader * ms,
        const Program * p) {
    ms->program = *p;
    return modelshader_init(ms);
}

ModelShader * modelshader_init_quick(
        ModelShader * ms,
        const char * source) {
    program_init_quick(&ms->program, source);
    return modelshader_init(ms);
}

ModelShader * modelshader_init_vertfrag(
        ModelShader * ms,
        const char * vert_source,
        const char * frag_source) {
    program_init_vertfrag(&ms->program, vert_source, frag_source);
    return modelshader_init(ms);
}

ModelShader * modelshader_init_file(
        ModelShader * ms,
        const char * path) {
   program_init_file(&ms->program, path);
   return modelshader_init(ms);
}

ModelShader * modelshader_init_resource(
        ModelShader * ms,
        const char * resource) {
   program_init_resource(&ms->program, resource);
   return modelshader_init(ms);
}

void modelshader_deinit(ModelShader * ms) {
    program_deinit(&ms->program);
}
