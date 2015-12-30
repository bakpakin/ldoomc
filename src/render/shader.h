#ifndef SHADER_HEADER
#define SHADER_HEADER

#include "config.h"
#include OPENGL_H
#include "vector.h"

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
    Vector attributes;
    Vector uniforms;
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
 * Deinitializes the shader in the openGL context.
 */
void shader_deinit(Shader * s);

/*
 * Initializes a program with a number of shaders. When the program is
 * created, the shaders a re detached but not destory. Shaders must be
 * deinitialized separately.
 */
Program * program_init(Program * p, int shader_count, ...);

/*
 * Initializes a program directly from the vertex shader source and the fragment shader source.
 */
Program * program_init_quick(Program * p, const char * vert_source, const char * frag_source);

/*
 * Initializes a progam from a simgle file. Use defines to define multiple shaders
 * in one file. Surround the shaders with #ifdef VERTEX for the vertex shader, 
 * #ifdef FRAGMENT for the fragment shader, and #ifdef GEOMETRY geometry shaders.
 * Programs initialized this way can only have one shader of each type. For more
 * general loading, see program_init.
 */
Program * program_init_file(Program * p, const char * path);

/*
 * Deinitializes a program.
 */
void program_deinit(Program * p);

/*
 * Gets the program atribute associated with the given index.
 */
ProgramAttribute * program_attribute(Program * p, unsigned index);

/*
 * Gets the program uniform associated with the given index.
 */
ProgramUniform * program_uniform(Program * p, unsigned index);

/*
 * Find an attribute by name. Similar to glGetUniformLocation.
 */
int program_find_attribute(Program * p, const char * name);

/*
 * Find a uniform by name. Equivalent to glGetUniformLocation.
 */
int program_find_uniform(Program * p, const char * name);

#endif
