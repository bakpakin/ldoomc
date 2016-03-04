#include "arenastate.h"
#include "ldoom.h"

static MobDef mobdef;
static MobDef playerdef;
static Mob player;
static Model cyl;
static Mesh mesh;
static Mesh grassmesh;
static Model grassmodel;

static float yaw = M_PI / 4;
static float pitch = 0;

static void init() {

    scene_init();

    // Create 100 mobs
    mobdef_init(&mobdef);
    mobdef.radius = 0.5f;
    mesh_init_cylinder(&mesh, 1.0f, 0.5f, 40);
    cyl.mesh = &mesh;
    mobdef.model = &cyl;
    mobdef.height = 1;
    mobdef.friction = 0;
    //mobdef.restitution = 0;
    /* mobdef.inv_mass = 0; */
    Mob * ms = malloc(1000 * sizeof(Mob));
    texture_init_resource(&cyl.diffuse, "diffuse.png");
    for (int x = 0; x < 10; x++)
        for (int y = 0; y < 1; y++)
            for (int z = 0; z < 10; z++) {
                vec3 p = {2.5f * x, 2.5 * y, 2.5f * z};
                Mob * m = ms + (x + 10 * y + 100 * z);
                mob_init(m, &mobdef, p);
                scene_add_mob(m);
            }

    // Create player
    mobdef_init(&playerdef);
    playerdef.model = &cyl;
    playerdef.inv_mass = 0.5f;
    playerdef.radius = 0.5f;
    playerdef.height = 1;
    playerdef.friction = 0.02f;
    static const vec3 start = {-2, 0, -2};
    mob_init(&player, &playerdef, start);
    scene_add_mob(&player);

    ldlog_stdout_set(0);

    //Load ground
    mesh_init_quickcube(&grassmesh);
    grassmodel.mesh = &grassmesh;
    //texture_init_resource(&grassmodel.diffuse, "grass.png");
}

static void deinit() {
    mesh_deinit(&mesh);
    texture_deinit(&cyl.diffuse);
    scene_deinit();
}

static void show() {

}

static void hide() {

}

static void button(PlatformButton b, PlatformButtonAction a) {
    if (a == PBA_DOWN && b == PBUTTON_SYS) {
        platform_set_pointer_mode(
                platform_get_pointer_mode() == PPOINTERMODE_LOCKED ?
                PPOINTERMODE_FREE : PPOINTERMODE_LOCKED);
    }
}

static void update(double dt) {
    if (platform_get_pointer_mode() == PPOINTERMODE_LOCKED) {
        yaw -= platform_poll_axis(PAXIS_X1) * platform_delta() * 8;
        pitch -= platform_poll_axis(PAXIS_Y1) * platform_delta() * 8;
    }
    pitch = ldm_min(ldm_max(-M_PI/2 + 0.1f, pitch), M_PI/2 - 0.1f);
    player.facing[0] = cosf(pitch) * cos(yaw);
    player.facing[1] = sinf(pitch);
    player.facing[2] = cosf(pitch) * sin(yaw);
    camera_set_direction(&scene_camera, player.facing);
    float strafe = platform_poll_axis(PAXIS_X2) * 0.06f;
    float forward = platform_poll_axis(PAXIS_Y2) * 0.06f;

    mob_impulse_move(&player, forward, strafe);

    scene_update(dt);

    mob_limit_hspeed(&player, 0.1f);

    vec3 cvec;
    vec3_assign(cvec, player.position);
    cvec[1] += 1.5f;
    camera_set_position(&scene_camera, cvec);
}

static void draw() {
    scene_render();
    qd_rgb(1, 0, 0);
    qd_circle(platform_width() / 2, platform_height() / 2, 30, 50, QD_LINELOOP);
}

static void updateTick() {
    ldlog("$@F00Colored Text: $@8FF Blah blah....");
}

Gamestate arenastate = {
    init,
    deinit,
    hide,
    show,
    update,
    button,
    draw,
    NULL,
    updateTick,
};
