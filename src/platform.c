#include "platform.h"
#include "util.h"
#include "ldmath.h"
#include "console.h"
#include "quickdraw.h"
#include "scene.h"
#include "luainterop.h"
#include "fntdraw.h"
#include "glfw.h"
#include "audio.h"
#include <string.h>
#include <ctype.h>

// Set up Lua Interop

static const LuaEventSignature les_tick = { "tick", 0, NULL };
static const LuaEventSignature les_draw = { "draw", 0, NULL };
static const LuaEventSignature les_load = { "load", 0, NULL };
static const LuaEventSignature les_unload = { "unload", 0, NULL };

static const int les_update_args[] = { LUA_TNUMBER };
static const LuaEventSignature les_update = { "update", 1, les_update_args };

static const int les_mouse_args[] = { LUA_TNUMBER, LUA_TSTRING, LUA_TNUMBER, LUA_TNUMBER };
static const LuaEventSignature les_mouse = { "mouse", 4, les_mouse_args };

static const int les_keyboard_args[] = { LUA_TSTRING, LUA_TSTRING, LUA_TNUMBER, LUA_TNUMBER };
static const LuaEventSignature les_keyboard = { "keyboard", 4, les_keyboard_args };

static const int les_error_args[] = { LUA_TSTRING };
static const LuaEventSignature les_error = { "error", 1, les_error_args };

static const int les_resize_args[] = { LUA_TNUMBER, LUA_TNUMBER };
static const LuaEventSignature les_resize = { "resize", 2, les_resize_args };

static const int les_wheel_args[] = { LUA_TNUMBER, LUA_TNUMBER };
static const LuaEventSignature les_wheel = { "wheel", 2, les_wheel_args };

// Platform stuff

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

static mat4 screen_matrix = {1.0, 0.0, 0.0, 0.0,
                             0.0, 1.0, 0.0, 0.0,
                             0.0, 0.0, 1.0, 0.0,
                             0.0, 0.0, 0.0, 1.0};

const float * platform_screen_matrix() {
    return screen_matrix;
}

#ifdef RELEASE
//Platform specific resource resolution
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
// End platform specific resource resolution
#else

static char *platform_path_predicate = "resources"
#ifdef _WIN32
"\\";
#else
"/";
#endif

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

char * platform_res2file_ez(const char * resource) {
    uqfree_if_needed();
    size_t slen = strlen(resource);
    char * pathbuf = uqmalloc(slen + platform_buffer_predsize);
    memcpy(pathbuf, platform_path_predicate, platform_buffer_predsize);
    strcpy(pathbuf + platform_buffer_predsize, resource);
    return pathbuf;
}

#endif

static GLFWwindow * game_window;

static const char * platform_get_action(int action) {
    switch (action) {
        case GLFW_PRESS: return "down";
        case GLFW_RELEASE: return "up";
        case GLFW_REPEAT: return "repeat";
        default: return "unknown";
    }
}

