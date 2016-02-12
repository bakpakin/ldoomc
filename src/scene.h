/*
 * The main type for storing a 3d scene. Includes rendering, physics, collision,
 * and Mob information. Basically, the meat of the game state.
 */

#ifndef SCENE_HEADER
#define SCENE_HEADER

#include "grid3d.h"
#include "geom.h"
#include "vector.h"
#include "spatial.h"
#include "ldmath.h"
#include "camera.h"
#include "opool.h"

typedef struct {

    Flexpool mobpool;

    Vector mobs;
    Vector statics;

    Camera camera;

    // Buffers for deferred rendering
    unsigned diffuseBuffer;
    unsigned normalBuffer;
    unsigned specularBuffer;
    unsigned lightBuffer;

    Grid solidGrid;

    double timeBuffer;

} Scene;

Scene * scene_init(Scene * s);

void scene_deinit(Scene * s);

// Mobs
Mob * scene_add_mob(Scene * s, MobDef * type, vec3 position);

void scene_remove_mob(Scene * s, Mob * mob);

void scene_free_mob(Scene * s, Mob * mob);

// Update with constant interval for reliable physics.
void scene_update(Scene * s, double dt);

void scene_render(Scene * s);

#endif
