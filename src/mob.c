#include "mob.h"

// Mobs: Method like functions
Mob * mob_init(Mob * m, MobDef * md) {
    static const vec3 fw = {1, 0, 0};
    static const vec3 zero = {0, 0, 0};

    m->flags = md->flags & MOB_INHERITED_FLAGS;

    vec3_assign(m->facing, fw);
    vec3_assign(m->velocity, zero);
    vec3_assign(m->position, zero);

    m->health = md->starting_health;
    m->speed = md->speed;
    m->jump = md->jump;

    return m;
}

void mob_deinit(Mob * m) {
    ; // Currently a noop
}

void mob_look(Mob * m, float yaw, float pitch) {
    vec3 direction;
    direction[0] = cosf(pitch) * cosf(yaw);
    direction[1] = sinf(pitch);
    direction[2] = cosf(pitch) * sinf(yaw);
    vec3_assign(m->facing, direction);
}

void mob_displcement_flat(Mob * m, vec3 out, float forward, float strafe) {

    float invcpitch = 1.0f / sqrtf(1.0 - (m->facing[1] * m->facing[1]));
    float cyaw = m->facing[0] * invcpitch;
    float syaw = m->facing[2] * invcpitch;

    out[0] = (strafe * syaw + forward * cyaw);
    out[1] = 0;
    out[2] = (forward * syaw - strafe * cyaw);

}

void mob_cameralook(Mob * m, Camera * c) {

    camera_set_position(c, m->position);
    camera_set_direction(c, m->facing);

}