static const char * platform_get_key(int key) {
    switch(key) {
        case GLFW_KEY_SPACE: return " ";
        case GLFW_KEY_APOSTROPHE: return "'";
        case GLFW_KEY_COMMA: return ",";
        case GLFW_KEY_MINUS: return "-";
        case GLFW_KEY_PERIOD: return ".";
        case GLFW_KEY_SLASH: return "/";
        case GLFW_KEY_0: return "0";
        case GLFW_KEY_1: return "1";
        case GLFW_KEY_2: return "2";
        case GLFW_KEY_3: return "3";
        case GLFW_KEY_4: return "4";
        case GLFW_KEY_5: return "5";
        case GLFW_KEY_6: return "6";
        case GLFW_KEY_7: return "7";
        case GLFW_KEY_8: return "8";
        case GLFW_KEY_9: return "9";
        case GLFW_KEY_SEMICOLON: return ";";
        case GLFW_KEY_EQUAL: return "=";
        case GLFW_KEY_A: return "a";
        case GLFW_KEY_B: return "b";
        case GLFW_KEY_C: return "c";
        case GLFW_KEY_D: return "d";
        case GLFW_KEY_E: return "e";
        case GLFW_KEY_F: return "f";
        case GLFW_KEY_G: return "g";
        case GLFW_KEY_H: return "h";
        case GLFW_KEY_I: return "i";
        case GLFW_KEY_J: return "j";
        case GLFW_KEY_K: return "k";
        case GLFW_KEY_L: return "l";
        case GLFW_KEY_M: return "m";
        case GLFW_KEY_N: return "n";
        case GLFW_KEY_O: return "o";
        case GLFW_KEY_P: return "p";
        case GLFW_KEY_Q: return "q";
        case GLFW_KEY_R: return "r";
        case GLFW_KEY_S: return "s";
        case GLFW_KEY_T: return "t";
        case GLFW_KEY_U: return "u";
        case GLFW_KEY_V: return "v";
        case GLFW_KEY_W: return "w";
        case GLFW_KEY_X: return "x";
        case GLFW_KEY_Y: return "y";
        case GLFW_KEY_Z: return "z";
        case GLFW_KEY_LEFT_BRACKET: return "[";
        case GLFW_KEY_BACKSLASH: return "\\";
        case GLFW_KEY_RIGHT_BRACKET: return "/";
        case GLFW_KEY_GRAVE_ACCENT: return "`";
        case GLFW_KEY_WORLD_1: return "world1";
        case GLFW_KEY_WORLD_2: return "world2";
        case GLFW_KEY_ESCAPE: return "escape";
        case GLFW_KEY_ENTER: return "enter";
        case GLFW_KEY_TAB: return "tab";
        case GLFW_KEY_BACKSPACE: return "backspace";
        case GLFW_KEY_INSERT: return "insert";
        case GLFW_KEY_DELETE: return "delete";
        case GLFW_KEY_RIGHT: return "right";
        case GLFW_KEY_LEFT: return "left";
        case GLFW_KEY_DOWN: return "down";
        case GLFW_KEY_UP: return "up";
        case GLFW_KEY_PAGE_UP: return "pageup";
        case GLFW_KEY_PAGE_DOWN: return "pagedown";
        case GLFW_KEY_HOME: return "home";
        case GLFW_KEY_END: return "end";
        case GLFW_KEY_CAPS_LOCK: return "capslock";
        case GLFW_KEY_SCROLL_LOCK: return "scrolllock";
        case GLFW_KEY_NUM_LOCK: return "numlock";
        case GLFW_KEY_PRINT_SCREEN: return "printscreen";
        case GLFW_KEY_PAUSE: return "pause";
        case GLFW_KEY_F1: return "f1";
        case GLFW_KEY_F2: return "f2";
        case GLFW_KEY_F3: return "f3";
        case GLFW_KEY_F4: return "f4";
        case GLFW_KEY_F5: return "f5";
        case GLFW_KEY_F6: return "f6";
        case GLFW_KEY_F7: return "f7";
        case GLFW_KEY_F8: return "f8";
        case GLFW_KEY_F9: return "f9";
        case GLFW_KEY_F10: return "f10";
        case GLFW_KEY_F11: return "f11";
        case GLFW_KEY_F12: return "f12";
        case GLFW_KEY_F13: return "f13";
        case GLFW_KEY_F14: return "f14";
        case GLFW_KEY_F15: return "f15";
        case GLFW_KEY_F16: return "f16";
        case GLFW_KEY_F17: return "f17";
        case GLFW_KEY_F18: return "f18";
        case GLFW_KEY_F19: return "f19";
        case GLFW_KEY_F20: return "f20";
        case GLFW_KEY_F21: return "f21";
        case GLFW_KEY_F22: return "f22";
        case GLFW_KEY_F23: return "f23";
        case GLFW_KEY_F24: return "f24";
        case GLFW_KEY_F25: return "f25";
        case GLFW_KEY_KP_0: return "kp0";
        case GLFW_KEY_KP_1: return "kp1";
        case GLFW_KEY_KP_2: return "kp2";
        case GLFW_KEY_KP_3: return "kp3";
        case GLFW_KEY_KP_4: return "kp4";
        case GLFW_KEY_KP_5: return "kp5";
        case GLFW_KEY_KP_6: return "kp6";
        case GLFW_KEY_KP_7: return "kp7";
        case GLFW_KEY_KP_8: return "kp8";
        case GLFW_KEY_KP_9: return "kp9";
        case GLFW_KEY_KP_DECIMAL: return "kp.";
        case GLFW_KEY_KP_DIVIDE: return "kp/";
        case GLFW_KEY_KP_MULTIPLY: return "kp*";
        case GLFW_KEY_KP_SUBTRACT: return "kp-";
        case GLFW_KEY_KP_ADD: return "kp+";
        case GLFW_KEY_KP_ENTER: return "kpenter";
        case GLFW_KEY_KP_EQUAL: return "kpequal";
        case GLFW_KEY_LEFT_SHIFT: return "lshift";
        case GLFW_KEY_LEFT_CONTROL: return "lcontorl";
        case GLFW_KEY_LEFT_ALT: return "lalt";
        case GLFW_KEY_LEFT_SUPER: return "lsuper";
        case GLFW_KEY_RIGHT_SHIFT: return "rshift";
        case GLFW_KEY_RIGHT_CONTROL: return "rcontorl";
        case GLFW_KEY_RIGHT_ALT: return "ralt";
        case GLFW_KEY_RIGHT_SUPER: return "rsuper";
        case GLFW_KEY_MENU: return "menu";
        case GLFW_KEY_UNKNOWN:
        default: return "unknown";
    }
}

