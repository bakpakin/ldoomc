#ifndef MOB_HEADER
#define MOB_HEADER

#include "ldmath.h"
#include "model.h"
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
    float walk_accel;
    float restitution;
    float friction;
    unsigned flags;

    // AI parameters
    float agression_radius;
    float aggression;

    // Rendering
    ModelInstance * model;

    // User defined data.
    void * user;

} MobDef;

// Mobs are represented by cylinders
typedef struct {

    unsigned flags;

    // Defines the type of the Mob. Many mobs should share one MobDef.
    MobDef * type;

    // Spatial Information; You can change these willy nilly
    vec3 position;
    vec3 velocity;

    // Physics Implementation; Don't change these willy nilly
    vec3 _position_penalty;
    vec3 _acceleration;

    vec3 facing;

    // Game Information
    float friction;
    float walk_acccel;
    float health;
    float speed;
    float jump;

    // Scene implementation
    unsigned sceneIndex;

} Mob;

MobDef * mobdef_init(MobDef * md);

// Mobs: Method like functions
Mob * mob_init(Mob * m, MobDef * md, const vec3 pos);

void mob_deinit(Mob * m);

void mob_apply_force(Mob * m, const vec3 force);

void mob_apply_impulse(Mob * m, const vec3 impulse);

void mob_limit_speed(Mob * m, float maxspeed);

void mob_limit_hspeed(Mob * m, float maxspeed);

void mob_limit_vspeed(Mob * m, float maxspeed);

void mob_apply_friction(Mob * m, float scale);

void mob_impulse_move(Mob * m, float forward, float strafe);

void mob_look(Mob * m, float yaw, float pitch);

void mob_cameralook(Mob * m, Camera * c);

#endif
