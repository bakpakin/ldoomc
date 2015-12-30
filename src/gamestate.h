#ifndef GAMESTATE_HEADER
#define GAMESTATE_HEADER

#include "config.h"
#include OPENGL_H
#include SDL_H

extern unsigned game_fps;
extern SDL_GLContext * game_gl;
extern SDL_Window * game_window;
extern float game_delta;

void game_init();

void game_exit();

void game_exit_now();

typedef struct {
    void (*init)();
    void (*deinit)();
    void (*update)(double dt);
    void (*keydown)(SDL_KeyboardEvent e, SDL_Keycode key);
    void (*key)(SDL_Scancode skey);
    void (*keyup)(SDL_KeyboardEvent e, SDL_Keycode key);
    void (*mousemoved)(SDL_MouseMotionEvent e, int x, int y, int dx, int dy);
    void (*mouseup)(SDL_MouseButtonEvent e, int button, int x, int y);
    void (*mousedown)(SDL_MouseButtonEvent e, int button, int x, int y);
    void (*mousewheel)(SDL_MouseWheelEvent e, int dy);
    void (*draw)();
    void * state;
} Gamestate;

void game_mainloop(Gamestate * initial_state);

Gamestate * gamestate_init(Gamestate * gs);

void gamestate_switch(Gamestate * gs);

void gamestate_push(Gamestate * gs);

void gamestate_jump(unsigned index);

void gamestate_jump_back(unsigned index);

void gamestate_pop();

#endif
