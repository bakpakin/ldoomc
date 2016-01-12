#include "arenastate.h"
#include "ldoom.h"

static mat4 hudmatrix;
static FontDef fd;
static Text txt;

static void init() {
    mat4_proj_ortho(hudmatrix, 0, 1000, 600, 0, 0, 10);
    fnt_init(&fd, "../resources/consolefont.txt");
    text_init(&txt, &fd, "Hello, World!", 144, ALIGN_LEFT, ALIGN_TOP, 500);
    txt.threshold = 0.5f;
    txt.smoothing = 1.0f / 32.0f;
    txt.color[0] = txt.color[1] = txt.color[2] = 0.0f;
}

static void deinit() {
    fnt_deinit(&fd);
    text_deinit(&txt);
}

static void show() {

}

static void hide() {

}

static void mousemoved(float x, float y, float dx, float dy) {

}

static void key(int key) {

}

static void keyup(int key) {

}

static void keydown(int keydown) {

}

static void mouseup(int button, float x, float y) {

}

static void mousedown(int button, float x, float y) {

}

static void mousewheel(float dx, float dy) {

}

static void resize(int width, int height) {

}

static void update(double dt) {

}

static void draw() {
    text_draw(&txt, hudmatrix);
}

Gamestate arenastate = {
    init,
    deinit,
    hide,
    show,
    update,
    keydown,
    key,
    keyup,
    mousemoved,
    mouseup,
    mousedown,
    mousewheel,
    draw,
    resize,
    NULL,
};
