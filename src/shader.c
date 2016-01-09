#include "shader.h"
#include <string.h>
#include "util.h"
#include <stdarg.h>

VECTOR_STATIC_GENERATE(ProgramAttribute, A);
VECTOR_STATIC_GENERATE(ProgramUniform, U);

static const char * all_prepend = "\n";
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
	return s;
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

    // Get Attribute information
    GLint numAttributes;
    GLint attribBufferLength;
    glGetProgramiv(prg, GL_ACTIVE_ATTRIBUTES, &numAttributes);
    glGetProgramiv(prg, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &attribBufferLength);
    attribBufferLength++;
    vector_init_A(&p->attributes, numAttributes);
    char * buf = malloc(sizeof(char) * attribBufferLength * numAttributes);
    char * bufstart = buf;
    for (int i = 0; i < numAttributes; i++) {
        GLsizei length;
        GLenum type;
        GLint size;
        glGetActiveAttrib(prg, i, attribBufferLength, &length, &size, &type, buf);
        ProgramAttribute a = {
            buf,
            type, 
            size
        };
        buf = buf + length + 1;
        vector_push_A(&p->attributes, a);
    }
    realloc(bufstart, buf - bufstart + 1);
    
    // Get Uniform information
    GLint numUniforms;
    GLint uniformBufferLength;
    glGetProgramiv(prg, GL_ACTIVE_UNIFORMS, &numUniforms);
    glGetProgramiv(prg, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniformBufferLength);
    uniformBufferLength++;
    vector_init_U(&p->uniforms, numUniforms);
    buf = malloc(sizeof(char) * uniformBufferLength * numUniforms);
    bufstart = buf;
    for (int i = 0; i < numUniforms; i++) {
        GLsizei length;
        GLenum type;
        GLint size;
        glGetActiveUniform(prg, i, uniformBufferLength, &length, &size, &type, buf);
        ProgramUniform u = {
            buf,
            type, 
            size
        };
        buf = buf + length + 1;
        vector_push_U(&p->uniforms, u);
    }
    realloc(bufstart, buf - bufstart + 1);

    return p;
}

Program * program_init_quick(Program * p, const char * vert_source, const char * frag_source) {
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
	program_init_quick(p, source, source);
    free(source);
    return p;
}

void program_deinit(Program * p) {
    if (p->attributes.count > 0) {
        free(vector_ptr_A(&p->attributes, 0)->name);
    }
    if (p->uniforms.count > 0) {
        free(vector_ptr_U(&p->uniforms, 0)->name);
    }
    vector_deinit_A(&p->attributes);
    vector_deinit_U(&p->uniforms);
    glDeleteProgram(p->id);
}

ProgramAttribute * program_attribute(Program * p, unsigned index) {
    return vector_ptr_A(&p->attributes, index);
}

ProgramUniform * program_uniform(Program * p, unsigned index) {
    return vector_ptr_U(&p->uniforms, index);
}

int program_find_atribute(Program * p, const char * name) {
    for (int i = 0; i < p->attributes.count; i++) {
        if (!strcmp(name, program_attribute(p, i)->name))
            return i;
    }
    return -1;
}

int program_find_uniform(Program * p, const char * name) {
    for (int i = 0; i < p->uniforms.count; i++) {
        if (!strcmp(name, program_uniform(p, i)->name))
            return i;
    }
    return -1;
}
