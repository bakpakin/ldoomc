/*
 * The main class for storing a 3d scene. Includes rendering, physics, collision,
 * and Mob information. Basically, the meat of the game state.
 */

#ifndef SCENE_HEADER
#define SCENE_HEADER

#include "drenderer.h"
#include "grid3d.h"
#include "vector.h"
#include "mob.h"
#include "ldmath.h"

typedef struct {

    DRenderer * renderer;
    Vector mobs;
    Grid mobGrid;
    Grid staticGrid;

} Scene;

#endif
