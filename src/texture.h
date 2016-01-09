#ifndef TEXTURE_HEADER
#define TEXTURE_HEADER

#include "glfw.h"

typedef struct {
    GLuint id;
    unsigned w;
    unsigned h;
} Texture;

/*
 * Loads a texture from a file. If pathlen < 0, assumes a nul-terminated string.
 */
Texture * texture_init_file(Texture * t, const char * path, int pathlen);

void texture_deinit(Texture * t);

#endif
