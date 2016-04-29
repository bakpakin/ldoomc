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
    struct {
        unsigned count;
        unsigned capacity;
        void * data;
    } kerning;
} FontCharDef;

typedef struct {
    unsigned size;
    float lineHeight;
    float base;
    Texture tex;
    unsigned charcount;
    FontCharDef * chars;
} FontDef;

#define FNTDRAW_ESCAPE '$'
#define FNTDRAW_6COLOR '#'
#define FNTDRAW_ALPHA 'A'
#define FNTDRAW_3COLOR '@'

#define FNTDRAW_TEXT_LOADED_BIT 0x01
#define FNTDRAW_TEXT_DYNAMIC_BIT 0x02
#define FNTDRAW_TEXT_NODF_BIT 0x04

typedef enum {
    ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT, ALIGN_JUSTIFY, ALIGN_TOP, ALIGN_BOTTOM
} TextAlign;

typedef struct {
    unsigned first;
    unsigned last;
    unsigned visibleCharCount;
    float width;
} TextLine;

typedef struct {
    FontDef * font;
    float pt;
    float smoothing;
    float threshold;
    TextAlign halign;
    TextAlign valign;
    float width;
    int dynamic;
    int distanceField;
    vec4 startColor;
    vec2 position;
} TextOptions;

typedef struct {

    // Misc
    unsigned flags;

    // Text data
    char * text;
    unsigned text_length;
    unsigned text_capacity;

    // Rendering
    const FontDef * fontdef;
    GLuint VBO; // Vertex Buffer Object for position and texture coordinate data
    GLuint EBO;
    GLuint VAO;
    GLushort * elementBuffer;
    GLfloat * vertexBuffer;
    unsigned num_quads;
    unsigned quad_capacity;
    float smoothing;
    float threshold;
    float color[4];

    // Metrics
    vec2 position;
    float pt;
    float max_width;
    TextAlign halign;
    TextAlign valign;
    unsigned line_count;
    TextLine * lines;

} Text;

FontDef * fnt_init(FontDef * fd, const char * resource);

void fnt_deinit(FontDef * fd);

TextOptions * fnt_default_options(FontDef * fd, TextOptions * out);

Text * text_init(Text * t, const TextOptions * options, const char * text);

Text * text_init_multi(Text * t, const TextOptions * options, int textcount, ...);

void text_set_multi(Text * t, int textcount, ...);

void text_set(Text * t, const char * newtext);

void text_format(Text * t, size_t maxlength, const char * format, ...);

void text_deinit(Text * t);

void text_loadbuffer(Text * t);

void text_unloadbuffer(Text * t);

void text_draw(Text * t, const mat4 mvp);

void text_draw_screen(Text * t);

void text_draw_range(Text * t, const mat4 mvp, unsigned start, unsigned length);

void text_draw_range_screen(Text * t, unsigned start, unsigned length);

#endif
