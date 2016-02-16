#include "spatial.h"

MobDef * mobdef_init(MobDef * md) {

    md->height = 10;
    md->jump = 10;
    md->flags = 0;
    md->inv_mass = 1;
    md->starting_health = 100;
    md->user = NULL;
    md->radius = 0.5;
    md->agression_radius = 30;
    md->aggression = 1;
    md->continuation = 1;
    md->model = NULL;
    md->speed = 1;
    md->squishyness = 0;

    return md;
}

// Mobs: Method like functions
Mob * mob_init(Mob * m, MobDef * md, const vec3 pos) {
    static const vec3 fw = {1, 0, 0};
    static const vec3 zero = {0, 0, 0};

    m->type = md;
    m->flags = md->flags & MOB_INHERITED_FLAGS;

    vec3_assign(m->facing, fw);
    vec3_assign(m->impulse, zero);
    vec3_assign(m->position, pos);
    vec3_assign(m->prev_position, pos);

    m->health = md->starting_health;
    m->speed = md->speed;
    m->jump = md->jump;
    m->continuation = md->continuation;

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
