#include "platform.h"
#include "util.h"
#include "vector.h"
#include "ldmath.h"
#include "console.h"
#include "quickdraw.h"
#include "scene.h"
#include "log.h"

#include <string.h>

//////////////////////////////////////// COMMON START

static double _platform_delta = 0.0;
static double _platform_fps = 0.0;
static int _platform_width = 0;
static int _platform_height = 0;

double platform_delta() {
    return _platform_delta;
}

double platform_fps() {
    return _platform_fps;
}

void platform_get_window(PlatformWindow * w) {
    w->width = _platform_width;
    w->height = _platform_height;
}

int platform_width() {
    return _platform_width;
}

int platform_height() {
    return _platform_height;
}

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
    for (unsigned i = current_index; i > index; i--) {
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

static unsigned int platform_buttons;
static float platform_axes[4];

int platform_poll_button(PlatformButton button) {
    return platform_buttons & (1 << button);
}

float platform_poll_axis(PlatformAxis axis) {
    return platform_axes[axis];
}

static PlatformPointerMode pointer_mode = PPOINTERMODE_LOCKED;

PlatformPointerMode platform_get_pointer_mode() {
    return pointer_mode;
}

static mat4 screen_matrix = {1.0, 0.0, 0.0, 0.0,
                             0.0, 1.0, 0.0, 0.0,
                             0.0, 0.0, 1.0, 0.0,
                             0.0, 0.0, 0.0, 1.0};

const float * platform_screen_matrix() {
    return screen_matrix;
}

//////////////////////////////////////// COMMON END

//////////////////////////////////////// APPLE START
#ifdef __APPLE__

#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CFData.h>

int platform_res2file(const char * resource, char * pathbuf, unsigned bufsize) {
    CFURLRef url = CFBundleCopyResourceURL(
        CFBundleGetMainBundle(),
        CFStringCreateWithCString(
            NULL,
            resource,
            kCFStringEncodingASCII),
        NULL,
        NULL
    );

    if (!url)
        return 0;

    if(!CFURLGetFileSystemRepresentation(url, 1, (unsigned char *) pathbuf, bufsize))
        return 0;

    return 1;
}

#endif
//////////////////////////////////////// END APPLE

#if defined _WIN32 || defined __linux__

static char *platform_path_predicate = "resources/";
static size_t platform_buffer_predsize = 10; // strlen(platform_path_predicate);

// For now, just let it build. This should be replaced with platfrom specifics later.
int platform_res2file(const char * resource, char * pathbuf, unsigned bufsize) {
    size_t slen = strlen(resource);
    if (slen + platform_buffer_predsize > bufsize + 1) {
        return 0;
    }
    memcpy(pathbuf, platform_path_predicate, platform_buffer_predsize);
    strcpy(pathbuf + platform_buffer_predsize, resource);
    return 1;
}

#endif

//////////////////////////////////////// DESKTOP CODE START
#ifdef PLATFORM_DESKTOP

#include "glfw.h"
#include "opengl.h"

static GLFWwindow * game_window;

static unsigned long button_flags;

static int button_a_code = GLFW_KEY_Z;
static int button_b_code = GLFW_KEY_X;
static int button_c_code = GLFW_KEY_C;
static int button_d_code = GLFW_KEY_V;
static int button_e_code = GLFW_KEY_SPACE;
static int button_sys_code = GLFW_KEY_ESCAPE;

static PlatformButton get_pbutton(int key) {
    if (key == button_a_code) return PBUTTON_A;
    if (key == button_b_code) return PBUTTON_B;
    if (key == button_c_code) return PBUTTON_C;
    if (key == button_d_code) return PBUTTON_D;
    if (key == button_e_code) return PBUTTON_E;
    if (key == button_sys_code) return PBUTTON_SYS;
    return PBUTTON_OTHER;
}

static void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods) {
    PlatformButton button = get_pbutton(key);
    if (button == PBUTTON_OTHER) return;
    PlatformButtonAction pba = action == GLFW_PRESS ? PBA_DOWN : PBA_UP;
    if (action == GLFW_PRESS) {
        button_flags |= (1 << button);
    } else {
        button_flags &= ~(1 << button);
    }
    if (current_state.button) {
        current_state.button(button, pba);
    }
}

static double xmold = 0, ymold = 0;
static int cursor_tracking_started = 0;
static void cursor_position_callback(GLFWwindow * window, double xpos, double ypos) {
    if (pointer_mode == PPOINTERMODE_LOCKED) {
        if (!cursor_tracking_started) {
            cursor_tracking_started = 1;
            xmold = xpos; ymold = ypos;
        }
        platform_axes[0] = (xpos - xmold) * _platform_delta;
        platform_axes[1] = (ypos - ymold) * _platform_delta;
        xmold = xpos;
        ymold = ypos;
    } else if (pointer_mode == PPOINTERMODE_FREE) {
        platform_axes[0] = _platform_width * 2 / xpos - 1;
        platform_axes[1] = _platform_height * 2 / ypos - 1;
    } else { // PPOINTERMODE_PIXEL
        platform_axes[0] = xpos;
        platform_axes[1] = ypos;
    }
}

