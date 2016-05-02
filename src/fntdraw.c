#include "fntdraw.h"
#include "platform.h"
#include "luainterop.h"
#include "shader.h"
#include "util.h"
#include <stdarg.h>
#include <stdlib.h>

#define CHAR_VERT_SIZE 8
#define CHAR_SIZE (CHAR_VERT_SIZE * 4)

// Begin Sparse Array implementation for kerning tables

typedef struct {
    unsigned long key;
    float value;
} SparseArrayBucket;

typedef struct {
    unsigned count;
    unsigned capacity;
    SparseArrayBucket * data;
} SparseArray;

static SparseArray * sarray_init(SparseArray * sa, unsigned capacity) {
    sa->capacity = capacity;
    sa->count = 0;
    sa->data = malloc(sizeof(SparseArrayBucket) * capacity);
    return sa;
}

static void sarray_deinit(SparseArray * sa) {
    free(sa->data);
}

static int sarray_bucket(const SparseArray * sa, unsigned long key) {
    SparseArrayBucket * low = sa->data;
    SparseArrayBucket * hi = sa->data + sa->count - 1;
    SparseArrayBucket * mid;
    while(hi >= low) {
        mid = low + (hi - low) / 2;
        if (mid->key > key) {
            hi = mid - 1;
        } else if (mid->key < key) {
            low = mid + 1;
        } else {
            return mid - sa->data;
        }
    }
    return (sa->data - low) - 1;
}

// Returns 0 by default, which is okay for kerning
static float sarray_get(const SparseArray * sa, unsigned long key) {
    int bucketi;
    if ((bucketi = sarray_bucket(sa, key)) >= 0) {
        return sa->data[bucketi].value;
    } else {
        return 0.0f;
    }
}

static void sarray_trim(SparseArray * sa) {
    sa->data = realloc(sa->data, sa->count * sizeof(SparseArrayBucket));
    sa->capacity = sa->count;
}

static void sarray_set(SparseArray * sa, unsigned long key, float value) {
    int bucketi;
    if ((bucketi = sarray_bucket(sa, key)) >= 0) { // Key already exists.
        sa->data[bucketi].value = value;
    } else { // Insert new bucket
        int dif = -bucketi - 1;
        sa->count++;
        if (sa->capacity < sa->count) {
            sa->capacity = 2 * sa->count;
            sa->data = realloc(sa->data, sa->capacity * sizeof(SparseArrayBucket));
        }
        memmove(sa->data + dif + 1, sa->data + dif, (sa->count - dif - 1) * sizeof(SparseArrayBucket));
        SparseArrayBucket * bucket = sa->data + dif;
        bucket->key = key;
        bucket->value = value;
    }
}

// End Sparse Array implementation

// Text Shaders start

static unsigned fntdef_count = 0;

// Distance Field Shader Uniform Locations
static int shader_tex_loc;
static int shader_mvp_loc;
static int shader_offset_loc;
static int shader_color_loc;
static int shader_smoothing_loc;
static int shader_threshold_loc;

// No Distance Field Shader Uniform Locations
static int nodf_shader_tex_loc;
static int nodf_shader_mvp_loc;
static int nodf_shader_offset_loc;
static int nodf_shader_color_loc;

// Text Shader Programs
static Program text_shader_program;
static Program text_shader_nodf_program;

static const char text_shader_nodf_source[] =
"#version 330 core\n"
"\n"
"uniform mat4 mvp;\n"
"uniform vec2 offset;\n"
"uniform sampler2D tex;\n"
"uniform vec4 textcolor;\n"
"\n"
"#ifdef VERTEX\n"
"\n"
"layout (location = 0) in vec2 position;\n"
"layout (location = 1) in vec2 texcoord;\n"
"layout (location = 2) in vec4 vcolor;\n"
"\n"
"smooth out vec2 vtexcoord;\n"
"smooth out vec4 _vcolor;\n"
"\n"
"void main() {\n"
"gl_Position = mvp * vec4(position + offset, 0.0, 1.0);\n"
"vtexcoord = texcoord;\n"
"_vcolor = vcolor;\n"
"}\n"
"\n"
"#endif\n"
"\n"
"#ifdef FRAGMENT\n"
"\n"
"smooth in vec2 vtexcoord;\n"
"smooth in vec4 _vcolor;\n"
"\n"
"out vec4 fragcolor;\n"
"\n"
"void main() {\n"
"fragcolor = texture(tex, vtexcoord) * textcolor * _vcolor;\n"
"}\n"
"\n"
"#endif\n";

static const char text_shader_source[] =
"#version 330 core\n"
"\n"
"uniform mat4 mvp;\n"
"uniform vec2 offset;\n"
"uniform sampler2D tex;\n"
"uniform vec4 textcolor;\n"
"uniform float smoothing;\n"
"uniform float threshold;\n"
"\n"
"#ifdef VERTEX\n"
"\n"
"layout (location = 0) in vec2 position;\n"
"layout (location = 1) in vec2 texcoord;\n"
"layout (location = 2) in vec4 vcolor;\n"
"\n"
"smooth out vec2 vtexcoord;\n"
"smooth out vec4 _vcolor;\n"
"\n"
"void main() {\n"
"gl_Position = mvp * vec4(position + offset, 0.0, 1.0);\n"
"vtexcoord = texcoord;\n"
"_vcolor = vcolor;\n"
"}\n"
"\n"
"#endif\n"
"\n"
"#ifdef FRAGMENT\n"
"\n"
"smooth in vec2 vtexcoord;\n"
"smooth in vec4 _vcolor;\n"
"\n"
"out vec4 fragcolor;\n"
"\n"
"void main() {\n"
"float a = texture(tex, vtexcoord).a;\n"
"fragcolor = vec4(textcolor.rgb, smoothstep(threshold - smoothing, threshold + smoothing, a) * textcolor.a) * _vcolor;\n"
"}\n"
"\n"
"#endif\n";

