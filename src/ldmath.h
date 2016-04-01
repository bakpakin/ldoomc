/*
Math functions inspired by Kazmath (https://github.com/Kazade/kazmath)
*/
#ifndef LD_MATH_HEADER
#define LD_MATH_HEADER

#include <math.h>
#include <float.h>
#include <stdlib.h>

#define LD_PI 3.1415926535897932384626f
#define LD_180_OVER_PI (180.0f / LD_PI)
#define LD_PI_OVER_180 (LD_PI / 180.0f)

// float function
float ldm_min(float a, float b);
float ldm_max(float a, float b);
float ldm_clamp(float x, float min, float max);
float ldm_lerp(float a, float b, float t);
int ldm_almost_equal(float a, float b);

/*
 * Declares a vector type with n components.
 */
#define def_vecn(n) \
typedef float vec##n[n]; \
void vec##n##_add(vec##n out, vec##n const a, vec##n const b); \
void vec##n##_sub(vec##n out, vec##n const a, vec##n const b); \
float vec##n##_dot(const vec##n a, const vec##n b); \
void vec##n##_assign(vec##n out, const vec##n in); \
void vec##n##_scale(vec##n out, const vec##n in, float s); \
float vec##n##_len2(const vec##n v); \
float vec##n##_len(const vec##n v); \
int vec##n##_norm(vec##n out, const vec##n in); \
void vec##n##_addmul(vec##n out, const vec##n in, const vec##n a, float s); \
int vec##n##_equal(const vec##n a, const vec##n b); \
int vec##n##_almost_equal(const vec##n a, const vec##n b); \
void vec##n##_max(vec##n out, const vec##n a, const vec##n b); \
void vec##n##_min(vec##n out, const vec##n a, const vec##n b);
def_vecn(2);
def_vecn(3);
def_vecn(4);
#undef def_vecn

// Special vector definitions
float vec2_crossl(const vec2 a, const vec2 b);
void vec3_cross(vec3 out, const vec3 a, const vec3 b);

#define def_aabbn(n) \
typedef vec##n aabb##n[2]; \
int aabb##n##_check(aabb##n a); \
void aabb##n##_fill(aabb##n out, const vec##n min, const vec##n max); \
void aabb##n##_assign(aabb##n out, const aabb##n in); \
void aabb##n##_translate(aabb##n out, const aabb##n in, vec##n d); \
int aabb##n##_union(aabb##n out, const aabb##n a, const aabb##n b); \
int aabb##n##_difference(aabb##n out, const aabb##n a, const aabb##n b); \
int aabb##n##_contains(const aabb##n a, const vec##n p); \
int aabb##n##_overlaps(const aabb##n a, const aabb##n b); \
void aabb##n##_minkowksi(aabb##n out, const aabb##n a, const aabb##n b); \

def_aabbn(2);
def_aabbn(3);
def_aabbn(4);

#undef def_aabbn

#define def_matn(n) \
typedef float mat##n[n * n]; \
float mat##n##_get(const mat##n m, int row, int col); \
void mat##n##_fill(mat##n out, const float * data); \
void mat##n##_identity(mat##n out); \
void mat##n##_mul(mat##n out, const mat##n a, const mat##n b); \
void mat##n##_transpose(mat##n out, const mat##n m); \
int mat##n##_equal(const mat##n a, const mat##n b); \
int mat##n##_almost_equal(const mat##n a, const mat##n b); \

def_matn(2);
def_matn(3);
def_matn(4);

#undef def_matn

// Quaternions

typedef float quat[4];

void quat_identity(quat q);
void quat_scale(quat q, float scale);
void quat_add(quat out, const quat a, const quat b);
void quat_sub(quat out, const quat a, const quat b);
void quat_mul(quat r, quat p, quat q);
float quat_inner_product(quat a, quat b);
void quat_2mat4(const quat q, mat4 m);
void quat_norm(quat q);
void quat_conj(quat out, const quat a);
void quat_rot(quat q, vec3 axis, float angle);
void quat_mul_vec3(vec3 r, quat q, vec3 v);

// Transformations

void mat4_rot_x(mat4 out, float rad);
void mat4_rot_y(mat4 out, float rad);
void mat4_rot_z(mat4 out, float rad);
void mat4_rot_ypr(mat4 out, float yaw, float pitch, float roll);
void mat4_scaling(mat4 out, float x, float y, float z);
void mat4_scaling_vec3(mat4 out, const vec3 scale);
void mat4_translation(mat4 out, float x, float y, float z);
void mat4_translation_vec3(mat4 out, const vec3 translate);

// Projections

void mat4_proj_perspective(mat4 out, float fovY, float aspect, float zNear, float zFar);
void mat4_proj_ortho(mat4 out,
        float left, float right,
        float bottom, float top,
        float neardist, float fardist);
void mat4_look_vec(mat4 out, const vec3 eye, const vec3 direction, const vec3 up);
void mat4_look(mat4 out, float eye_x, float eye_y, float eye_z,
        float direction_x, float direction_y, float direction_z,
        float up_x, float up_y, float up_z);

#endif /* end of include guard: LD_MATH_HEADER */
