#include "scene.h"
#include "vector.h"
#include "shader.h"
#include "platform.h"
#include "camera.h"

VECGEN(Mob *, mob);

static Program diffuseshader;
static int rendererCount = 0;
static GLint diffuse_mvp_loc;
static GLint diffuse_diffuse_loc;

Scene * scene_init(Scene * s) {

    if (!rendererCount++) {
        program_init_resource(&diffuseshader, "diffuseshader.glsl");
        diffuse_mvp_loc = glGetUniformLocation(diffuseshader.id, "u_mvp");
        diffuse_diffuse_loc = glGetUniformLocation(diffuseshader.id, "u_diffuse");
    }

    PlatformWindow window;
    platform_get_window(&window);
    camera_init_perspective(&s->camera, 1.5f, window.width / (float) window.height, 0.05f, 100.0f);

    // Initialize mob allocator
    flexpool_init(&s->mobpool, sizeof(Mob), 100);

    // Initialize space for mobs
    vector_init_mob(&s->mobs, 10);

    // Initialize grids
    grid_init(&s->solidGrid, 4.0f, 10, 1, 10);

    // Generate buffers
    GLuint bufs[4];
    glGenFramebuffers(4, bufs);
    s->diffuseBuffer = bufs[0];
    s->normalBuffer = bufs[1];
    s->lightBuffer = bufs[2];
    s->specularBuffer = bufs[3];
    s->timeBuffer = 0;

    return s;
}

void scene_deinit(Scene * s) {

    // Deinitialize space for mobs
    vector_deinit_mob(&s->mobs);

    // Deinitialize mob allocator
    flexpool_deinit(&s->mobpool);

    // Deinitialize grids
    grid_deinit(&s->solidGrid);

    // Deallocate buffers
    GLuint bufs[4];
    bufs[0] = s->specularBuffer;
    bufs[1] = s->lightBuffer;
    bufs[2] = s->diffuseBuffer;
    bufs[3] = s->normalBuffer;
    glDeleteFramebuffers(4, bufs);

    if (!--rendererCount) {
        program_deinit(&diffuseshader);
    }
}

Mob * scene_add_mob(Scene * s, MobDef * type, vec3 position) {

    static const vec3 zero = {0, 0, 0};
    Mob * m = flexpool_alloc(&s->mobpool);
    if (!m) return NULL;
    mob_init(m, type);
    vec3_assign(m->position, position);
    vec3_assign(m->prev_position, position);

    unsigned index = s->mobs.count;
    vector_push_mob(&s->mobs, m);
    m->renderid = index;

    return m;
}

void scene_remove_mob(Scene * s, Mob * m) {

    if (m->renderid <= 0) return;
    unsigned index = m->renderid;
    vector_bag_remove_mob(&s->mobs, index);
    ((Mob *)s->mobs.data + index)->renderid = index;
    m->renderid = -1;
    mob_deinit(m);

}

void scene_free_mob(Scene * s, Mob * m) {

    if (m->renderid > 0) {
        scene_remove_mob(s, m);
    }
    flexpool_free(&s->mobpool, m);

}

void scene_render(Scene * s) {

    // Update camera aspect ratio if width and height of window have changed.
    PlatformWindow window;
    platform_get_window(&window);
    camera_set_perspective(&s->camera, s->camera.data.perspective.fovY, (float) window.width / (float) window.height, 0.05f, 100.0f);

    // For now, just render everything as diffuse. No spatial partioning yet.
    glUseProgram(diffuseshader.id);
    for (int i = 0; i < s->mobs.count; i++) {
        Mob * mob = vector_get_mob(&s->mobs, i);
        if (MOB_INVISIBLE & mob->flags) continue;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mob->type->model.diffuse.id);
        glUniform1i(diffuse_diffuse_loc, 0);
        mat4 mvp;
        camera_calc_mvp(&s->camera, mvp, mob->position);
        glUniformMatrix4fv(diffuse_mvp_loc, 1, GL_FALSE, mvp);
        mesh_draw(&mob->type->model.mesh);
    }

}

#define PHYSICS_UPDATE (1.0/60.0)

void scene_update(Scene * s, double dt) {

    s->timeBuffer += dt;

    // Iterate mobs and update positions.
    while (s->timeBuffer > 0) {
        s->timeBuffer -= PHYSICS_UPDATE;
        for (int i = 0; i < s->mobs.count; i++) {

            Mob * m = vector_get_mob(&s->mobs, i);
            vec3 oldposition;
            vec3_assign(oldposition, m->position);
            for(int j = 0; j < 3; j++)
                m->position[j] += (oldposition[j] - m->prev_position[j]);
            vec3_assign(m->prev_position, oldposition);

        }
    }

}
