#ifndef MOB_HEADER
#define MOB_HEADER

#include "ldmath.h"
#include "model.h"
#include "geom.h"
#include "camera.h"

#define MOB_INVISIBLE 1
#define MOB_GROUNDED 2
#define MOB_INHERITED_FLAGS (MOB_INVISIBLE)

typedef struct {

    // Basic capabilities
    float starting_health; // Starting health
    float speed;
    float weight;
    float height;
    float radius;
    float jump;
    unsigned flags;

    // AI parameters
    float agression_radius;
    float aggression;

    // Rendering
    Model model;

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
    vec3 velocity;
    vec3 facing;

    // Game Information
    float health;
    float speed;
    float jump;

    // Render implementation
    int renderid;

} Mob;

// Spatials are like mobs that don't interact with anything. Use them for clouds or something.
typedef struct {

    Model * modelptr;

    vec3 position;
    quat rotation;

    // Render implementation
    int renderid;

} Spatial;

// Brushes are static geomotry in a scene.
typedef struct {

    Model * modelptr;

    Prism body;

} Brush;

// Mobs: Method like functions
Mob * mob_init(Mob * m, MobDef * md);

void mob_deinit(Mob * m);

void mob_look(Mob * m, float yaw, float pitch);

void mob_displacement_flat(Mob * m, vec3 out, float forward, float strafe);

void mob_cameralook(Mob * m, Camera * c);

#endif
