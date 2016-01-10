#ifndef CAMERA_HEADER
#define CAMERA_HEADER

#include "ldmath.h"

typedef enum {

    CAMERATYPE_PERSPECTIVE,
    CAMERATYPE_ORTHOGRAPHIC

} CameraType;

typedef struct {

    // Describes the orientation and position of the camera. Used at the arguments to mat4_look_vec
    vec3 position;
    vec3 direction;
    vec3 upDirection;

    // The projection type of tha camera. Orthographic or perspective
    CameraType type;

    // A dirty bit used to detect when matrices need to be recomputed.
    unsigned char dirty;

    // The stored precomputed matrices. Matrix is the final matrix, while projection is the just the projection matrix
    // created by mat4_proj_ortho or mat4_proj_perspective.
    mat4 projection;
    mat4 matrix;

    //Bounding box of the projection frustum
    aabb3 bounds;

    // Data specific to the projection matrix.
    union {

        // Data to construct a perspective projection matri
        struct {
            float fovY;
            float aspect;
            float znear;
            float zfar;
        } perspective;

        // Data to construct an orthographic projection matrix.
        struct {
            float width;
            float height;
            float depth;
        } orthographic;

    } data;

} Camera;

Camera * camera_init_ortho(Camera * c,
        float width, float height, float depth);

Camera * camera_init_perspective(Camera * c,
        float fovY, float aspect, float zNear, float zFar);

CameraType camera_get_type(const Camera * c);

void camera_frustum_bounds(Camera * m, aabb3 out);

void camera_update(Camera * c);

void camera_set_perspective(Camera * c, float fovY, float aspect, float zNear, float zFar);

void camera_set_ortho(Camera * c, float width, float height, float depth);

void camera_set_position(Camera * c, const vec3 position);

void camera_set_direction(Camera * c, const vec3 direction);

void camera_set_up(Camera * c, const vec3 up);

#endif
