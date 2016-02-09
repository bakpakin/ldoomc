/*
 * The main type for storing a 3d scene. Includes rendering, physics, collision,
 * and Mob information. Basically, the meat of the game state.
 */

#ifndef SCENE_HEADER
#define SCENE_HEADER

#include "grid3d.h"
#include "geom.h"
#include "vector.h"
#include "mob.h"
#include "ldmath.h"
#include "camera.h"
#include "opool.h"

typedef struct {

    Flexpool mobpool;

    Vector mobs;

    Camera camera;

    // Buffers for deferred rendering
    unsigned diffuseBuffer;
    unsigned normalBuffer;
    unsigned specularBuffer;
    unsigned lightBuffer;

    Grid mobGrid;
    Grid staticGrid;

} Scene;

Scene * scene_init(Scene * s);

void scene_deinit(Scene * s);

Mob * scene_add_mob(Scene * s, MobDef * type, vec3 position);

void scene_remove_mob(Scene * s, Mob * mob);

void scene_free_mob(Scene * s, Mob * mob);

void scene_update(Scene * s, double dt);

void scene_render(Scene * s);

#endif
