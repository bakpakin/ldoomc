#include "drenderer.h"
#include "opengl.h"
#include "shader.h"
#include "platform.h"

VECTOR_STATIC_GENERATE(Mob *, mob);

static Program diffuseshader;
static int rendererCount = 0;
static GLint diffuse_mvp_loc;
static GLint diffuse_diffuse_loc;

DRenderer * drenderer_init(DRenderer * dr) {

    if (!rendererCount++) {
        program_init_resource(&diffuseshader, "diffuseshader.glsl");
        diffuse_mvp_loc = glGetUniformLocation(diffuseshader.id, "u_mvp");
        diffuse_diffuse_loc = glGetUniformLocation(diffuseshader.id, "u_diffuse");
    }

    PlatformWindow window;
    platform_get_window(&window);
    camera_init_perspective(&dr->camera, 1.0f, window.width / (float) window.height, 0.05f, 100.0f);

    // Initialize space for mobs
    vector_init_mob(&dr->mobs, 10);

    // Generate buffers
    GLuint bufs[4];
    glGenFramebuffers(4, bufs);
    dr->diffuseBuffer = bufs[0];
    dr->normalBuffer = bufs[1];
    dr->lightBuffer = bufs[2];
    dr->specularBuffer = bufs[3];

    return dr;
}

void drenderer_deinit(DRenderer * dr) {

    // Deinitialize space for mobs
    vector_deinit_mob(&dr->mobs);

    // Deallocate buffers
    glDeleteFramebuffers(4, &dr->diffuseBuffer);

    if (!--rendererCount) {
        program_deinit(&diffuseshader);
    }
}

void drenderer_add(DRenderer * dr, Mob * m) {

    unsigned index = dr->mobs.count;
    vector_push_mob(&dr->mobs, m);
    m->renderid = index;

}

void drenderer_remove(DRenderer * dr, Mob * m) {

    unsigned index = m->renderid;
    vector_bag_remove_mob(&dr->mobs, index);
    ((Mob *)dr->mobs.data + index)->renderid = index;

}

void drenderer_resize(DRenderer * dr, int width, int height) {

    camera_init_perspective(&dr->camera, 1.0f, width / (float) height, 0.05f, 100.0f);

}

void drenderer_render(DRenderer * dr) {

    // For now, just render everything. No spatial partioning yet.
    glUseProgram(diffuseshader.id);
    for (int i = 0; i < dr->mobs.count; i++) {
        Mob * mob = vector_get_mob(&dr->mobs, i);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mob->type->model.diffuse.id);
        glUniform1i(diffuse_diffuse_loc, 0);
        mat4 mvp;
        camera_calc_mvp(&dr->camera, mvp, mob->position);
        glUniformMatrix4fv(diffuse_mvp_loc, 1, GL_FALSE, mvp);
        mesh_draw(&mob->type->model.mesh);
    }

}