static void text_shader_init() {
    // Distance Field;
    program_init_quick(&text_shader_program, text_shader_source);
    shader_tex_loc = glGetUniformLocation(text_shader_program.id, "tex");
    shader_mvp_loc = glGetUniformLocation(text_shader_program.id, "mvp");
    shader_offset_loc = glGetUniformLocation(text_shader_program.id, "offset");
    shader_color_loc = glGetUniformLocation(text_shader_program.id, "textcolor");
    shader_smoothing_loc = glGetUniformLocation(text_shader_program.id, "smoothing");
    shader_threshold_loc = glGetUniformLocation(text_shader_program.id, "threshold");
    // No Distace Field
    program_init_quick(&text_shader_nodf_program, text_shader_nodf_source);
    nodf_shader_tex_loc = glGetUniformLocation(text_shader_nodf_program.id, "tex");
    nodf_shader_mvp_loc = glGetUniformLocation(text_shader_nodf_program.id, "mvp");
    nodf_shader_offset_loc = glGetUniformLocation(text_shader_nodf_program.id, "offset");
    nodf_shader_color_loc = glGetUniformLocation(text_shader_nodf_program.id, "textcolor");
}

static void text_shader_deinit() {
    program_deinit(&text_shader_program);
    program_deinit(&text_shader_nodf_program);
}

// Text Shaders end

// File Reader start

static void skip_whitespace(const char ** p) {
    const char * c = *p;
    while (*c == ' ' || *c == '\t' || *c == '\n' || *c == '\r')
        c++;
    *p = c;
}

static int is_eol(const char c) {
    return c == '\r' || c == '\n' || c == '\0';
}

static int is_whitespace(const char c) {
    return c <= 32;
}

static void skip_nonwhitespace(const char ** p) {
    const char * c = *p;
    while (*c > 32)
        c++;
    *p = c;
}

// Line Types
#define FNT_INFO_TYPE 0
#define FNT_COMMON_TYPE 1
#define FNT_PAGE_TYPE 2
#define FNT_CHARS_TYPE 3
#define FNT_CHAR_TYPE 4
#define FNT_KERNINGS_TYPE 5
#define FNT_KERNING_TYPE 6
#define FNT_UNKNOWN_TYPE 7

static int line_get_type(const char * str) {
    const char * end = str;
    skip_nonwhitespace(&end);
    size_t len = end - str;
    if (len == 4 && !strncmp("info", str, 4))
        return FNT_INFO_TYPE;
    if (len == 6 && !strncmp("common", str, 6))
        return FNT_COMMON_TYPE;
    if (len == 4 && !strncmp("page", str, 4))
        return FNT_PAGE_TYPE;
    if (len == 5 && !strncmp("chars", str, 5))
        return FNT_CHARS_TYPE;
    if (len == 4 && !strncmp("char", str, 4))
        return FNT_CHAR_TYPE;
    if (len == 8 && !strncmp("kernings", str, 8))
        return FNT_KERNINGS_TYPE;
    if (len == 7 && !strncmp("kerning", str, 7))
        return FNT_KERNING_TYPE;
    return FNT_UNKNOWN_TYPE;
}

static const char * line_find_value(const char * str, const char * key, char * buffer, unsigned buf_max_len) {
    const char * c = str;
    size_t len = strlen(key);
    int keyfound;
    for(;;) {
        skip_whitespace(&c);
        keyfound = 1;
        for (unsigned i = 0; i < len; i++) {
            if (c[i] != key[i])	{
                keyfound = 0;
                break;
            }
        }
        if (keyfound) {
            if (c[len] == '=') {
                c += len + 1;
                break;
            }
        }
        skip_nonwhitespace(&c);
        if (*c == 0)
            return NULL;
    }
    const char * end = c;
    skip_nonwhitespace(&end);
    end--;
    if ((*c == '"') && (*end == '"')) {
        c++;
        end--;
    }
    unsigned length = end - c + 1;
    if (length + 1 >= buf_max_len) {
        uerr("String buffer overflow.");
    }
    strncpy(buffer, c, length);
    buffer[length] = 0;
    return end;
}

static const char * line_next(const char * str) {
    if (str == NULL || *str == 0)
        return NULL;
    while (*++str != '\n' && *str != '\r' && *str != 0)
        ;
    if (*str == 0)
        return NULL;
    return ++str;
}

static void resolve_path_name(const char * path, const char * file, char * buf, unsigned max_len) {
    unsigned pathlen = strlen(path);
#if defined(_WIN32) || defined(WIN32)
    char sep = '\\';
#else
    char sep = '/';
#endif
    const char * c = path + pathlen;
    while (*c != sep && c != path)
        c--;
    if (c == path)
        uerr("Could not find file from path.");
    unsigned filelen = strlen(file);
    if (filelen + pathlen + 1 > max_len)
        uerr("Buffer overflow.");
    strncpy(buf, path, max_len);
    strncpy(buf + (c - path ) + 1, file, max_len - (c - path) - 1);
}

