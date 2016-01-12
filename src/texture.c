#include "texture.h"
#include "util.h"
#include <stdlib.h>
#include "lodepng.h"
#include "platform.h"

Texture * texture_init_file(Texture * t, const char * path, int pathlen) {

    unsigned error;
    unsigned char* image;
    unsigned width, height;

    #define BUFLEN 40
    char buf[BUFLEN];

    if (pathlen >= 0) {
        if (pathlen + 1 > BUFLEN)
            uerr("Texture path buffer overflow.");
        path = buf;
        strncpy(buf, path, pathlen);
    }

    error = lodepng_decode32_file(&image, &width, &height, path);
    if(error)
        uerr(lodepng_error_text(error));

    glGenTextures(1, &t->id);
    glBindTexture(GL_TEXTURE_2D, t->id);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

    glGenerateMipmap(GL_TEXTURE_2D);

    free(image);

    t->w = width;
    t->h= height;

    return t;
}

/*
 * Loads a tetxture from a named resource.
 */
Texture * texture_init_resource(Texture * t, const char * resource) {
    char file[200];
    platform_res2file(resource, file, 200);
    return texture_init_file(t, file, -1);
}

void texture_deinit(Texture * t) {
    glDeleteTextures(1, &t->id);
}
