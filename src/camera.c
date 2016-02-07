#include "camera.h"

#define PROJECTION_DIRTY_BIT 0x01
#define LOOKAT_DIRTY_BIT 0x02

static void camera_update_projection(Camera * c) {
    float w2, h2, d;
    float fovY, aspect, znear, zfar;
    switch (c->type) {
        case CAMERATYPE_ORTHOGRAPHIC:
            w2 = c->data.orthographic.width / 2;
            h2 = c->data.orthographic.height / 2;
            d = c->data.orthographic.depth;
            mat4_proj_ortho(c->projection, -w2, w2, h2, -h2, 0, d);
            break;
        case CAMERATYPE_PERSPECTIVE:
            fovY = c->data.perspective.fovY;
            aspect = c->data.perspective.aspect;
            znear = c->data.perspective.znear;
            zfar = c->data.perspective.zfar;
            mat4_proj_perspective(c->projection, fovY, aspect, znear, zfar);
            break;
    }
}

void camera_frustum_bounds(Camera * c, aabb3 out) {
    camera_update(c);
    c->dirty = 0;
}

static void camera_update_matrix(Camera * c) {
    mat4 look;
    mat4_look_vec(look, c->position, c->direction, c->upDirection);
    mat4_mul(c->matrix, look, c->projection);
}

CameraType camera_get_type(const Camera * c) {
    return c->type;
}

void camera_set_perspective(Camera * c, float fovY, float aspect, float zNear, float zFar) {
    c->type = CAMERATYPE_PERSPECTIVE;
    c->data.perspective.fovY = fovY;
    c->data.perspective.aspect = aspect;
    c->data.perspective. znear = zNear;
    c->data.perspective.zfar = zFar;
    c->dirty |= PROJECTION_DIRTY_BIT;
}

void camera_set_ortho(Camera * c, float width, float height, float depth) {
    c->type = CAMERATYPE_ORTHOGRAPHIC;
    c->data.orthographic.width = width;
    c->data.orthographic.height = height;
    c->data.orthographic.depth = depth;
    c->dirty |= PROJECTION_DIRTY_BIT;
}

void camera_set_position(Camera * c, const vec3 position) {
    vec3_assign(c->position, position);
    c->dirty |= LOOKAT_DIRTY_BIT;
}

void camera_set_direction(Camera * c, const vec3 direction) {
    vec3_assign(c->direction, direction);
    c->dirty |= LOOKAT_DIRTY_BIT;
}

void camera_set_up(Camera * c, const vec3 up) {
    vec3_assign(c->upDirection, up);
    c->dirty |= LOOKAT_DIRTY_BIT;
}

static void cam_init(Camera * c) {
    static const vec3 zero = {0, 0, 0};
    static const vec3 up = {0, 1, 0};
    static const vec3 forward = {0, 0, 1};
    vec3_assign(c->position, zero);
    vec3_assign(c->upDirection,  up);
    vec3_assign(c->direction, forward);
    c->dirty = PROJECTION_DIRTY_BIT | LOOKAT_DIRTY_BIT;
}

Camera * camera_init_ortho(Camera * c,
        float width, float height, float depth) {
    camera_set_ortho(c, width, height, depth);
    cam_init(c);
    return c;
}

Camera * camera_init_perspective(Camera * c,
        float fovY, float aspect, float zNear, float zFar) {
    camera_set_perspective(c, fovY, aspect, zNear, zFar);
    cam_init(c);
    return c;
}

void camera_update(Camera * c) {
    unsigned char dirty = c->dirty;
    if (dirty & PROJECTION_DIRTY_BIT) {
        camera_update_projection(c);
    }
    if (dirty & (LOOKAT_DIRTY_BIT | PROJECTION_DIRTY_BIT)) {
        camera_update_matrix(c);
    }
    c->dirty = 0;
}

void camera_calc_mvp(Camera * c, mat4 mvp, vec3 position) {
    camera_update(c);
    mat4 translationMatrix;
    mat4_translation_vec3(translationMatrix, position);
    mat4_mul(mvp, translationMatrix, c->matrix);
}

#undef PROJECTION_DIRTY_BIT
#undef LOOKAT_DIRTY_BIT
