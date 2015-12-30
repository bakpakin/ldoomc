#ifndef TEXTURE_HEADER
#define TEXTURE_HEADER

#include "config.h"

#include OPENGL_H

typedef struct {
    GLuint id;
    unsigned w;
    unsigned h;
} Texture;

Texture * texture_init_file(Texture * t, const char * path);

void texture_deinit(Texture * t);

#endif
