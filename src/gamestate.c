#include "gamestate.h"

#include "util.h"
#include <stdint.h>
#include <stdlib.h>

#define MAX_STATE_STACK 32

static Gamestate * stack[MAX_STATE_STACK];
static Gamestate current_state;
static unsigned current_index = 0;

/*
 * Initialize a gamestate structure.
 */
Gamestate * gamestate_init(Gamestate * gs) {
    memset(gs, 0, sizeof(Gamestate));
    return gs;
}

/*
 * Switch to a new gamestate and remove the current state from the stack.
 */
void gamestate_switch(Gamestate * gs) {
    if (current_state.deinit) current_state.deinit();
    stack[current_index] = gs;
    current_state = *gs;
    if (current_state.init) current_state.init();
}

/*
 * Jump to a gamestate index less than the current index and jump to that state.
 */
void gamestate_jump(unsigned index) {
    if (index > current_index)
        uerr("Invalid gamestate jump index.");
    if (current_state.deinit) current_state.deinit();
    current_index = index;
    current_state = *stack[index];
    if (current_state.init) current_state.init();
}

/*
 * Pop index number of states of the state stack.
 */
void gamestate_jump_back(unsigned index) {
    if (index > current_index)
        uerr("Invalid gamestate jump index.");
    if (current_state.deinit) current_state.deinit();
    current_index -= index;
    current_state = *stack[index];
    if (current_state.init) current_state.init();
}

/*
 * Push a new state onto the state stack.
 */
void gamestate_push(Gamestate * gs) {
    if (current_index == MAX_STATE_STACK) {
        uerr("Gamestate stack overflow.");
    }
    if (current_state.deinit) current_state.deinit();
    stack[current_index++] = gs;
    current_state = *gs;
    if (current_state.init) current_state.init();
}

/*
 * Pop a gamestate off of the state stack.
 */
void gamestate_pop() {
    if (current_index == 0) {
        uerr("Gamestate stack underflow.");
    }
    if (current_state.deinit) current_state.deinit();
    current_state = *stack[--current_index];
    if (current_state.init) current_state.init();
}

static void gamestate_update(double dt) {
    if (current_state.update != NULL) {
        current_state.update(dt);
    }
}

static void gamestate_draw() {
    if (current_state.draw != NULL) {
        current_state.draw();
    }
}

static inline void handle_window_event(SDL_WindowEvent e) {
    switch (e.event) {
    case SDL_WINDOWEVENT_RESIZED:
        break;
    case SDL_WINDOWEVENT_CLOSE:
        game_exit();
        break;
    default:
        break;
    }
}

void gamestate_poll_input() {
    SDL_Event e;
    Gamestate gs = current_state;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_KEYDOWN:
                if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                    game_exit();
                if (gs.keydown && !e.key.repeat)
                    gs.keydown(e.key, e.key.keysym.sym);
                break;
            case SDL_KEYUP:
                if (gs.keyup && !e.key.repeat)
                    gs.keyup(e.key, e.key.keysym.sym);
                break;
            case SDL_MOUSEMOTION:
                if (gs.mousemoved)
                    gs.mousemoved(e.motion, e.motion.x, e.motion.y, e.motion.xrel, e.motion.yrel);
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (gs.mousedown)
                    gs.mousedown(e.button, e.button.button, e.button.x, e.button.y);
                break;
            case SDL_MOUSEBUTTONUP:
                if (gs.mouseup)
                    gs.mouseup(e.button, e.button.button, e.button.x, e.button.y);
                break;
            case SDL_MOUSEWHEEL:
                if (gs.mousewheel)
                    gs.mousewheel(e.wheel, e.wheel.y);
                break;
            case SDL_WINDOWEVENT:
                handle_window_event(e.window);
                break;
            default:
                break;
        }
    }
}

unsigned game_fps = 60;
SDL_Window * game_window = NULL;
SDL_GLContext * game_gl = NULL;
float game_delta;

void game_init() {
    SDL_Init(SDL_INIT_VIDEO);

	// Set SDL and GL settings
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetSwapInterval(1);

    SDL_DisplayMode dm;
    SDL_GetDesktopDisplayMode(0, &dm);

    unsigned flags = SDL_WINDOW_OPENGL |
        SDL_WINDOW_ALLOW_HIGHDPI |
        SDL_WINDOW_FULLSCREEN;

	game_window = SDL_CreateWindow("Ldoom",
				   SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
				   dm.w,
				   dm.h,
				   flags);
	game_gl = SDL_GL_CreateContext(game_window);

    SDL_SetRelativeMouseMode(SDL_TRUE);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

static int game_done = 0;

void game_exit() {
    game_done = 1;
}

static void game_quit() {
    if (current_state.deinit) current_state.deinit();
    SDL_Quit();
}

void game_exit_now() {
    game_quit();
}

void game_mainloop(Gamestate * initial_state) {
    game_done = 0;
    current_index = 0;
    memset(&current_state, 0, sizeof(Gamestate));
    gamestate_switch(initial_state);
    uint32_t frametime, last_frametime = 0;
    const unsigned char * keystates;
    int keystates_length;
    while (!game_done) {
        last_frametime = frametime;
        frametime = SDL_GetTicks();
        gamestate_poll_input();
        keystates = SDL_GetKeyboardState(&keystates_length);
        if (current_state.key)
            for (int i = 0; i < keystates_length; i++) {
                if (keystates[i])
                    current_state.key(i);
            }
        if (last_frametime > frametime) { // we have wrapped
            game_delta = (4294967295 - last_frametime + frametime) / 1000.0;
        } else {
            game_delta = (frametime - last_frametime) / 1000.0;
        }
        gamestate_update(game_delta);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        gamestate_draw();
        SDL_GL_SwapWindow(game_window);
    }
    game_quit();
}
