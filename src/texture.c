#include "texture.h"
#include "util.h"
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#define STB_ONLY_PNG
// Undo only png define if we decide to use other images
#include "stb_image.h"
#include "platform.h"

static unsigned char * loadImage(const char * path, int pathlen, unsigned * w, unsigned * h) {

    int width, height, comp;
    unsigned char * image = stbi_load(path, &width, &height, &comp, STBI_rgb_alpha);

    if (image != NULL) {
        *w = width;
        *h = height;
    }

	return image;

}

Texture * texture_init_file(Texture * t, const char * path, int pathlen) {

    unsigned width, height;
    unsigned char * image = loadImage(path, pathlen, &width, &height);

    glGenTextures(1, &t->id);
    glBindTexture(GL_TEXTURE_2D, t->id);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

    glGenerateMipmap(GL_TEXTURE_2D);

    free(image);

    t->w = width;
    t->h= height;
    t->type = GL_TEXTURE_2D;

    return t;
}

/*
 * Loads a tetxture from a named resource.
 */
Texture * texture_init_resource(Texture * t, const char * resource) {
    return texture_init_file(t, platform_res2file_ez(resource), -1);
}

/*
 * Loads a cubemap from 6 images.
 */
Texture * texture_cube_init_file(Texture * t, const char * paths[6]) {
	GLuint textureID;
	glGenTextures(1, &textureID);
	t->id = textureID;
    glActiveTexture(GL_TEXTURE0);

    unsigned width, height;
    unsigned char* image;

    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    for(GLuint i = 0; i < 6; i++) {
        image = loadImage(paths[i], -1, &width, &height);
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
            GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image
        );
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	t->w = width;
	t->h = height;
	t->type = GL_TEXTURE_CUBE_MAP;

	return t;
}

/*
 * Loads a cubemap from 6 resources.
 */
Texture * texture_cube_init_resource(Texture * t, const char * resources[6]) {
    char files[6][100];
	for (int i = 0; i < 6; i++) {
		platform_res2file(resources[i], files[i], 200);
	}
	const char * parray[6] = {
		files[0],
		files[1],
		files[2],
		files[3],
		files[4],
		files[5]
	};
    return texture_cube_init_file(t, parray);
}

void texture_deinit(Texture * t) {
    glDeleteTextures(1, &t->id);
}