FontDef * fnt_init(FontDef * fd, const char * resource) {
    char * path = platform_res2file_ez(resource);
    if (fntdef_count == 0) {
        text_shader_init();
    }
    fntdef_count++;
    static const size_t BUFLEN = 100;
    char buf[BUFLEN];
    long source_len;
    char * source = util_slurp(path, &source_len);
    const char * srcp = source;
    // Extract basic font def information.
    line_find_value(srcp, "size", buf, BUFLEN);
    fd->size = atoi(buf);
    srcp = line_next(srcp);
    // Get some more basic information on the next line.
    line_find_value(srcp, "lineHeight", buf, BUFLEN);
    fd->lineHeight = atof(buf);
    line_find_value(srcp, "base", buf, BUFLEN);
    fd->base = atof(buf);
    srcp = line_next(srcp);
    // Get the texture
    line_find_value(srcp, "file", buf, BUFLEN);
    char buf2[BUFLEN];
    resolve_path_name(path, buf, buf2, BUFLEN);
    texture_init_file(&fd->tex, buf2, -1);
    srcp = line_next(srcp);
    // Get the number of characters
    line_find_value(srcp, "count", buf, BUFLEN);
    fd->charcount = atoi(buf);
    FontCharDef * cd = fd->chars = calloc(256, sizeof(FontCharDef));
    for(;;) {
        srcp = line_next(srcp);
        if (!srcp || (line_get_type(srcp) != FNT_CHAR_TYPE)) {
            break;
        }
        unsigned id;
        unsigned x, y, w, h;
        float xoff, yoff, xadvance;
        line_find_value(srcp, "id", buf, BUFLEN); id = atoi(buf);
        line_find_value(srcp, "x", buf, BUFLEN); x = atoi(buf);
        line_find_value(srcp, "y", buf, BUFLEN); y = atoi(buf);
        line_find_value(srcp, "width", buf, BUFLEN); w = atoi(buf);
        line_find_value(srcp, "height", buf, BUFLEN); h = atoi(buf);
        line_find_value(srcp, "xoffset", buf, BUFLEN); xoff = atof(buf);
        line_find_value(srcp, "yoffset", buf, BUFLEN); yoff = atof(buf);
        line_find_value(srcp, "xadvance", buf, BUFLEN); xadvance = atof(buf);
        // Ignore non ascii and extended characters. Unicode support would be awesome, though.
        if (id > 255) continue;
        cd[id].valid = 1;
        cd[id].x = x;
        cd[id].y = y;
        cd[id].w = w;
        cd[id].h = h;
        cd[id].xoffset = xoff;
        cd[id].yoffset = yoff;
        cd[id].xadvance = xadvance;
        cd[id].kerning.data = NULL;
    }
    // Infer tabs to be 4 x spaces
    if (!cd[9].valid && cd[32].valid) {
        cd[9].valid = 1;
        cd[9].x = cd[32].x;
        cd[9].y = cd[32].y;
        cd[9].w = cd[32].w;
        cd[9].h = cd[32].h;
        cd[9].xoffset = cd[32].xoffset;
        cd[9].yoffset = cd[32].yoffset;
        cd[9].xadvance = 4 * cd[32].xadvance;
        cd[9].kerning.data = NULL;
    }
    if (srcp)  {
        line_find_value(srcp, "count", buf, BUFLEN);
        int kerningcount = atoi(buf);
        for (int ii = 0; ii < kerningcount; ii++) {
            srcp = line_next(srcp);
            line_find_value(srcp, "first", buf, BUFLEN);
            unsigned long first = atoi(buf);
            line_find_value(srcp, "second", buf, BUFLEN);
            unsigned long second = atoi(buf);
            line_find_value(srcp, "amount", buf, BUFLEN);
            float amount = atof(buf);
            SparseArray * sa = (SparseArray *) &cd[first].kerning;
            if (!sa->data)
                sarray_init(sa, 10);

            sarray_set(sa, second, amount);
        }
        for (int id = 0; id < 128; id++) {
            if (cd[id].valid && cd[id].kerning.data) {
                sarray_trim((SparseArray *) &cd[id].kerning);
            }
        }
    }
    free(source);
    uqfree_if_needed();
    return fd;
}

// File Reader end

void fnt_deinit(FontDef * fd) {
    for (int i = 0; i < 128; i++) {
        FontCharDef * fcd = fd->chars + i;
        if (fcd->valid && fcd->kerning.data)
            sarray_deinit((SparseArray *) &fcd->kerning);
    }
    free(fd->chars);
    texture_deinit(&fd->tex);
    if (--fntdef_count == 0) {
        text_shader_deinit();
    }
}

TextOptions * fnt_default_options(FontDef * fd, TextOptions * out) {
    out->width = 500;
    out->halign = ALIGN_LEFT;
    out->valign = ALIGN_TOP;
    out->font = fd;
    out->pt = 14;
    out->dynamic = 1;
    out->useMarkup = 1;
    out->threshold = 0.5;
    out->smoothing = 1.0f / 16.0f;
    out->position[0] = out->position[1] = 0.0f;
    out->startColor[0] = out->startColor[1] = out->startColor[2] = out->startColor[3] = 1.0f;
    return out;
}

