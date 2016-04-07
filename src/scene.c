#include "scene.h"
#include "ldmath.h"
#include "shader.h"
#include "platform.h"
#include "camera.h"
#include "log.h"
#include "sky.h"

Camera scene_camera;

static unsigned updates_per_frame = 1;

static Program diffuseshader;
static GLint diffuse_mvp_loc;
static GLint diffuse_diffuse_loc;
//static GLuint gBuffer;

static double timeBuffer = 0.0;

static Mob ** scene_mobs;
static size_t scene_mob_capacity;
static size_t scene_mob_count;

static Model ** scene_models;
static size_t scene_model_capacity;
static size_t scene_model_count;

void scene_init(void) {

    program_init_resource(&diffuseshader, "diffuseshader.glsl");
    diffuse_mvp_loc = glGetUniformLocation(diffuseshader.id, "u_mvp");
    diffuse_diffuse_loc = glGetUniformLocation(diffuseshader.id, "u_diffuse");

    PlatformWindow window;
    platform_get_window(&window);
    camera_init_perspective(&scene_camera, 1.5f, window.width / (float) window.height, 0.05f, 100.0f);

    scene_mob_capacity = 10;
    scene_mob_count = 0;
    scene_mobs = malloc(scene_mob_capacity * sizeof(Mob *));

    scene_model_capacity = 10;
    scene_model_count = 0;
    scene_models = malloc(scene_model_capacity * sizeof(Model *));

    timeBuffer = 0.0;

    // Generate buffers
    //glGenFramebuffers(1, &gBuffer);

    sky_init();

}

void scene_deinit() {

    // Deinitialize space for mobs
    free(scene_mobs);
    free(scene_models);

    // Deallocate buffers
    //glDeleteFramebuffers(1, &gBuffer);

    program_deinit(&diffuseshader);

    sky_deinit();
}

void scene_add_mob(Mob * mob) {
    if (scene_mob_count + 1 >= scene_mob_capacity) {
        scene_mob_capacity = 2 * scene_mob_count + 1;
        scene_mobs = realloc(scene_mobs, scene_mob_capacity * sizeof(Mob *));
    }
    scene_mobs[scene_mob_count++] = mob;
}

void scene_remove_mob(Mob * mob) {
     for (unsigned i = 0; i < scene_mob_count; i++) {
         if (scene_mobs[i] == mob) {
             scene_mob_count--;
             if (i != scene_mob_count) {
                 scene_mobs[i] = scene_mobs[scene_mob_count];
             }
             return;
         }
     }
}

void scene_add_model(Model * model) {
    if (scene_model_count + 1 >= scene_model_capacity) {
        scene_model_capacity = 2 * scene_model_count + 1;
        scene_models = realloc(scene_models, scene_model_capacity * sizeof(Model *));
    }
    scene_models[scene_model_count++] = model;
}

void scene_remove_model(Model * model) {

}

void scene_resize(int width, int height) {
    camera_set_perspective(&scene_camera, scene_camera.data.perspective.fovY, width / (float) height, 0.05f, 100.0f);
}

void scene_render() {

    // For now, just render everything as diffuse. No spatial partitioning yet.
    glUseProgram(diffuseshader.id);

    for (Mob ** mobp = scene_mobs; mobp < scene_mobs + scene_mob_count; mobp++) {
        Mob * mob = *mobp;
        Model * model = mob->type->model;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, model->diffuse.id);
        glUniform1i(diffuse_diffuse_loc, 0);
        mat4 mvp;
        mat4 rot;
        mat4_rot_y(rot, atan2(mob->facing[0], mob->facing[2]));
        mat4_translation_vec3(mvp, mob->position);
        mat4_mul(mvp, rot, mvp);
        mat4_mul(mvp, mvp, camera_matrix(&scene_camera));
        glUniformMatrix4fv(diffuse_mvp_loc, 1, GL_FALSE, mvp);
        mesh_draw(model->mesh);
    }

    for (Model ** modp = scene_models; modp < scene_models + scene_model_count; modp++) {
        Model * model = *modp;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, model->diffuse.id);
        glUniform1i(diffuse_diffuse_loc, 0);
        glUniformMatrix4fv(diffuse_mvp_loc, 1, GL_FALSE, camera_matrix(&scene_camera));
        mesh_draw(model->mesh);
    }

    // Draw sky last
    sky_render(&scene_camera);
}

