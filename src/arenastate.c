#include "arenastate.h"
#include "ldoom.h"

static Camera cam;
static mat4 hudmatrix;
static FontDef fd;
static Text txt;

static void init() {
    fnt_init(&fd, "consolefont.txt");
    text_init(&txt, &fd, "fps: 0.00   ", 14, ALIGN_LEFT, ALIGN_TOP, 500, 1);
    txt.threshold = 0.3f;
    txt.smoothing = 1.0f / 4.0f;
    txt.position[0] = txt.position[1] = 5.0f;
}

static void deinit() {
    fnt_deinit(&fd);
    text_deinit(&txt);
}

static void show() {

}

static void hide() {

}

static void button(PlatformButton b, PlatformButtonAction a) {

}

static void resize(int width, int height) {
    glViewport(0, 0, width, height);
    mat4_proj_ortho(hudmatrix, 0, width, height, 0, 0, 10);
}

static void update(double dt) {

}

static void updateTick() {
   text_format(&txt, 25, "fps: %.2f", platform_fps);
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
    button,
    draw,
    resize,
    updateTick
};
