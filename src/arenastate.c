#include "arenastate.h"
#include "ldoom.h"

static Camera cam;
static mat4 hudmatrix;
static FontDef fd;
static Text txt;
static grid3d * grid;

static DRenderer renderer;
static MobDef mobdef;
static Mob mobs[100];

static float yaw, pitch;
static vec3 cam_position;

static void init() {

    // Font init
    fnt_init(&fd, "consolefont.txt");
    text_init(&txt, &fd, "fps: 60.00  ", 14, ALIGN_LEFT, ALIGN_TOP, 500, 1);
    txt.threshold = 0.3f;
    txt.smoothing = 1.0f / 4.0f;
    txt.position[0] = txt.position[1] = 5.0f;

    // Grid init
    grid = grid3d_new(12, 12, 1);

    // Render init
    drenderer_init(&renderer);

    // Create 100 mobs
    mesh_init_quickcube(&mobdef.model.mesh);
    texture_init_resource(&mobdef.model.diffuse, "diffuse.png");

    for (int i = 0; i < 10; i++) {
        mobs[i].type = &mobdef;
        mobs[i].position[0] = 1.5f * i;
        mobs[i].position[1] = 0;
        mobs[i].position[2] = 0;
        drenderer_add(&renderer, mobs + i);
    }

}

static void deinit() {
    fnt_deinit(&fd);
    text_deinit(&txt);
    grid3d_delete(grid);
    drenderer_deinit(&renderer);
    mesh_deinit(&mobdef.model.mesh);
    texture_deinit(&mobdef.model.diffuse);
}

static void show() {

}

static void hide() {

}

static void button(PlatformButton b, PlatformButtonAction a) {
    if (a == PBA_DOWN && b == PBUTTON_SYS) {
        platform_toggle_pointer_mode();
    }
}

static void resize(int width, int height) {
    glViewport(0, 0, width, height);
    mat4_proj_ortho(hudmatrix, 0, width, height, 0, 0, 10);
    drenderer_resize(&renderer, width, height);
}

static void update(double dt) {
    yaw -= platform_poll_axis(PAXIS_X1) * platform_delta;
    pitch -= platform_poll_axis(PAXIS_Y1) * platform_delta;

    float strafe = platform_poll_axis(PAXIS_X2);
    float forward = platform_poll_axis(PAXIS_Y2);
    cam_position[0] += (strafe * sinf(yaw) + forward * cosf(yaw)) * platform_delta * 4;
    cam_position[2] += (forward * sinf(yaw) - strafe * cosf(yaw)) * platform_delta * 4;

    camera_set_position(&renderer.camera, cam_position);
    vec3 direction;
    direction[0] = cosf(pitch) * cos(yaw);
    direction[1] = sinf(pitch);
    direction[2] = cosf(pitch) * sin(yaw);
    camera_set_direction(&renderer.camera, direction);
}

static void updateTick() {
    text_format(&txt, 25, "fps: %.2f", platform_fps);
}

static void draw() {
    drenderer_render(&renderer);
    text_draw(&txt, hudmatrix);
}

Gamestate arenastate = {
    init,
    deinit,
    hide,
    show,
    update,
    button,
    draw,
    resize,
    updateTick
};
