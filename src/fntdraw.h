#ifndef FONTDRAW_HEADER
#define FONTDRAW_HEADER

#include "glfw.h"
#include "texture.h"
#include "ldmath.h"

typedef struct {
    unsigned x;
    unsigned y;
    unsigned w;
    unsigned h;
    float xoffset;
    float yoffset;
    float xadvance;
} FontCharDef;

typedef struct {
    unsigned size;
    float lineHeight;
    float base;
    Texture tex;
    unsigned charcount;
    FontCharDef * chars;
} FontDef;

#define FNTDRAW_TEXT_LOADED_BIT 0x01

typedef struct {
    const FontDef * fontdef;
    char * text;
    unsigned flags;
    unsigned text_length;
    unsigned text_capacity;
    GLuint VBO;
    GLuint EBO;
    GLuint VAO;
    GLushort * elementBuffer;
    GLfloat * vertexBuffer;

    // Metrics
    unsigned line_count;
    unsigned * line_lengths;
    float * line_widths;
} Text;

FontDef * fnt_init(FontDef * fd, const char * path);

void fnt_deinit(FontDef * fd);

Text * text_init(Text * t, const FontDef * fd, const char * text);

void text_deinit(Text * t);

void text_append(Text * t, const char * text);

void text_clear(Text * t);

void text_insert(Text * t, unsigned index, const char * text);

void text_remove(Text * t, unsigned index, unsigned count);

void text_loadbuffer(Text * t);

void text_unloadbuffer(Text * t);

void text_draw(Text * t, mat4 mvp, vec4 color);

#endif