static const char * first_printable_token(const char * c, int escape) {
    if (!escape) return c;
    if (*c == FNTDRAW_ESCAPE) {
        char next = *++c;
        int n;
        switch(next) {
            case FNTDRAW_6COLOR:
                n = 7; break;
            case FNTDRAW_3COLOR:
                n = 4; break;
            case FNTDRAW_ALPHA:
                n = 3; break;
            default:
                return c;
        }
        while(n-- && *++c)
            ;
        return first_printable_token(c, escape);
    }
    return c;
}

static inline const char * next_token(const char * c, int escape) {
    if (*c == '\0')
        return c;
    else
        return first_printable_token(c + 1, escape);
}

static size_t fntdraw_string_join_len(int n, const char ** strings, const char * sep) {
    if (n < 1) return 0;
    size_t seplen = 0;
    if (sep) seplen = strlen(sep);
    size_t finallen = (n - 1) * seplen;
    for (int i = 0; i < n; i++) {
        finallen += strlen(strings[i]);
    }
    return finallen;
}

static void fntdraw_string_join(char * buffer, int n, const char ** strings, const char * sep) {
    char * current = buffer;
    for (int i = 0; i < n; i++) {
        if (i) {
            for (const char * c = sep; *c; c++) {
                *current++ = *c;
            }
        }
        for (const char * c = strings[i]; *c; c++) {
            *current++ = *c;
        }
    }
    *current = 0;
}

#define CHARNONE ((unsigned long) -1)

// Gets the kerning between two characters
static float get_kerning(const FontCharDef * fcd, unsigned long next) {
    if (!fcd->kerning.data)
        return 0.0f;
    SparseArray * sa = (SparseArray *) &fcd->kerning;
    return sarray_get(sa, next);
}

// Gets the width and kerning between two characters.
// Behavior depends on if c1 or c2 are CHARNONE
static float get_charwidth(const FontDef * fd, unsigned long c1, unsigned long c2) {
    const FontCharDef * fcds, * fcd1, * fcd2;
    if (c1 == CHARNONE) return get_charwidth(fd, c2, c1);
    fcds = fd->chars;
    fcd1 = fcds + c1; if (!fcd1->valid) fcd1 = fcds + ' ';
    if (c2 == CHARNONE) {
        if (c1 < 255 && is_eol(c1)) return 0;
        return fcd1->xadvance;
    }
    fcd2 = fcds + c2; if (!fcd2->valid) fcd2 = fcds + ' ';
    if (c2 == CHARNONE) return fcd1->xadvance;
    if (!fcd1->kerning.data)
        return fcd2->xadvance;
    SparseArray * sa = (SparseArray *) &fcd1->kerning;
    float kerning = sarray_get(sa, c2);
    return kerning + fcd2->xadvance;
}

char* escape(const char* buffer, size_t l){
    unsigned i,j;
    char esc_char[]= { '\a','\b','\f','\n','\r','\t','\v','\\'};
    char essc_str[]= {  'a', 'b', 'f', 'n', 'r', 't', 'v','\\'};
  char* dest  =  (char*)calloc( l*2,sizeof(char));
    char* ptr=dest;
    for(i=0;i<l;i++){
        for(j=0; j< 8 ;j++){
            if( buffer[i]==esc_char[j] ){
              *ptr++ = '\\';
              *ptr++ = essc_str[j];
                 break;
            }
        }
        if(j == 8 )
      *ptr++ = buffer[i];
    }
  *ptr='\0';
    return dest;
}

static void print_lines_debug(Text * t) {
    printf("t->line_count: %d\n", t->line_count);
    for (unsigned i = 0; i < t->line_count; i++) {
        TextLine * l = t->lines + i;
        char * mtext = escape(t->text + l->first, l->last - l->first);
        printf("\ttext:%s\n", mtext);
        printf("\t%d, %d, %d\n", l->first, l->last, l->visibleCharCount);
        free(mtext);
    }
}

static const char * append_line(Text * t, TextLine tl) {
    if (!t->lines) { // We're just refilling the old line buffer
        t->line_capacity = 10;
        t->lines = malloc(t->line_capacity * sizeof(TextLine));
    }
    unsigned line = t->line_count++;
    if (line >= t->line_capacity) {
        t->line_capacity = (line > 15 ? line * 1.4 : line * 2) + 1;
        t->lines = realloc(t->lines, t->line_capacity * sizeof(TextLine));
    }
    TextLine * newline = t->lines + line;
    *newline = tl;
    const char * ret = t->text + tl.last;
    if (is_eol(*ret)) ret++;
    while (*ret == ' ') ret++;
    return ret;
}

