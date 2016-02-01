#ifndef MODEL_HEADER
#define MODEL_HEADER

#include "mesh.h"
#include "texture.h"

typedef struct {

    Mesh mesh;

    Texture normal;
    Texture diffuse;
    Texture specular;

} Model;

#endif
