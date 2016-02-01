#ifndef DEFERRED_RENDERER_HEADER
#define DEFERRED_RENDERER_HEADER

#include "vector.h"
#include "model.h"
#include "mob.h"
#include "camera.h"

// Describes how a DRenderer draws mobs.
typedef enum {
    RENDERMODE_ALL,
    RENDERMODE_DIFFUSE,
    RENDERMODE_NORMAL, // This is NOT the default mode, this shows mob normals.
    RENDERMODE_SPECULAR,
    RENDERMODE_WIREFRAME
} RenderMode;

typedef struct {

    Vector mobs;

    RenderMode mode;

    Camera camera;

    // Buffers for deferred rendering
    unsigned diffuseBuffer;
    unsigned normalBuffer;
    unsigned specularBuffer;
    unsigned lightBuffer;

} DRenderer;

DRenderer * drenderer_init(DRenderer * dr);

void drenderer_deinit(DRenderer * dr);

void drenderer_add(DRenderer * dr, Mob * m);

void drenderer_remove(DRenderer * dr, Mob * m);

void drenderer_render(DRenderer * dr);

void drenderer_resize(DRenderer * dr, int width, int height);

#endif
