#include "shader.h"
#include "opengl.h"
#include <string.h>
#include "util.h"
#include <stdarg.h>
#include "platform.h"

static const char * vert_prepend = "\n#define VERTEX 1\n";
static const char * frag_prepend = "\n#define FRAGMENT 1\n";
static const char * geom_prepend = "\n#define GEOMETRY 1\n";

static const char * get_source_header(GLint shaderType) {
    switch(shaderType) {
        case GL_VERTEX_SHADER:
            return vert_prepend;
        case GL_FRAGMENT_SHADER:
            return frag_prepend;
        case GL_GEOMETRY_SHADER:
            return geom_prepend;
        default:
            return "";
    }
}

static GLuint shader_create_shader(const GLchar * source, GLint type, const GLchar * prepend) {

    GLuint s = glCreateShader(type);

    char * split_ptr = strstr(source, "#version");
    char * cr_loc = strstr(split_ptr, "\n");
    if (!cr_loc) cr_loc = strstr(split_ptr, "\r");
    split_ptr = cr_loc;

    GLint lens[4] = {
        split_ptr - source,
        -1,
        -1,
        -1
    };

    const GLchar * sources[4] = {
        source,
        get_source_header(type),
        prepend,
        split_ptr
    };

    glShaderSource(s, 4, sources, lens);
    glCompileShader(s);
    GLint status;
    glGetShaderiv(s, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        GLint infologlength;
        glGetShaderiv(s, GL_INFO_LOG_LENGTH, &infologlength);
        char * str = malloc(infologlength + 1);
        glGetShaderInfoLog(s, infologlength, NULL, str);
        uerr(str);
        free(str);
    }
    return s;
}

Shader * shader_init(Shader * s, const char * source, GLint type) {
    s->type = type;
    s->id = shader_create_shader(source, type, "");
    return s;
}

Shader * shader_init_file(Shader * s, const char * path, GLint type) {
	s->type = type;
	char * source = util_slurp(path, NULL);
	s->id = shader_create_shader(source, type, "");
	free(source);
	return s;
}

Shader * shader_init_resource(Shader * s, const char * resource, GLint type) {
    char file[200];
    platform_res2file(resource, file, 200);
    return shader_init_file(s, file, type);
}

void shader_deinit(Shader * s) {
    glDeleteShader(s->id);
}

Program * program_init(Program * p, int shader_count, ...) {

    va_list l;
    GLuint prg = glCreateProgram();
    int j;

    va_start(l, shader_count);
    for (j = 0; j < shader_count; j++) {
        GLuint s = va_arg(l, Shader *)->id;
        glAttachShader(prg, s);
    }
    va_end(l);

    glLinkProgram(prg);

    GLint isLinked = 0;
    glGetProgramiv(prg, GL_LINK_STATUS, &isLinked);
    if (isLinked != GL_TRUE) {
        GLint infologlength;
        glGetProgramiv(prg, GL_INFO_LOG_LENGTH, &infologlength);
        char * str = malloc(infologlength + 1);
        glGetProgramInfoLog(prg, infologlength, NULL, str);
        uerr(str);
        free(str);
    }

    va_start(l, shader_count);
    for (j = 0; j < shader_count; j++) {
        GLuint s = va_arg(l, Shader *)->id;
        glDetachShader(prg, s);
    }
    va_end(l);

    p->id = prg;

    return p;
}

Program * program_init_quick(Program * p, const char * source) {
   Shader vert, frag;
   shader_init(&vert, source, GL_VERTEX_SHADER);
   shader_init(&frag, source, GL_FRAGMENT_SHADER);
   program_init(p, 2, &vert, &frag);
   shader_deinit(&vert);
   shader_deinit(&frag);
   return p;
}

Program * program_init_vertfrag(Program * p, const char * vert_source, const char * frag_source) {
   Shader vert, frag;
   shader_init(&vert, vert_source, GL_VERTEX_SHADER);
   shader_init(&frag, frag_source, GL_FRAGMENT_SHADER);
   program_init(p, 2, &vert, &frag);
   shader_deinit(&vert);
   shader_deinit(&frag);
   return p;
}

Program * program_init_file(Program * p, const char * path) {
    char * source = util_slurp(path, NULL);
	program_init_vertfrag(p, source, source);
    free(source);
    return p;
}

Program * program_init_resource(Program * p, const char * resource) {
    char file[200];
    platform_res2file(resource, file, 200);
    return program_init_file(p, file);
}

void program_deinit(Program * p) {
    glDeleteProgram(p->id);
}
