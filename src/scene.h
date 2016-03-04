/*
 * The main module for storing a 3d scene. Includes rendering, physics, collision,
 * and Mob information. Basically, the meat of the game state.
 */

#ifndef SCENE_HEADER
#define SCENE_HEADER

#include "geom.h"
#include "vector.h"
#include "mob.h"
#include "ldmath.h"
#include "camera.h"
#include "opool.h"

extern Camera scene_camera;

void scene_init();

void scene_deinit();

// Mobs
void scene_add_mob(Mob * mob);

void scene_remove_mob(Mob * mob);

// Static Models
void scene_add_model(Model * model);

void scene_remove_model(Model * model);

// Events
void scene_update(double dt);

void scene_render();

void scene_resize(int width, int height);

#endif
