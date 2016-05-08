#ifndef MODEL_HEADER
#define MODEL_HEADER

#include "mesh.h"
#include "texture.h"
#include "glfw.h"
#include <stdint.h>

#define MODEL_OWNS_MATERIALS_BIT 0x01
#define MODEL_OWNS_VERTICIES_BIT 0x02
#define MODEL_OWNS_BONES_BIT 0x04
#define MODEL_OWNS_ANIMATIONS_BIT 0x08
#define MODEL_OWNS_MESHES_BIT 0x10
#define MODEL_OWNS_TRIANGLES_BIT 0x20
#define MODEL_OWNS_TEXT_BIT 0x40

#define MODEL_ANIMATION_LOOPS_BIT 0x01

typedef struct ModelVertex {

    float position[3];
    float normal[3];
    float texcoord[2];
    uint8_t boneIndicies[4];
    uint8_t boneWeights[4];

} ModelVertex;

typedef struct ModelBone {

    uint32_t name;
    int32_t parent; // index, less than 0 is a root bone.

    float restingPosition[3];
    float restingRotation[4];
    float restingScale[3];

} ModelBone;

typedef struct ModelTriangle {

    uint32_t verts[3];

} ModelTriangle;

typedef struct ModelBonePose {

    float position[3];
    float rotation[4];
    float scale[3];

} ModelBonePose;

typedef struct ModelAnimation {

    uint32_t name;
    uint32_t flags;
    float framerate;

    uint32_t start;
    uint32_t count;

} ModelAnimation;

typedef struct ModelMaterial {

    Texture diffuse;
    Texture normal;
    Texture specular;

} ModelMaterial;

typedef struct ModelMesh {

    uint32_t name;
    uint32_t materialid;
    uint32_t firstVertex;
    uint32_t vertexCount;
    uint32_t firstTriangle;
    uint32_t triangleCount;

} ModelMesh;

typedef struct Model {

    uint32_t flags;

    uint32_t vertexCount;        ModelVertex * vertices;
    uint32_t triangleCount;      ModelTriangle * triangles;
    uint32_t materialCount;      ModelMaterial * materials;
    uint32_t boneCount;          ModelBone * bones;
    uint32_t animationCount;     ModelAnimation * animations;
    uint32_t meshCount;          ModelMesh * meshes;
    uint32_t frameCount;         ModelBonePose * frames;

    size_t textSize;
    uint8_t * textData;

} Model;

typedef struct ModelInstance {

    uint32_t flags;

    Model * model;
    ModelVertex * mesh;

    uint32_t animation;
    uint32_t frame;

} ModelInstance;

int model_loadfile(Model * model, const char * file);

int model_loadresource(Model * model, const char * resource);

void model_deinit(Model * model);

void model_drawdebug(Model * model);

ModelInstance * model_instance(Model * mode, ModelInstance * instance);

void modeli_deinit(ModelInstance * instance);

void modeli_set_animation(ModelInstance * instance);

void modeli_update(ModelInstance * instance, float dt);

void modeli_drawdebug(ModelInstance * instance); // Simply draws the diffuse color for all textures.

#endif