#define D_EPSILON 0.000001f

static int scene_resolve_mob_collision(Mob * a, Mob * b) {
    float by = b->position[1];
    float bh = b->type->height;
    float ay = a->position[1];
    float ah = a->type->height;
    if (by + bh < ay || ay + ah < by) return 0;
    float dy;
    if (fabs(ay - by - bh) > fabs(ay + ah - by)) {
        dy = ay + ah - by;
    } else {
        dy = ay - by + bh;
    }
    float r = a->type->radius + b->type->radius;
    float dx = b->position[0] - a->position[0];
    float dz = b->position[2] - a->position[2];
    float r2 = r * r;
    float d2 = dx * dx + dz * dz;
    if (r2 > d2) {
        float b_inv_mass = b->type->inv_mass;
        float a_inv_mass = a->type->inv_mass;
        float d = sqrtf(d2);
        float afactor = a_inv_mass / (a_inv_mass + b_inv_mass);
        float bfactor = b_inv_mass / (a_inv_mass + b_inv_mass);
        vec3 vel;
        vec3_sub(vel, b->velocity, a->velocity);
        float restitution = a->type->restitution * b->type->restitution;
        if (fabs(r - d) > fabs(dy)) { // Vertical (Y) correction
            a->_position_penalty[1] -= afactor * dy;
            b->_position_penalty[1] += bfactor * dy;
            float speed = vel[1];
            dy = dy > 0 ? dy : D_EPSILON;
            float accelfactor = speed * restitution / dy;
            a->_acceleration[1] -= accelfactor * afactor;
            b->_acceleration[1] += accelfactor * bfactor;
        } else { // horizontal (XZ) correction
            d = d > 0 ? d : D_EPSILON;
            float factor = (r - d) / d;
            a->_position_penalty[0] -= factor * afactor * dx;
            a->_position_penalty[2] -= factor * afactor * dz;
            b->_position_penalty[0] += factor * bfactor * dx;
            b->_position_penalty[2] += factor * bfactor * dz;
            float speed = sqrtf(vel[0] * vel[0] + vel[2] * vel[2]);
            float accelfactor = restitution * speed / d;
            a->_acceleration[0] -= accelfactor * afactor * dx;
            a->_acceleration[2] -= accelfactor * afactor * dz;
            b->_acceleration[0] += accelfactor * bfactor * dx;
            b->_acceleration[2] += accelfactor * bfactor * dz;
        }
        return 1;
    }
    return 0;
}

void scene_update() {

    static const vec3 zero = {0, 0, 0};

    for (unsigned update_count = 0; update_count < updates_per_frame; update_count++) {

        // Iterate mobs and integrate for new position.
        for (unsigned i = 0; i < scene_mob_count; i++) {
            Mob * m = scene_mobs[i];
            vec3 oldpos;
            vec3_assign(oldpos, m->position);
            vec3_add(m->position, oldpos, m->velocity);
            vec3_addmul(m->position, m->position, m->_acceleration, 0.5f);
            vec3_add(m->velocity, m->velocity, m->_acceleration);
            vec3_assign(m->_acceleration, zero);
            vec3_assign(m->_position_penalty, zero);
            // Apply latent friction
            if (m->friction > 0)
                mob_apply_friction(m, m->friction);
        }

        // Better broadphase later. This was giving me so many headaches. Ill just use N^2 approach.
        for (unsigned i = 0; i < scene_mob_count; i++) {
            Mob * a = scene_mobs[i];
            if (a->flags & MOB_NOCOLLIDE) continue;
            for (unsigned j = i + 1; j < scene_mob_count; j++) {
                Mob * b = scene_mobs[j];
                if (b->flags & MOB_NOCOLLIDE) continue;
                scene_resolve_mob_collision(a, b);
            }
        }

        // Apply penalty collision displacement to make collisions look better and not explode.
        // (I.E. Restitution force and position penalty not applied multiple times)
        for (unsigned i = 0; i < scene_mob_count; i++) {
            Mob * m = scene_mobs[i];
            vec3 oldpos;
            vec3_assign(oldpos, m->position);
            vec3_add(m->position, m->position, m->_position_penalty);
        }

    }
}