static void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods) {
    if (window != game_window) return;
    luai_event(&les_keyboard, platform_get_key(key), platform_get_action(action), (double) scancode, (double) mods);
}

static void mouse_button_callback(GLFWwindow * window, int button, int action, int mods) {
    if (window != game_window) return;
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    luai_event(&les_mouse, (double) button, platform_get_action(action), xpos, ypos);
}

static void window_resize_callback(int width, int height) {
    glViewport(0, 0, width, height);
    _platform_width = width;
    _platform_height = height;
    mat4_proj_ortho(screen_matrix, -1, width, height, 0, 0, 1);
    luai_event(&les_resize, (double) width, (double) height);
}

static void error_callback(int error, const char * message) {
    fprintf(stderr, "Error code %d: ", error);
    uerr(message);
}

void platform_mainloop() {
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
        glfwPollEvents();
        _platform_delta = frametime - last_frametime;
        if (frametime > fps_check_time + 1) {
            _platform_fps = framecount / (frametime - fps_check_time);
            framecount = 0;
            fps_check_time = frametime;
            luai_event(&les_tick);
        }
        luai_event(&les_update, _platform_delta);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        luai_event(&les_draw);
        console_draw();
    }
}

void platform_exit() {
    glfwSetWindowShouldClose(game_window, 1);
}

// LUA INTEROP

static int luai_platform_quit(lua_State * L) {
    platform_exit();
    return 0;
}

// INITIALIZATION / DEINITIALIZATION

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

    GLFWmonitor * monitor = glfwGetPrimaryMonitor();
    int mcount;
    const GLFWvidmode * mode = glfwGetVideoMode(monitor);
    const GLFWvidmode * modes = glfwGetVideoModes(monitor, &mcount);
    int max = 0;
    for (int i = 0; i < mcount; i++) {
        const GLFWvidmode * m = modes + i;
        int size = m->width + m->height + m->refreshRate;
        if (size > max) {
            max = size;
            mode = m;
        }
    }

    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

	game_window = glfwCreateWindow(mode->width, mode->height, "Ldoom", monitor, NULL);

	if (!game_window) {
	    glfwTerminate();
    }
    glfwMakeContextCurrent(game_window);
    glfwSwapInterval(1);

    // Use GLAD to get stuff.
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        glfwTerminate();
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
    glfwSetInputMode(game_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Misc
    mat4_proj_ortho(screen_matrix, -1, width, height, 0, 0, 1);

    luai_init();
    const luaL_Reg module[] = {
        {"quit", luai_platform_quit},
        {NULL, NULL}
    };
    luai_addtomainmodule(module);
    console_init();
    qd_init();
    audio_init();
    fntdraw_loadlib();

    luai_doresource("scripts/bootstrap.lua");

    luai_event(&les_load);
}

void platform_deinit() {

    luai_event(&les_unload);

    console_deinit();
    qd_deinit();
    audio_deinit();

    luai_deinit();

    glfwDestroyWindow(game_window);
    glfwTerminate();

}
