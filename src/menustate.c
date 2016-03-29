#include "menustate.h"

static void init() {

}

static void deinit() {

}

static void update(double dt) {

}

static void draw() {

}

static void resize(int width, int height) {

}

static void show() {

}

static void hide() {

}

static void button(PlatformButton b, PlatformButtonAction action) {

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
