#ifndef TEXTURE_HEADER
#define TEXTURE_HEADER

#include "glfw.h"

typedef struct {
    GLuint id;
    GLuint type;
    unsigned w;
    unsigned h;
    unsigned d;
} Texture;

/*
 * Loads a 2d texture from a file. If pathlen < 0, assumes a null-terminated string.
 */
Texture * texture_init_file(Texture * t, const char * path, int pathlen);

/*
 * Loads a 2d texture from a named resource.
 */
Texture * texture_init_resource(Texture * t, const char * resource);

/*
 * Loads a cubemap from 6 images.
 */
Texture * texture_cube_init_file(Texture * t, const char * paths[6]);

/*
 * Loads a cubemap from 6 resources.
 */
Texture * texture_cube_init_resource(Texture * t, const char * resources[6]);

void texture_deinit(Texture * t);

#endif
