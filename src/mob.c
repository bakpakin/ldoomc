#include "mob.h"

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
    md->friction = 0;
    md->restitution = 1;
    md->model = NULL;
    md->speed = 1;

    return md;
}

// Mobs: Method like functions
Mob * mob_init(Mob * m, MobDef * md, const vec3 pos) {
    static const vec3 fw = {1, 0, 0};
    static const vec3 zero = {0, 0, 0};

    m->type = md;
    m->flags = md->flags & MOB_INHERITED_FLAGS;

    vec3_assign(m->facing, fw);
    vec3_assign(m->position, pos);
    vec3_assign(m->velocity, zero);
    vec3_assign(m->_acceleration, zero);

    m->health = md->starting_health;
    m->speed = md->speed;
    m->jump = md->jump;
    m->friction = md->friction;

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

void mob_cameralook(Mob * m, Camera * c) {

    camera_set_position(c, m->position);
    camera_set_direction(c, m->facing);

}

void mob_apply_force(Mob * m, const vec3 force) {
    vec3_addmul(m->_acceleration, m->_acceleration, force, vec3_len(force) * m->type->inv_mass);
}

void mob_apply_impulse(Mob * m, const vec3 impulse) {
    vec3_add(m->_acceleration, m->_acceleration, impulse);
}

void mob_limit_speed(Mob * m, float maxspeed) {
    float speed2 = vec3_len2(m->velocity);
    if (speed2 > maxspeed * maxspeed) {
        float scale = maxspeed / sqrtf(speed2);
        vec3_scale(m->velocity, m->velocity, scale);
    }
}

void mob_limit_hspeed(Mob * m, float maxspeed) {
    float speed2 = m->velocity[0] * m->velocity[0] + m->velocity[2] * m->velocity[2];
    if (speed2 > maxspeed * maxspeed) {
        float scale = maxspeed / sqrtf(speed2);
        m->velocity[0] *= scale;
        m->velocity[2] *= scale;
    }
}

void mob_limit_vspeed(Mob * m, float maxspeed) {
    if (fabs(m->velocity[1]) > maxspeed) {
        m->velocity[1] = (m->velocity[1] > 0 ? 1 : -1) * maxspeed;
    }
}

void mob_apply_friction(Mob * m, float scale) {
    // Apply the force of friction
    vec3 fric;
    fric[0] = m->velocity[0]; fric[1] = 0; fric[2] = m->velocity[2];
    float vellen = vec3_len(fric);
    if (vellen <= scale || vellen < 0.0000001f) {
        vec3_scale(fric, fric, -1);
    } else {
        float vel_fric_scale = -m->friction / vellen;
        vec3_scale(fric, fric, vel_fric_scale);
    }
    mob_apply_impulse(m, fric);
}

void mob_impulse_move(Mob * m, float forward, float strafe) {
    vec3 normforward;
    vec3 normstrafe;
    vec3_assign(normforward, m->facing);
    normforward[1] = 0;
    vec3_norm(normforward, normforward);
    normstrafe[1] = 0;
    normstrafe[0] = normforward[2];
    normstrafe[2] = -normforward[0];
    vec3_scale(normstrafe, normstrafe, strafe);
    vec3_scale(normforward, normforward, forward);
    vec3_add(normforward, normforward, normstrafe);
    mob_apply_impulse(m, normforward);
}
