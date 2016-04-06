#include "menustate.h"
#include "fntdraw.h"

static FontDef hudft;
static Text title;

static void init() {
    fnt_init(&hudft, "hud.txt");
    text_init(&title, &hudft, "Ldoomc", 64, ALIGN_CENTER, ALIGN_CENTER, 600, 0);
    title.position[0] = platform_width()/2 - 300;
    title.position[1] = 100;
}

static void deinit() {

}

static void update(double dt) {

}

static void draw() {
    text_draw_screen(&title);
}

static void resize(int width, int height) {
    title.position[0] = width / 2 - 300;
}

static void show() {
    platform_set_pointer_mode(PPOINTERMODE_FREE);
}

static void hide() {

}

static void button(PlatformButton b, PlatformButtonAction action) {
    if (b == PBUTTON_SYS)
        platform_exit();
}

static void updateTick() {

}

Gamestate menustate = {
    init,
    deinit,
    hide,
    show,
    update,
    button,
    draw,
    resize,
    updateTick,
};
