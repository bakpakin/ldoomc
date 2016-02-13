#ifndef SHADER_HEADER
#define SHADER_HEADER

#include "glfw.h"

typedef struct {
    GLuint id;
    GLint type;
} Shader;

typedef struct {
    char * name;
    GLenum type;
    GLint size;
} ProgramAttribute;

typedef struct {
    char * name;
    GLenum type;
    GLint size;
} ProgramUniform;

typedef struct {
    GLuint id;
} Program;

/*
 * Initializes a shader with the given source and type. Type must be one
 * of GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, or GL_GEOMETRY_SHADER.
 */
Shader * shader_init(Shader * s, const char * source, GLint type);

/*
 * Initializes a shader by reading the source from the given file.
 */
Shader * shader_init_file(Shader * s, const char * path, GLint type);

/*
 * Initializes a shader from a resource.
 */
Shader * shader_init_resource(Shader * s, const char * resource, GLint type);

/*
 * Deinitializes the shader in the openGL context.
 */
void shader_deinit(Shader * s);

/*
 * Initializes a program with a number of shaders. When the program is
 * created, the shaders are detached but not destory. Shaders must be
 * deinitialized separately.
 */
Program * program_init(Program * p, int shader_count, ...);

/*
 * Initializes a program directly from the vertex shader source and the fragment shader source.
 */
Program * program_init_vertfrag(Program * p, const char * vert_source, const char * frag_source);

/*
 * Initializes a program from a single source file with both the fragment and vertex shaders.
 */
Program * program_init_quick(Program * p, const char * source);

/*
 * Initializes a progam from a simgle file. Use defines to define multiple shaders
 * in one file. Surround the shaders with #ifdef VERTEX for the vertex shader,
 * #ifdef FRAGMENT for the fragment shader, and #ifdef GEOMETRY geometry shaders.
 * Programs initialized this way can only have one shader of each type. For more
 * general loading, see program_init.
 */
Program * program_init_file(Program * p, const char * path);

/*
 * Initializes a program from a named resource.
 */
Program * program_init_resource(Program * p, const char * resource);

/*
 * Deinitializes a program.
 */
void program_deinit(Program * p);

#endif