void platform_set_pointer_mode(PlatformPointerMode mode) {
    pointer_mode = mode;
    if (mode == PPOINTERMODE_LOCKED) {
        glfwGetCursorPos(game_window, &xmold, &ymold);
        glfwSetInputMode(game_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
        glfwSetInputMode(game_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

static void mouse_button_callback(GLFWwindow * window, int button, int action, int mods) {
    PlatformButton b = PBUTTON_OTHER;
    if (button == GLFW_MOUSE_BUTTON_LEFT) b = PBUTTON_S1;
    if (button == GLFW_MOUSE_BUTTON_RIGHT) b = PBUTTON_S2;
    PlatformButtonAction pba = PBA_HOLD;
    switch(action) {
        case GLFW_PRESS:
            pba = PBA_DOWN;
            break;
        case GLFW_RELEASE:
            pba = PBA_UP;
            break;
        default:
            break;
    }
    if (current_state.button)
        current_state.button(b, pba);
}

static inline int ckey(int k) {
    return GLFW_PRESS == glfwGetKey(game_window, k);
}

static void process_down_keys() {
    int left = ckey(GLFW_KEY_A) || ckey(GLFW_KEY_LEFT);
    int right = ckey(GLFW_KEY_D) || ckey(GLFW_KEY_RIGHT);
    int up = ckey(GLFW_KEY_W) || ckey(GLFW_KEY_UP);
    int down = ckey(GLFW_KEY_S) || ckey(GLFW_KEY_DOWN);
    platform_axes[2] = (left && !right) ? -1.0f : 0.0f;
    platform_axes[2] = (!left && right) ? 1.0f : platform_axes[2];
    platform_axes[3] = (down && !up) ? -1.0f : 0.0f;
    platform_axes[3] = (!down && up) ? 1.0f : platform_axes[3];
    if (current_state.button)
        for (int i = 0; i < PBUTTON_OTHER; i++)
            if (button_flags & (1 << i))
                current_state.button(i, PBA_HOLD);
}

static void window_resize_callback(int width, int height) {
    Gamestate gs = current_state;
    glViewport(0, 0, width, height);
    _platform_width = width;
    _platform_height = height;
    mat4_proj_ortho(screen_matrix, -1, width, height, 0, 0, 1);
    scene_resize(width, height);
    if (gs.resize)
        gs.resize(width, height);
}

static void error_callback(int error, const char * message) {
    uerr(message);
}


void platform_init() {

    glfwInit();

    glfwSetErrorCallback(&error_callback);

    // GLFW window and context creation
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 16);
	game_window = glfwCreateWindow(1000, 600, "Ldoom", NULL, NULL);
	if (!game_window) {
	    glfwTerminate();
    }
    glfwMakeContextCurrent(game_window);
    glfwSwapInterval(1);

    // Use GLAD to get stuff.
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        exit(1);
    }

    // GL initializing
    int width, height;
    glfwGetFramebufferSize(game_window, &width, &height);
    _platform_width = width;
    _platform_height = height;
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialize input
    glfwSetKeyCallback(game_window, &key_callback);
    glfwSetMouseButtonCallback(game_window, &mouse_button_callback);
    glfwSetCursorPosCallback(game_window, &cursor_position_callback);
    glfwSetInputMode(game_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Misc
    mat4_proj_ortho(screen_matrix, -1, width, height, 0, 0, 1);
#ifndef NOLOG
    ldlog_init();
#endif
#ifndef NOCONSOLE
    console_init();
#endif
    qd_init();
}

void platform_mainloop(Gamestate * initial_state) {
    current_index = 0;
    memset(&current_state, 0, sizeof(Gamestate));
    gamestate_switch(initial_state);
    double frametime = 0;
    double last_frametime = 0;

    int framecount = 0;
    double fps_check_time = glfwGetTime();
    while (!glfwWindowShouldClose(game_window)) {

        int width, height;
        glfwGetFramebufferSize(game_window, &width, &height);
        if (width != _platform_width && height != _platform_height) {
            window_resize_callback(width, height);
        }
        framecount++;
        last_frametime = frametime;
        frametime = glfwGetTime();
        glfwSwapBuffers(game_window);
        if (pointer_mode == PPOINTERMODE_LOCKED)
            platform_axes[0] = platform_axes[1] = 0.0f;
        glfwPollEvents();
        process_down_keys();
        _platform_delta = frametime - last_frametime;
        if (frametime > fps_check_time + 1) {
            _platform_fps = framecount / (frametime - fps_check_time);
            framecount = 0;
            fps_check_time = frametime;
            if (current_state.updateTick) {
                current_state.updateTick();
            }
        }
        gamestate_update(_platform_delta);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        gamestate_draw();
#ifndef NOCONSOLE
        console_draw();
#endif
    }
}

void platform_deinit() {

#ifndef NOCONSOLE
    console_deinit();
#endif
#ifndef NOLOG
    ldlog_deinit();
#endif
    qd_deinit();

    if (current_state.deinit) current_state.deinit();
    glfwDestroyWindow(game_window);
    glfwTerminate();
}

void platform_exit() {
    glfwSetWindowShouldClose(game_window, 1);
}

int platform_set_window(PlatformWindow * newWindow, PlatformWindow * result) {

    return 1;
}

#endif

//////////////////////////////////////// DESKTOP CODE END
