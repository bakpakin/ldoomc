#include "ldoom.h"

static mat4 ortho;
static FontDef fd;
static Text txt;

static Gamestate simple;

void init() {
    mat4_proj_ortho(ortho, 0, 1000, 600, 0, 0, 10);
    fnt_init(&fd, "../resources/font.txt");
    text_init(&txt, &fd, "Hello, World!\nHey there.", 144, ALIGN_LEFT, ALIGN_TOP, 500);
}

void draw() {
    text_draw(&txt, ortho);
}

void deinit() {
    fnt_deinit(&fd);
    text_deinit(&txt);
}

int main(int argc, char* argv[]) {
	game_init();
	gamestate_init(&simple);
	simple.init = init;
	simple.draw = draw;
	simple.deinit = deinit;
	game_mainloop(&simple);
    return 0;
}
