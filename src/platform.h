#ifndef PLATFORM_HEADER
#define PLATFORM_HEADER

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

// Flow Control
void platform_mainloop();
void platform_exit();
void platform_exit_now();

double platform_delta();
double platform_fps();

#endif
