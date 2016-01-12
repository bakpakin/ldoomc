#ifndef MOB_HEADER
#define MOB_HEADER

#include "ldmath.h"
#include "texture.h"
#include "mesh.h"
#include "shader.h"

typedef struct {

    // Basic capabilities
    float starting_health; // Starting health
    float speed;
    float weight;
    float height;
    float radius;

    // AI parameters
    float agression_radius;
    float aggression;

    // Rendering
    Program program;
    Mesh mesh;
    Texture diffuse;
    Texture normal;
    int diffuseLocation;
    int normalLocation;

    // User defined data.
    void * user;

} MobDef;

// Mobs are represented by cylinders
typedef struct {

    // Defins the type of the Mob. Many mobs should share one MobDef.
    MobDef * type;

    // Spatial Information
    vec3 position;
    vec3 velocity;
    vec3 facing;

    // Game Information
    float health;

} Mob;

#endif
