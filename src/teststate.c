#include "ldoom.h"
#include "teststate.h"
#include "vector.h"

static mat4 hudmatrix;
static Grid grid;
static int ids[10000];
static aabb3 bounds = {
    {0, 0, 0},
    {2000, 2000, 2000}
};

static void init() {
    grid_init(&grid, 10, 200, 1, 200);
    platform_set_pointer_mode(PPOINTERMODE_PIXEL);
    ldlog_stdout_set(0);
    for (int i = 0; i < 10000; i++) {
        float x = ldm_randf() * 2000;
        float y = ldm_randf() * 2000;
        float z = ldm_randf() * 2000;
        aabb3 aabb = {
            {x - 6, y - 6, z - 6},
            {x + 6, y + 6, z + 6}
        };
        ids[i] = grid_add(&grid, aabb);
    }
}

static void deinit() {
    grid_deinit(&grid);
}

static void update(double dt) {
    float x = platform_poll_axis(PAXIS_X1);
    float y = platform_poll_axis(PAXIS_Y1);
    bounds[0][0] = x - 100;
    bounds[0][1] = y - 100;
    bounds[1][0] = x + 100;
    bounds[1][1] = y + 100;
}

static void updateTick() {
    ldlog("$@F00Colored Text Test: $@FFFHi?\n");
}

static void drawbox(const aabb3 box) {
    qd_rect(box[0][0], box[0][1], box[1][0] - box[0][0], box[1][1] - box[0][1], QD_LINELOOP);
}

static void draw() {
    drawbox(bounds);
    int handle = -1;
    grid_iter(&grid, bounds);
    while (grid_iter_next(&grid, &handle)) {
        aabb3 bb;
        grid_aabb(&grid, handle, bb);
        drawbox(bb);
    }
}

Gamestate teststate = {
    init, // init
    deinit, // deinit
    NULL, // hide
    NULL, // show
    update, // update
    NULL, // button
    draw, // draw
    NULL, //resize
    updateTick // updateTick
};
