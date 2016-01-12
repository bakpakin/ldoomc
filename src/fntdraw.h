#ifndef FONTDRAW_HEADER
#define FONTDRAW_HEADER

#include "glfw.h"
#include "texture.h"
#include "ldmath.h"

typedef struct {
    unsigned char valid;
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

typedef enum {
    ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT, ALIGN_JUSTIFY, ALIGN_TOP, ALIGN_BOTTOM
} TextAlign;

typedef struct {
    unsigned first;
    unsigned last;
    float width;
} TextLine;

typedef struct {

    // Text data
    char * text;
    unsigned flags;
    unsigned text_length;

    // Rendering
    const FontDef * fontdef;
    GLuint VBO;
    GLuint EBO;
    GLuint VAO;
    GLushort * elementBuffer;
    GLfloat * vertexBuffer;
    unsigned num_quads;
    float smoothing;
    float threshold;
    float color[4];

    // Metrics
    float pt;
    float max_width;
    TextAlign halign;
    TextAlign valign;
    unsigned line_count;
    TextLine * lines;

} Text;

FontDef * fnt_init(FontDef * fd, const char * path);

void fnt_deinit(FontDef * fd);

Text * text_init(Text * t, const FontDef * fd, const char * text, float pt, TextAlign halign, TextAlign valign, float max_width);

void text_deinit(Text * t);

void text_loadbuffer(Text * t);

void text_unloadbuffer(Text * t);

void text_draw(Text * t, mat4 mvp);

void text_draw_range(Text * t, mat4 mvp, unsigned start, unsigned length);

#endif
