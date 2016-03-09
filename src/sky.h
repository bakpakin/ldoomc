#ifndef SKY_HEADER
#define SKY_HEADER

#include "camera.h"
#include "texture.h"

// Useful for environment mapping.
extern Texture sky_texture;

void sky_init();

void sky_deinit();

void sky_render();

#endif