static void calc_wrap(Text * t) {
    TextLine tl;
    t->line_count = 0;
    const char * text = t->text;
    const char * textend = text + t->text_length;
    const FontDef * fd = t->fontdef;
    float max_width = t->max_width;
    float scale = t->pt / t->fontdef->size;
    int escape = t->flags & FNTDRAW_TEXT_MARKUP_BIT;
    unsigned lastCount, count;
    float current_length, valid_length;
    const char * first, * last;
    first = text;
    while (first < textend) {
        tl.first = first - text;
        const char * current = first_printable_token(first, escape);
        if (*current == '\0') {
            break;
        } else if (is_eol(*current)) {
            tl.width = 0; tl.last = current - text; tl.visibleCharCount = 0;
            first = append_line(t, tl);
            continue;
        }
        count = lastCount = 1;
        current_length = valid_length = get_charwidth(fd, *current, CHARNONE);
        int spaceEncountered = 0;
        while (1) {
            float dlen = get_charwidth(fd, *current, *next_token(current, escape)) * scale;
            current_length += dlen;
            current = next_token(current, escape);
            if (max_width != 0 && current_length > max_width) break;
            if (!spaceEncountered || is_whitespace(*current) || is_eol(*current)) {
                last = current;
                lastCount = count;
                valid_length = current_length;
                if (is_eol(*current))
                    break;
                if (!is_whitespace(*current))
                    spaceEncountered = 1;
            }
            count++;
        }
        tl.last = last - text;
        tl.visibleCharCount = lastCount;
        tl.width = valid_length;
        first = append_line(t, tl);
    }
    /* print_lines_debug(t); */
}

#undef CHARNONE
#undef ADDLINE

static unsigned calc_num_quads(Text * t) {
    unsigned num_quads = 0;
    unsigned line_count = t->line_count;
    TextLine * lines = t->lines;
    for (unsigned i = 0; i < line_count; i++) {
        num_quads += lines[i].visibleCharCount;
    }
    return num_quads;
}

static unsigned read_hex_digit(const char * c) {
    unsigned x = *c;
    if (x >= '0' && x <= '9')
        return x - '0';
    x |= 32;
    if (x >= 'a' && x <= 'f')
        return x - 'a' + 10;
    return 0;
}

static void fill_buffers(Text * t) {
    float xcurrent = 0;
    float ycurrent = 0;
    unsigned index = 0;
    GLushort * els = t->elementBuffer;
    GLfloat * vs = t->vertexBuffer;
    float tw = t->fontdef->tex.w;
    float th = t->fontdef->tex.h;
    float invtw = 1.0f / tw;
    float invth = 1.0f / th;
    float max_width = t->max_width;
    float scale = t->pt / (double) t->fontdef->size;
    float vcolor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    switch (t->valign) {
        case ALIGN_TOP:
            ycurrent = 0.0f;
            break;
        case ALIGN_BOTTOM:
            ycurrent = -(t->line_count * t->fontdef->lineHeight * scale);
            break;
        case ALIGN_CENTER:
            ycurrent = -0.5f * (t->line_count * t->fontdef->lineHeight * scale);
            break;
        default:
            break;
    }
    for (unsigned i = 0; i < t->line_count; i++) {
        TextLine tl = t->lines[i];
        switch(t->halign) {
            case ALIGN_LEFT:
                xcurrent = 0.0f;
                break;
            case ALIGN_RIGHT:
                xcurrent = max_width - tl.width;
                break;
            case ALIGN_CENTER:
                xcurrent = (max_width - tl.width) * 0.5f;
                break;
            default:
                xcurrent = 0.0f;
                break;
        }
        // Make a quad for each letter of the line.
        for (unsigned j = tl.first; j < tl.last; j++) {
            char c = t->text[j];
            if (is_eol(c)) break;
            // Test for special characters (Escape character)
            if ((t->flags & FNTDRAW_TEXT_MARKUP_BIT) && c == FNTDRAW_ESCAPE) {
                c = t->text[++j];
                if (is_eol(c)) break;
                switch(c) {
                    case FNTDRAW_ALPHA:
                        if (j + 2 >= tl.last) break;
                        vcolor[3] = (16 * read_hex_digit(t->text + j + 1) + read_hex_digit(t->text + j + 2)) / 255.0f;
                        j += 2;
                        continue;
                    case FNTDRAW_3COLOR:
                        if (j + 3 >= tl.last) break;
                        for (int ii = 0; ii < 3; ii++)
                            vcolor[ii] = (17 * read_hex_digit(t->text + j + ii + 1)) / 255.0f;
                        j += 3;
                        continue;
                    case FNTDRAW_6COLOR:
                        if (j + 6 >= tl.last) break;
                        for (int ii = 0; ii < 3; ii++)
                            vcolor[ii] = (16 * read_hex_digit(t->text + j + 2 * ii + 1) + read_hex_digit(t->text + j + 2 * ii + 2)) / 255.0f;
                        j += 6;
                        continue;
                    default:
                        break;
                }
            }
            FontCharDef * fcd = t->fontdef->chars + c;
            if (!fcd->valid) {
                fcd = t->fontdef->chars + ' ';
            }
            float kerning = get_kerning(fcd, t->text[j + 1]);
            float x1 = xcurrent + fcd->xoffset * scale;
            float x2 = x1 + fcd->w * scale;
            float y1 = ycurrent + fcd->yoffset * scale;
            float y2 = y1 + fcd->h * scale;
            float u1 = fcd->x * invtw;
            float u2 = u1 + fcd->w * invtw;
            float v1 = fcd->y * invth;
            float v2 = v1 + fcd->h * invth;
            GLfloat * tmp_vs = vs;
            // Neg x, Neg y corner
            tmp_vs[0] = x1; tmp_vs[1] = y1; tmp_vs[2] = u1; tmp_vs[3] = v1;
            tmp_vs += CHAR_VERT_SIZE;
            // Neg x, Pos y corner
            tmp_vs[0] = x1; tmp_vs[1] = y2; tmp_vs[2] = u1; tmp_vs[3] = v2;
            tmp_vs += CHAR_VERT_SIZE;
            // Pos x, Neg y corner
            tmp_vs[0] = x2; tmp_vs[1] = y1; tmp_vs[2] = u2; tmp_vs[3] = v1;
            tmp_vs += CHAR_VERT_SIZE;
            // Pos x, Pos y corner
            tmp_vs[0] = x2; tmp_vs[1] = y2; tmp_vs[2] = u2; tmp_vs[3] = v2;
            tmp_vs += CHAR_VERT_SIZE;
            // Set elements in buffer
            els[0] = index + 0;
            els[1] = index + 1;
            els[2] = index + 2;
            els[3] = index + 1;
            els[4] = index + 3;
            els[5] = index + 2;
            // Per Character attributes
            for(int ii = 0; ii < 4; ii++) {
                vs[CHAR_VERT_SIZE * ii + 4] = vcolor[0];
                vs[CHAR_VERT_SIZE * ii + 5] = vcolor[1];
                vs[CHAR_VERT_SIZE * ii + 6] = vcolor[2];
                vs[CHAR_VERT_SIZE * ii + 7] = vcolor[3];
            }
            // Increment the pointers for the next letter quad.
            vs += CHAR_SIZE;
            els += 6;
            index += 4;
            // Move the location of the next character to the right.
            xcurrent += (fcd->xadvance + kerning) * scale;
        }
        // Move the next line down.
        ycurrent += t->fontdef->lineHeight * scale;
    }
}

