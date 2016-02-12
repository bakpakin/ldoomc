#include "ldoom.h"
#include "teststate.h"
#include "vector.h"

static mat4 hudmatrix;
static Grid grid;
static int ids[1000];
static aabb3 bounds = {
    {0, 0, 0},
    {500, 500, 500}
};

static void init() {
    grid_init(&grid, 10, 20, 1, 20);
    qd_init();
    for (int i = 0; i < 1000; i++) {
        float x = ldm_randf() * 500;
        float y = ldm_randf() * 500;
        float z = ldm_randf() * 500;
        aabb3 aabb = {
            {x - 6, y - 6, z - 6},
            {x + 6, y + 6, z + 6}
        };
        ids[i] = grid_add(&grid, aabb);
    }
    mat4_proj_ortho(hudmatrix, 0, platform_width(), platform_height(), 0, 0, 10);
}

static void deinit() {
    grid_deinit(&grid);
    qd_deinit();
}

static void resize(int width, int height) {
    mat4_proj_ortho(hudmatrix, 0, width, height, 0, 0, 10);
}

static void update(double dt) {

}

static void updateTick() {

}

static void drawbox(const aabb3 box) {
    qd_rect(box[0][0], box[0][1], box[1][0] - box[0][0], box[1][1] - box[0][1], QD_LINELOOP);
}

static void draw() {
    qd_matrix(hudmatrix);
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
    resize, //resize
    updateTick // updateTick
};
