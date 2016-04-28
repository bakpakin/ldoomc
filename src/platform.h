#ifndef PLATFORM_HEADER
#define PLATFORM_HEADER

#include <stdarg.h>

//////////////////////////////////////// DEFINES
#if (defined __APPLE__ || defined _WIN32 || defined __linux__)
#define PLATFORM_DESKTOP
#endif
//////////////////////////////////////// END DEFINES

// Initialization
void platform_init();
void platform_deinit();

// Files and Resource

// An immutable resource
int platform_res2file(const char * resource, char * pathbuf, unsigned bufsize);

char * platform_res2file_ez(const char * resource);

// A persistent data file
int platform_data2file(const char * data, char * pathbuf, unsigned bufsize);

char * platform_data2file_ez(const char * data);

// Window Code

#define WINDOW_FULLSCREEN 0x01

typedef struct {
    int width;
    int height;
    int flags;
} PlatformWindow;

void platform_get_window(PlatformWindow * w);

int platform_width();
int platform_height();
const float * platform_screen_matrix(); // Returns the orthographic projection matrix to convert NDC to pixel space.

// Generic Input

typedef enum {
    PBUTTON_A,
    PBUTTON_B,
    PBUTTON_C,
    PBUTTON_D,
    PBUTTON_E,
    PBUTTON_S1,
    PBUTTON_S2,
    PBUTTON_SYS,
    PBUTTON_SPECIAL,
    PBUTTON_OTHER
} PlatformButton;

typedef enum {
    PBA_UP,
    PBA_DOWN,
    PBA_HOLD
} PlatformButtonAction;

typedef enum {
    PPOINTERMODE_LOCKED, // In locked mode, pointer axis 1 acts like a joystick
    PPOINTERMODE_FREE, // In free mode, pointer axis 1 behaves like a mouse. Polling the axis returns a value between -1 and 1.
    PPOINTERMODE_PIXEL, // Similar to free, except polling returns a number between 0 and and theh width or height.
} PlatformPointerMode;

typedef enum {
    PAXIS_X1,
    PAXIS_Y1,
    PAXIS_X2,
    PAXIS_Y2
} PlatformAxis;

// Input Polling
float platform_poll_axis(PlatformAxis a);
int platform_poll_button(PlatformButton b);

// Pointer / FPS Mode (Locked mouse vs free mouse)
PlatformPointerMode platform_get_pointer_mode();
void platform_set_pointer_mode(PlatformPointerMode mode);

// Flow Control
void platform_mainloop();
void platform_exit();
void platform_exit_now();

double platform_delta();
double platform_fps();

// Gamestates

/*
 * Gamestate system code
 */
typedef struct {
    void (*init)();
    void (*deinit)();
    void (*hide)();
    void (*show)();
    void (*update)(double dt);
    void (*button)(PlatformButton b, PlatformButtonAction a);
    void (*draw)();
    void (*resize)(int width, int height);
    void (*updateTick)();
} Gamestate;

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
