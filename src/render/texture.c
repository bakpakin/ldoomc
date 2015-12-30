#include "texture.h"
#include "util.h"
#include <stdlib.h>
#include "lodepng.h"

Texture * texture_init_file(Texture * t, const char * path) {

    unsigned error;
    unsigned char* image;
    unsigned width, height;

    error = lodepng_decode32_file(&image, &width, &height, path);
    if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

    glGenTextures(1, &t->id);
    glBindTexture(GL_TEXTURE_2D, t->id);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

    glGenerateMipmap(GL_TEXTURE_2D);

    free(image);

    return t;
}

void texture_deinit(Texture * t) {
    glDeleteTextures(1, &t->id);
}
