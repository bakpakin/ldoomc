#ifndef SPATIAL_HEADER
#define SPATIAL_HEADER

#include "ldmath.h"
#include "model.h"
#include "geom.h"
#include "camera.h"

#define MOB_INVISIBLE 1
#define MOB_GROUNDED 2
#define MOB_NOCOLLIDE 4
#define MOB_INHERITED_FLAGS (MOB_INVISIBLE)

typedef struct {

    // Basic capabilities
    float starting_health; // Starting health
    float speed;
    float inv_mass;
    float height;
    float radius;
    float jump;
    float continuation;
    float squishyness;
    unsigned flags;

    // AI parameters
    float agression_radius;
    float aggression;

    // Rendering
    Model * model;

    // User defined data.
    void * user;

} MobDef;

// Mobs are represented by cylinders
typedef struct {

    unsigned flags;

    // Defins the type of the Mob. Many mobs should share one MobDef.
    MobDef * type;

    // Spatial Information
    vec3 position;
    vec3 prev_position;
    vec3 impulse; // Set impulse to move mob around
    vec3 facing;

    // Game Information
    float continuation;
    float health;
    float speed;
    float jump;

    // Scene implementation
    unsigned sceneIndex;

} Mob;

// Static geometry in scene
typedef struct {
    mat4 matrix;
    Model * model;
} Static;

MobDef * mobdef_init(MobDef * md);

// Mobs: Method like functions
Mob * mob_init(Mob * m, MobDef * md, const vec3 pos);

void mob_deinit(Mob * m);

void mob_look(Mob * m, float yaw, float pitch);

void mob_displacement_flat(Mob * m, vec3 out, float forward, float strafe);

void mob_cameralook(Mob * m, Camera * c);

#endif
