#include "arenastate.h"
#include "mob.h"
#include "scene.h"
#include "log.h"
#include "console.h"
#include "quickdraw.h"

static MobDef ARS_mobdef;
static MobDef ARS_playerdef;
static Mob ARS_player;
static Model ARS_cyl;
static Mesh ARS_mesh;

static float ARS_yaw = M_PI / 4;
static float ARS_pitch = 0;

static void ARS_init() {

    scene_init();

    // Create 100 mobs
    mobdef_init(&ARS_mobdef);
    ARS_mobdef.radius = 0.5f;
    mesh_init_cylinder(&ARS_mesh, 1.0f, 0.5f, 40);
    ARS_cyl.mesh = &ARS_mesh;
    ARS_mobdef.model = &ARS_cyl;
    ARS_mobdef.height = 1;
    ARS_mobdef.friction = 0.01;
    ARS_mobdef.restitution = 0.2f;
    /* mobdef.inv_mass = 0; */
    Mob * ms = malloc(1000 * sizeof(Mob));
    texture_init_resource(&ARS_cyl.diffuse, "diffuse.png");
    for (int x = 0; x < 10; x++)
        for (int y = 0; y < 1; y++)
            for (int z = 0; z < 10; z++) {
                vec3 p = {2.5f * x, 2.5 * y, 2.5f * z};
                Mob * m = ms + (x + 10 * y + 100 * z);
                mob_init(m, &ARS_mobdef, p);
                scene_add_mob(m);
            }

    // Create player
    mobdef_init(&ARS_playerdef);
    ARS_playerdef.model = &ARS_cyl;
    ARS_playerdef.inv_mass = 0.5f;
    ARS_playerdef.radius = 0.5f;
    ARS_playerdef.height = 1;
    ARS_playerdef.friction = 0.02f;
    const vec3 start = {-2, 0, -2};
    mob_init(&ARS_player, &ARS_playerdef, start);
    scene_add_mob(&ARS_player);

    ldlog_stdout_set(0);

}

static void ARS_deinit() {
    mesh_deinit(&ARS_mesh);
    texture_deinit(&ARS_cyl.diffuse);
    scene_deinit();
}

static void ARS_show() {

}

static void ARS_hide() {

}

static void ARS_button(PlatformButton b, PlatformButtonAction a) {
    if (a == PBA_DOWN && b == PBUTTON_SYS) {
        platform_set_pointer_mode(
                platform_get_pointer_mode() == PPOINTERMODE_LOCKED ?
                PPOINTERMODE_FREE : PPOINTERMODE_LOCKED);
    }
}

static void ARS_update(double dt) {
    if (platform_get_pointer_mode() == PPOINTERMODE_LOCKED) {
        ARS_yaw -= platform_poll_axis(PAXIS_X1) * platform_delta() * 8;
        ARS_pitch -= platform_poll_axis(PAXIS_Y1) * platform_delta() * 8;
    }
    ARS_pitch = ldm_min(ldm_max(-M_PI/2 + 0.1f, ARS_pitch), M_PI/2 - 0.1f);
    ARS_player.facing[0] = cosf(ARS_pitch) * cos(ARS_yaw);
    ARS_player.facing[1] = sinf(ARS_pitch);
    ARS_player.facing[2] = cosf(ARS_pitch) * sin(ARS_yaw);
    camera_set_direction(&scene_camera, ARS_player.facing);
    float strafe = platform_poll_axis(PAXIS_X2) * 0.06f;
    float forward = platform_poll_axis(PAXIS_Y2) * 0.06f;

    mob_impulse_move(&ARS_player, forward, strafe);

    scene_update(dt);

    mob_limit_hspeed(&ARS_player, 0.1f);

    vec3 cvec;
    vec3_assign(cvec, ARS_player.position);
    cvec[1] += 1.5f;
    camera_set_position(&scene_camera, cvec);
}

static void ARS_draw() {
    scene_render();
    qd_rgb(1, 0, 0);
    qd_circle(platform_width() / 2, platform_height() / 2, 30, 50, QD_LINELOOP);
}

static void ARS_updateTick() {

}

Gamestate arenastate = {
    ARS_init,
    ARS_deinit,
    ARS_hide,
    ARS_show,
    ARS_update,
    ARS_button,
    ARS_draw,
    NULL,
    ARS_updateTick
};