static void text_buffer_data(Text * t) {
    GLenum drawtype = (t->flags & FNTDRAW_TEXT_DYNAMIC_BIT) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    glBindBuffer(GL_ARRAY_BUFFER, t->VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, t->EBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * CHAR_SIZE * t->num_quads, t->vertexBuffer, drawtype);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 6 * t->num_quads, t->elementBuffer, drawtype);
}

void text_loadbuffer(Text * t) {
    if (t->flags & FNTDRAW_TEXT_LOADED_BIT) {
        return;
    }
    t->flags |= FNTDRAW_TEXT_LOADED_BIT;
    glGenBuffers(1, &t->VBO);
    glGenBuffers(1, &t->EBO);
    glGenVertexArrays(1, &t->VAO);
    glBindVertexArray(t->VAO);
    text_buffer_data(t);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, CHAR_VERT_SIZE * sizeof(GL_FLOAT), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, CHAR_VERT_SIZE * sizeof(GL_FLOAT), (GLvoid *)(2 * sizeof(GL_FLOAT)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, CHAR_VERT_SIZE * sizeof(GL_FLOAT), (GLvoid *)(4 * sizeof(GL_FLOAT)));
    glBindVertexArray(0);
}

void text_unloadbuffer(Text * t) {
    if (!(t->flags & FNTDRAW_TEXT_LOADED_BIT)) {
        return;
    }
    t->flags &= ~FNTDRAW_TEXT_LOADED_BIT;
    glBindVertexArray(0);
    glDeleteBuffers(1, &t->VBO);
    glDeleteBuffers(1, &t->EBO);
    glDeleteVertexArrays(1, &t->VAO);
}

static void update_buffers(Text * t) {
    unsigned new_num_quads = calc_num_quads(t);
    if (new_num_quads > t->quad_capacity) {
        t->quad_capacity = new_num_quads;
        size_t vbuf_size = sizeof(GLfloat) * CHAR_SIZE * new_num_quads;
        size_t ebuf_size = sizeof(GLushort) * 6 * new_num_quads;
        void * ptr = realloc(t->vertexBuffer, vbuf_size + ebuf_size);
        t->vertexBuffer = (ptr);
        t->elementBuffer = (ptr + vbuf_size);
    }
    t->num_quads = new_num_quads;
    fill_buffers(t);
    if (t->flags & FNTDRAW_TEXT_LOADED_BIT)
        text_buffer_data(t);
}

static void bind_shader(const Text * t, const mat4 mvp) {
    if (t->flags & FNTDRAW_TEXT_NODF_BIT) {
        glUseProgram(text_shader_nodf_program.id);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, t->fontdef->tex.id);
        glUniform1i(nodf_shader_tex_loc, 0);
        glUniform4fv(nodf_shader_color_loc, 1, t->color);
        glUniform2fv(nodf_shader_offset_loc, 1, t->position);
        glUniformMatrix4fv(nodf_shader_mvp_loc, 1, GL_FALSE, mvp);
    } else {
        glUseProgram(text_shader_program.id);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, t->fontdef->tex.id);
        glUniform1i(shader_tex_loc, 0);
        glUniform4fv(shader_color_loc, 1, t->color);
        glUniform2fv(shader_offset_loc, 1, t->position);
        glUniformMatrix4fv(shader_mvp_loc, 1, GL_FALSE, mvp);
        glUniform1f(shader_smoothing_loc, t->smoothing);
        glUniform1f(shader_threshold_loc, t->threshold);
    }
}

