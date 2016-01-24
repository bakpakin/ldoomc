#include "model.h"
#include "mesh.h"

Model * model_init(Model * m,
        const Mesh * mesh,
        const Texture * normal,
        const Texture * diffuse,
        const Texture * specular,
        ModelShader * ms) {
    m->mesh = *mesh;
    m->modelShader = ms;

    m->diffuse = *diffuse;
    m->normal = *normal;
    m->specular = *specular;

    return m;
}

void model_deinit(Model * m) {
    mesh_deinit(&m->mesh);
}

void model_draw(Model * m, mat4 mvp, float time) {

    ModelShader * ms = m->modelShader;

    glUseProgram(ms->program.id);

    if (ms->timeLocation >= 0)
        glUniform1f(ms->timeLocation, time);

    if (ms->mvpLocation >= 0)
        glUniformMatrix4fv(ms->mvpLocation, 1, GL_FALSE, mvp);

    if (ms->diffuseLocation >= 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m->diffuse.id);
        glUniform1i(ms->diffuseLocation, 0);
    }

    if (ms->normalLocation >= 0) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m->normal.id);
        glUniform1i(ms->normalLocation, 1);
    }

    if (ms->specularLocation >= 0) {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, m->specular.id);
        glUniform1i(ms->specularLocation, 2);
    }

    mesh_draw(&m->mesh);
}
