#include "gamestate.h"

#include "vector.h"
#include "util.h"
#include <stdint.h>
#include <stdlib.h>

VECTOR_STATIC_GENERATE(int, int)

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
    if (current_state.show) current_state.show();
}

/*
 * Jump to a gamestate index less than the current index and jump to that state.
 */
void gamestate_jump(unsigned index) {
    if (index > current_index)
        uerr("Invalid gamestate jump index.");
    for (int i = current_index; i > index; i--) {
        if (stack[i]->deinit)
            stack[i]->deinit();
    }
    if (current_state.show) current_state.show();
}

/*
 * Pop index number of states of the state stack.
 */
void gamestate_jump_back(unsigned index) {
    if (index > current_index)
        uerr("Invalid gamestate jump index.");
    gamestate_jump(current_index - index);
}

/*
 * Push a new state onto the state stack.
 */
void gamestate_push(Gamestate * gs) {
    if (current_index == MAX_STATE_STACK) {
        uerr("Gamestate stack overflow.");
    }
    if (current_state.hide) current_state.hide();
    stack[current_index++] = gs;
    current_state = *gs;
    if (current_state.init) current_state.init();
    if (current_state.show) current_state.show();
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
    if (current_state.update) {
        current_state.update(dt);
    }
}

static void gamestate_draw() {
    if (current_state.draw) {
        current_state.draw();
    }
}

static Vector downKeys;

static int find_key(int key) {
    for (int i = 0; i < downKeys.count; i++) {
        if (vector_get_int(&downKeys, i) == key)
            return i;
    }
    return -1;
}

static void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE) {
        game_exit();
    } else {
        Gamestate gs = current_state;
        switch(action) {
            case GLFW_PRESS:
                if (gs.keydown) {
                    gs.keydown(key);
                }
                vector_push_int(&downKeys, key);
                break;
            case GLFW_RELEASE:
                if (gs.keyup) {
                    gs.keyup(key);
                }
                int index = find_key(key);
                if (index != -1) {
                    vector_bag_remove_int(&downKeys, index);
                }
                break;
            default:
                break;
        }
    }
}

static double cursor_old_xpos = 0;
static double cursor_old_ypos = 0;
static void cursor_position_callback(GLFWwindow * window, double xpos, double ypos) {
    if (xpos != cursor_old_xpos || ypos != cursor_old_ypos) {
        if (current_state.mousemoved){
            current_state.mousemoved(xpos, ypos, xpos - cursor_old_xpos, ypos - cursor_old_ypos);
        }
    }
    cursor_old_xpos = xpos;
    cursor_old_ypos = ypos;
}

static void mouse_button_callback(GLFWwindow * window, int button, int action, int mods) {
    Gamestate gs = current_state;
    switch(action) {
        case GLFW_PRESS:
            if (gs.mousedown)
                gs.mousedown(button, cursor_old_xpos, cursor_old_ypos);
            break;
        case GLFW_RELEASE:
            if (gs.mouseup)
                gs.mouseup(button, cursor_old_xpos, cursor_old_ypos);
            break;
        default:
            break;
    }
}

static void process_down_keys() {
    if (current_state.key) {
        for (int i = 0; i < downKeys.count; i++) {
            int key = vector_get_int(&downKeys, i);
            current_state.key(key);
        }
    }
}

static void error_callback(int error, const char * message) {
    uerr(message);
}

double game_delta;
static GLFWwindow * game_window;

void game_init() {

    glfwInit();

    glfwSetErrorCallback(&error_callback);

    // GLFW window and context creation
    GLFWmonitor * monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode * vmode = glfwGetVideoMode(monitor);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
	game_window = glfwCreateWindow(vmode->width, vmode->height, "Ldoom", NULL, NULL);
	if (!game_window) {
	    glfwTerminate();
    }
    glfwMakeContextCurrent(game_window);
    glfwSwapInterval(1);

    // GL initializing
    int width, height;
    glfwGetFramebufferSize(game_window, &width, &height);
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialize input
    vector_init_int(&downKeys, 128);
    glfwSetKeyCallback(game_window, &key_callback);
    glfwSetMouseButtonCallback(game_window, &mouse_button_callback);
    glfwSetCursorPosCallback(game_window, &cursor_position_callback);
    glfwSetInputMode(game_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void game_exit() {
    glfwSetWindowShouldClose(game_window, GL_TRUE);
}

static void game_quit() {
    if (current_state.deinit) current_state.deinit();
    vector_deinit_int(&downKeys);
    glfwDestroyWindow(game_window);
    glfwTerminate();
}

void game_exit_now() {
    game_quit();
}

void game_mainloop(Gamestate * initial_state) {
    current_index = 0;
    memset(&current_state, 0, sizeof(Gamestate));
    gamestate_switch(initial_state);
    double frametime = 0;
    double last_frametime = 0;
    while (!glfwWindowShouldClose(game_window)) {
        last_frametime = frametime;
        frametime = glfwGetTime();
        glfwSwapBuffers(game_window);
        glfwPollEvents();
        process_down_keys();
        game_delta = frametime - last_frametime;
        gamestate_update(game_delta);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        gamestate_draw();
    }
    game_quit();
}