void text_draw(Text * t, const mat4 mvp) {
    bind_shader(t, mvp);
    glBindVertexArray(t->VAO);
    glDrawElements(GL_TRIANGLES, t->num_quads * 6, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
}

void text_draw_screen(Text * t) {
    text_draw(t, platform_screen_matrix());
}

void text_draw_range(Text * t, const mat4 mvp, unsigned start, unsigned length) {
    if (start + length > t->text_length) {
        uerr("Range not renderable.");
    }
    bind_shader(t, mvp);
    glBindVertexArray(t->VAO);
    glDrawElements(GL_TRIANGLES, length * 6, GL_UNSIGNED_SHORT, (GLvoid *)(start * 6 * sizeof(GLushort)));
    glBindVertexArray(0);
}

void text_draw_range_screen(Text * t, unsigned start, unsigned length) {
    text_draw_range(t, platform_screen_matrix(), start, length);
}

static void text_init_common(Text * t, const TextOptions * options, size_t slen) {
    // Initilaize basic variables
    t->flags = (options->dynamic ? FNTDRAW_TEXT_DYNAMIC_BIT : 0) |
               (options->useMarkup ? FNTDRAW_TEXT_MARKUP_BIT : 0);
    t->fontdef = options->font;
    t->text_length = slen;
    t->pt = options->pt;
    t->halign = options->halign;
    t->valign = options->valign;
    t->max_width = options->width;
    t->smoothing = options->smoothing;
    t->threshold = options->threshold;
    for (int i = 0; i < 4; i++)
        t->color[i] = options->startColor[i];
    for (int i = 0; i < 2; i++)
        t->position[0] = options->position[i];
    // Get the number of lines and store the line buffer.
    t->lines = NULL;
    t->line_count = 0;
    calc_wrap(t);
    // Calculate how many quads need to be drawn.
    t->num_quads = calc_num_quads(t);
    // Allocate vertex buffers
    size_t vbuf_size = sizeof(GLfloat) * CHAR_SIZE * t->num_quads;
    size_t ebuf_size = sizeof(GLushort) * 6 * t->num_quads;
    void * ptr = malloc(vbuf_size + ebuf_size);
    t->vertexBuffer = (ptr);
    t->elementBuffer = (ptr + vbuf_size);
    t->quad_capacity = t->num_quads;
    fill_buffers(t);
    text_loadbuffer(t);
}

Text * text_init(Text * t, const TextOptions * options, const char * text) {
    size_t slen = strlen(text);
    // Allocate text buffer
    t->text = malloc(slen + 1);
    t->text_capacity = slen;
    t->text_length = slen;
    memcpy(t->text, text, slen + 1);
    text_init_common(t, options, slen);
    return t;
}

Text * text_init_nocopy(Text * t, const TextOptions * options, char * text) {
    size_t slen = strlen(text);
    t->text = text;
    t->text_capacity = slen;
    t->text_length = slen;
    text_init_common(t, options, slen);
    return t;
}

Text * text_initn(Text * t, const TextOptions * options, const char * text, size_t len) {
    // Allocate text buffer
    t->text = malloc(len + 1);
    t->text_capacity = len;
    t->text_length = len;
    strncpy(t->text, text, len);
    text_init_common(t, options, len);
    return t;
}

void text_deinit(Text * t) {
    text_unloadbuffer(t);
    free(t->text);
    free(t->vertexBuffer);
    free(t->lines);
}

void text_set(Text * t, const char * newtext) {
    unsigned slen = strlen(newtext);
    t->text_length = slen;
    if (slen > t->text_capacity) {
        t->text_capacity = slen;
        t->text = realloc(t->text, slen + 1);
    }
    strcpy(t->text, newtext);
    calc_wrap(t);
    update_buffers(t);
}

void text_setn(Text * t, const char * string, size_t len) {
    t->text_length = len;
    if (len > t->text_capacity) {
        t->text_capacity = len;
        t->text = realloc(t->text, len + 1);
    }
    strncpy(t->text, string, len);
    calc_wrap(t);
    update_buffers(t);
}

// TEXT UTIL

size_t textutil_get_escapedlength(const char * text) {
    size_t ret = 0;
    const char * scanner = text;
    char last;
    while ((last = *scanner++) != '\0') {
        if (last == FNTDRAW_ESCAPE) {
            ret += 2;
        } else {
            ret++;
        }
    }
    return ret;
}

size_t textutil_get_visiblelength(const char * markup) {
    if (!markup) return 0;
    size_t ret = 1;
    while((markup = first_printable_token(markup + 1, 1)))
        ret++;
    return ret;
}

static void textutil_escape_impl(const char * text, char * buffer) {
    const char * scanner = text;
    char * printer = buffer;
    char last;
    while ((last = *scanner++) != '\0') {
        if (last == FNTDRAW_ESCAPE) {
            *printer++ = FNTDRAW_ESCAPE;
            *printer++ = FNTDRAW_ESCAPE;
        } else {
            *printer++ = last;
        }
    }
    *printer = '\0';
}

int textutil_escape(const char * text, size_t buflen, char * buffer) {
    size_t final_len = textutil_get_escapedlength(text);
    if (final_len + 1 <= buflen) {
        textutil_escape_impl(text, buffer);
    }
    return final_len;
}

int textutil_escape_inplace(char * text, size_t buflen) {
    size_t final_len = textutil_get_escapedlength(text);
    if (final_len < buflen) {
        int scanner = strlen(text);
        int printer = final_len;
        char last;
        while (scanner >= 0) {
            if ((last = text[scanner--]) == FNTDRAW_ESCAPE) {
                text[printer] = FNTDRAW_ESCAPE;
                text[printer - 1] = FNTDRAW_ESCAPE;
                printer -= 2;
            } else {
                text[printer--] = last;
            }
        }
    }
    return final_len;
}

char * textutil_escapex(const char * text) {
    size_t final_len = textutil_get_escapedlength(text);
    char * out = malloc(final_len + 1);
    textutil_escape_impl(text, out);
    return out;
}

static void textutil_normalize_impl(char * out, const char * in) {
    const char * leading = in;
    char * trailing = out;
    while (*leading != '\0') {
        *trailing = *leading;
        trailing++;
        leading = first_printable_token(leading + 1, 1);
    }
}

int textutil_normalize(const char * text, size_t buflen, char * buffer) {
    size_t final_len = textutil_get_visiblelength(text);
    if (final_len + 1 > buflen) return final_len;
    textutil_normalize_impl(buffer, text);
    return final_len;
}

char * textutil_normalizex(const char * text) {
    size_t final_len = textutil_get_visiblelength(text);
    char * out = malloc(final_len + 1);
    textutil_normalize_impl(out, text);
    return out;
}

void textutil_normalize_inplace(char * text) {
    textutil_normalize_impl(text, text);
}

char * textutil_join(int n, const char ** parts, const char * sep) {
    char * buf = malloc(fntdraw_string_join_len(n, parts, sep) + 1);
    fntdraw_string_join(buf, n, parts, sep);
    return buf;
}

// LUA INTEROP

static TextOptions fntdraw_lua_default_options;

static FontDef * luai_fnt_check(lua_State * L) {
    void * ud = luaL_checkudata(L, 1, "ldoom.Font");
    luaL_argcheck(L, ud != NULL, 1, "'ldoom.Font' expected.");
    return (FontDef *) ud;
}

static int luai_fntdraw_loadfont(lua_State * L) {
    const char * resource = lua_tostring(L, -1);
    FontDef * fd = lua_newuserdata(L, sizeof(FontDef));
    luaL_getmetatable(L, "ldoom.Font");
    lua_setmetatable(L, -2);
    fnt_init(fd, resource);
    return 1;
}

static int luai_fntdraw_deinit(lua_State * L) {
    FontDef * fd = luai_fnt_check(L);
    if (fd)
        fnt_deinit(fd);
    return 0;
}

static int luai_fnt_maketext(lua_State * L) {
    FontDef * fd = luai_fnt_check(L);
    if (fd) {
        TextOptions * to = &fntdraw_lua_default_options;
        to->pt = 24;
        to->font = fd;
        // TODO: Use custom text otions if provided.
        Text * text = lua_newuserdata(L, sizeof(Text));
        luaL_getmetatable(L, "ldoom.Text");
        lua_setmetatable(L, -2);
        size_t l;
        const char * string = lua_tolstring(L, 2, &l);
        text_initn(text, to, string, l);
        return 1;
    }
    return 0;
}

LUAI_MAKECHECKER(Text)
static Text * luai_text_check(lua_State * L) {
    void * ud = luaL_checkudata(L, 1, "ldoom.Text");
    luaL_argcheck(L, ud != NULL, 1, "'ldoom.Text' expected.");
    return (Text *) ud;
}

static int luai_text_deinit(lua_State * L) {
    Text * t = luai_text_check(L);
    if (t)
        text_deinit(t);
    return 0;
}

static int luai_text_draw(lua_State * L) {
    Text * t = luai_text_check(L);
    if (t) {
        text_draw_screen(t);
    }
    return 0;
}

LUAI_SETTER1N(Text, setX, t->position[0] = v1)
LUAI_SETTER1N(Text, setY, t->position[1] = v1)
LUAI_SETTER2N(Text, setPosition, t->position[0] = v1, t->position[1] = v2)
LUAI_SETTER1S(Text, setText, text_set(t, v1))

LUAI_GETTER1N(Text, getX, t->position[0])
LUAI_GETTER1N(Text, getY, t->position[1])
LUAI_GETTER2N(Text, getPosition, t->position[0], t->position[1])
LUAI_GETTER1S(Text, getText, t->text);

void fntdraw_loadlib() {
    fnt_default_options(NULL, &fntdraw_lua_default_options);
    const luaL_Reg textmethods[] = {
        {"draw", luai_text_draw},
        {"setX", LUAI_F(Text, setX)},
        {"setY", LUAI_F(Text, setY)},
        {"setPosition", LUAI_F(Text, setPosition)},
        {"setText", LUAI_F(Text, setText)},
        {"getX", LUAI_F(Text, getX)},
        {"getY", LUAI_F(Text, getY)},
        {"getPosition", LUAI_F(Text, getPosition)},
        {"getText", LUAI_F(Text, getText)},
        {NULL, NULL}
    };
    const luaL_Reg textmetamethods[] = {
        {"__gc", luai_text_deinit},
        {NULL, NULL}
    };
    const luaL_Reg fontmethods[] = {
        {"newText", luai_fnt_maketext},
        {NULL, NULL}
    };
    const luaL_Reg fontmetamethods[] = {
        {"__gc", luai_fntdraw_deinit},
        {NULL, NULL}
    };
    const luaL_Reg module[] = {
        {"loadFont", luai_fntdraw_loadfont},
        {NULL, NULL}
    };
    luai_newclass("ldoom.Text", textmethods, textmetamethods);
    luai_newclass("ldoom.Font", fontmethods, fontmetamethods);
    luai_addsubmodule("text", module);
}

#undef CHAR_VERT_SIZE
#undef CHAR_SIZE
