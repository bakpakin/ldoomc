#ifndef CONFIG_HEADER
#define CONFIG_HEADER

#ifdef __APPLE__
    #define OPENGL_H <OpenGL/gl3.h>
#else
    #define OPENGL_H <GL/gl3.h>
#endif

#define SDL_H <SDL2/SDL.h>

#endif
