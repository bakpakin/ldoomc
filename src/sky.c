#include "sky.h"
#include "shader.h"
#include "glfw.h"
#include "ldmath.h"
#include "mesh.h"
#include "texture.h"
#include "camera.h"
#include "scene.h"
#include "string.h"

static Program skyshader;
Texture sky_texture;
static GLuint cubemap;
static GLint skyboxTexLocation;
static GLint vpLocation;
static Mesh skymesh;

static void generateCubemap() {
    glGenTextures(1, &cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
	const char * skybox[6] = {
		"skyright.png",
		"skyleft.png",
		"skytop.png",
		"skybottom.png",
		"skyback.png",
		"skyfront.png"
	};
	texture_cube_init_resource(&sky_texture, skybox);
}

void sky_init() {
    program_init_resource(&skyshader, "sky.glsl");
    skyboxTexLocation = glGetUniformLocation(skyshader.id, "u_skybox");
    vpLocation = glGetUniformLocation(skyshader.id, "u_vp");
    mesh_init_quickcube(&skymesh, 1, 1);
	generateCubemap();
}

void sky_deinit() {
    program_deinit(&skyshader);
	texture_deinit(&sky_texture);
	mesh_deinit(&skymesh);
}

void sky_render() {
    Camera c;
    memcpy(&c, &scene_camera, sizeof(Camera));
    static const vec3 zero = {0, 0, 0};
    camera_set_position(&c, zero);
    glUseProgram(skyshader.id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, sky_texture.id);
    glUniform1i(skyboxTexLocation, 0);
    glUniformMatrix4fv(vpLocation, 1, GL_FALSE, camera_matrix(&c));
    mesh_draw(&skymesh);
}
