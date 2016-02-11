#include "arenastate.h"
#include "ldoom.h"

static Camera cam;
static mat4 hudmatrix;
static FontDef fd;
static Text txt;
static Scene scene;

static MobDef mobdef;

static float yaw, pitch;
static vec3 cam_position;

static void init() {

    // Quickdraw init
    qd_init();

    // Font init
    fnt_init(&fd, "consolefont.txt");
    text_init(&txt, &fd, "fps: 60  ", 14, ALIGN_LEFT, ALIGN_TOP, 500, 1);
    txt.threshold = 0.3f;
    txt.smoothing = 1.0f / 4.0f;
    txt.position[0] = txt.position[1] = 5.0f;

    scene_init(&scene);

    // Create 100 mobs
    mobdef_init(&mobdef);
    mesh_init_quickcube(&mobdef.model.mesh);
    texture_init_resource(&mobdef.model.diffuse, "diffuse.png");
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            vec3 p = {2.5f * i - 12.5, 0, 2.5f * j - 12.5};
            scene_add_mob(&scene, &mobdef, p);
        }
    }
}

static void deinit() {
    qd_deinit();
    fnt_deinit(&fd);
    text_deinit(&txt);
    scene_deinit(&scene);
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
    mat4_proj_ortho(hudmatrix, 0, width, height, 0, 0, 10);
}

static void update(double dt) {
    yaw -= platform_poll_axis(PAXIS_X1) * platform_delta();
    pitch -= platform_poll_axis(PAXIS_Y1) * platform_delta();
    float strafe = platform_poll_axis(PAXIS_X2);
    float forward = platform_poll_axis(PAXIS_Y2);
    cam_position[0] += (strafe * sinf(yaw) + forward * cosf(yaw)) * platform_delta() * 4;
    cam_position[2] += (forward * sinf(yaw) - strafe * cosf(yaw)) * platform_delta() * 4;

    camera_set_position(&scene.camera, cam_position);
    vec3 direction;
    direction[0] = cosf(pitch) * cos(yaw);
    direction[1] = sinf(pitch);
    direction[2] = cosf(pitch) * sin(yaw);
    camera_set_direction(&scene.camera, direction);

    scene_update(&scene, dt);
}

static void updateTick() {
    text_format(&txt, 25, "fps: %.0f", platform_fps());
}

static void draw() {
    scene_render(&scene);
    text_draw(&txt, hudmatrix);

    qd_matrix(hudmatrix);
    qd_rgb(1, 0, 0);
    qd_circle(platform_width() / 2, platform_height() / 2, 10, 50, QD_LINELOOP);
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
