#include "ldoom.h"

static Program p;
static Mesh m;
static Texture t;
static Camera c;

static Gamestate simple;

static float yaw = 0;
static float pitch = 0;

static vec3 position;
static vec3 direction;

static int mvpLoc;
static int diffuseLoc;

void init() {
    mesh_init_cylinder(&m, 1, 0.5, 40);
    camera_init_perspective(&c, 1, 10.0/6, 0.1, 100);
    position[0] = 0;
    position[1] = 0;
    position[2] = -5;
    yaw = 0;
    program_init_file(&p, "../resources/basic.glsl");
    texture_init_file(&t, "../resources/crate.png");
    mvpLoc = program_find_uniform(&p, "mvp");
    diffuseLoc = program_find_uniform(&p, "diffuse");
}

void draw() {
    glUseProgram(p.id);
    camera_apply(&c, mvpLoc);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, t.id);
    glUniform1i(diffuseLoc, 0);
    mesh_draw(&m);
}

void update(double dt) {
    direction[0] = sin(yaw) * cos(pitch);
    direction[1] = sin(pitch);
    direction[2] = cos(yaw) * cos(pitch);
    camera_set_direction(&c, direction);
    camera_set_position(&c, position);
}

void mousemoved(SDL_MouseMotionEvent e, int x, int y, int dx, int dy) {
    float scale = 0.4;
    yaw += dx * game_delta * scale;
    pitch -= dy * game_delta * scale;
    pitch = ldm_clamp(pitch, LD_PI / -2.1, LD_PI / 2.1);
}

void key(SDL_Scancode key) {
    float dp = game_delta * 4;
    switch(key) {
    case SDL_SCANCODE_LEFT:
        yaw -= 0.3 * dp;
        break;
    case SDL_SCANCODE_RIGHT:
        yaw += 0.3 * dp;
        break;
    case SDL_SCANCODE_E:
        position[1] -= dp;
        break;
    case SDL_SCANCODE_Q:
        position[1] += dp;
        break;
    case SDL_SCANCODE_S:
        position[0] -= sin(yaw) * dp;
        position[2] -= cos(yaw) * dp;
        break;
    case SDL_SCANCODE_W:
        position[0] += sin(yaw) * dp;
        position[2] += cos(yaw) * dp;
        break;
    case SDL_SCANCODE_A:
        position[0] -= cos(yaw) * dp;
        position[2] += sin(yaw) * dp;
        break;
    case SDL_SCANCODE_D:
        position[0] += cos(yaw) * dp;
        position[2] -= sin(yaw) * dp;
        break;
    default:
        break;
    }
}

void deinit() {
    mesh_deinit(&m);
    program_deinit(&p);
    texture_deinit(&t);
}

int main(int argc, char* argv[]) {
	game_init();
	gamestate_init(&simple);
	simple.init = init;
	simple.draw = draw;
	simple.update = update;
	simple.key = key;
	simple.mousemoved = mousemoved;
	simple.deinit = deinit;
	game_mainloop(&simple);
    return 0;
}
