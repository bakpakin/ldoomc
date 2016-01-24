#ifndef MODEL_HEADER
#define MODEL_HEADER

#include "mesh.h"
#include "ldmath.h"
#include "texture.h"
#include "shader.h"
#include "modelshader.h"

typedef struct {

    Mesh mesh;

    Texture normal;
    Texture diffuse;
    Texture specular;

    ModelShader * modelShader;

} Model;

typedef struct TextureLocationPair TextureLocationPair;

Model * model_init(Model * m,
        const Mesh * mesh,
        const Texture * normal,
        const Texture * diffuse,
        const Texture * specular,
        ModelShader * ms);

void model_deinit(Model * m);

void model_draw(Model * m, mat4 mvp, float time);

#endif
