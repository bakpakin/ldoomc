#ifndef MODEL_HEADER
#define MODEL_HEADER

#include "mesh.h"
#include "texture.h"
#include "ldmath.h"
#include "glfw.h"
#include <stdint.h>

#define MODEL_OWNS_MATERIALS_BIT 0x01
#define MODEL_OWNS_VERTICIES_BIT 0x02
#define MODEL_OWNS_BONES_BIT 0x04
#define MODEL_OWNS_ANIMATIONS_BIT 0x08
#define MODEL_OWNS_RANGES_BIT 0x10
#define MODEL_OWNS_TRIANGLES_BIT 0x20

#define MODEL_ANIMATION_LOOPS_BIT 0x01

typedef struct ModelBone {

    uint32_t id;
    int32_t parent; // less than 0 is a root bone.

    uint32_t vertexCount;
    struct {
        uint32_t id;
        float weight;
    } verts[0];

} ModelBone;

typedef struct ModelBonePose {

    vec3 position;
    vec3 scale;
    quat rotation;

} ModelBonePose;

typedef struct ModelAnimation {

    uint32_t flags;
    float framerate;

    uint32_t frameCount;
    ModelBonePose * frames; // there should parentAnimation.boneCount * frameCount poses

} ModelAnimation;

typedef struct ModelMaterial {

    Texture diffuse;
    Texture normal;
    Texture specular;

} ModelMaterial;

typedef struct ModelMaterialRange {

    uint32_t material;
    uint32_t firstTriangle;
    uint32_t triangleCount;

} ModelMaterialRange;

typedef struct Model {

    uint32_t flags;

    uint32_t vertexCount;
    Vertex * verts;

    uint32_t triangleCount;
    uint32_t * triangles; // Triangles are groups of three vertex ids.

    uint32_t materialCount;
    ModelMaterial * materials;

    uint32_t boneCount;
    ModelBone * bones;

    uint32_t animationCount;
    ModelAnimation * animations;

    uint32_t materialRangeCounts;
    ModelMaterialRange * materialRanges;

} Model;

typedef struct ModelInstance {

    uint32_t flags;

    Model * model;
    Mesh * meshes; // There are model->materialRangesCount meshes

    uint32_t animation;
    float frame; // frame can be non integer, in which case the frame positions will be interpolated.

} ModelInstance;

Model * model_loadfile(Model * model, const char * file);

Model * model_loadresource(Model * model, const char * resource);

void model_deinit(Model * model);

ModelInstance * model_instance(Model * mode, ModelInstance * instance);

void modeli_deinit(ModelInstance * instance);

void modeli_set_animation(ModelInstance * instance);

void modeli_update(ModelInstance * instance, float dt);

void modeli_drawdebug(ModelInstance * instance); // Simply draws the diffuse color for all textures.

#endif
