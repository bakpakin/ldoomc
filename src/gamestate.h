#ifndef GAMESTATE_HEADER
#define GAMESTATE_HEADER

#include "glfw.h"

/*
 * Game state globals
 */
extern double game_delta;
extern double game_fps;

/*
 * Global callbacks
 */
void game_init();

void game_exit();

void game_exit_now();

/*
 * Gamestate system code
 */
typedef struct {
    void (*init)();
    void (*deinit)();
    void (*hide)();
    void (*show)();
    void (*update)(double dt);
    void (*keydown)(int key);
    void (*key)(int key);
    void (*keyup)(int key);
    void (*mousemoved)(float x, float y, float dx, float dy);
    void (*mouseup)(int button, float x, float y);
    void (*mousedown)(int button, float x, float y);
    void (*mousewheel)(float dx, float dy);
    void (*draw)();
    void (*resize)(int width, int height);
    void (*updateTick)();
    void * state;
} Gamestate;

/*
 * Starts the main loop.
 */
void game_mainloop(Gamestate * initial_state);

/*
 * Initializes a gamestate to Null values.
 */
Gamestate * gamestate_init(Gamestate * gs);

/*
 * Switch to a new gamestate without changing the stack.
 */
void gamestate_switch(Gamestate * gs);

/*
 * Change to a new gamestate by popping a gamestate on the stack.
 */
void gamestate_push(Gamestate * gs);

/*
 * Pop multiple states of the stack at once. index is the absolute position in the stack.
 */
void gamestate_jump(unsigned index);
/*
 * Same as gamestate_jump, but index is relative to the current index.
 */
void gamestate_jump_back(unsigned index);

/*
 * Pop the curretn gamestate off the stack and switch to the one below.
 */
void gamestate_pop();

#endif
