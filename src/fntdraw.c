#include "fntdraw.h"
#include "platform.h"

#include <stdlib.h>
#include "shader.h"
#include "mesh.h"
#include "util.h"

static void skip_whitespace(const char ** p) {
    const char * c = *p;
    while (*c == ' ' || *c == '\t' || *c == '\n' || *c == '\r')
        c++;
    *p = c;
}

static void skip_whitespace_notabs(const char ** p) {
    const char * c = *p;
    while (*c == ' '|| *c == '\n' || *c == '\r')
        c++;
    *p = c;
}

static void skip_nonwhitespace(const char ** p) {
    const char * c = *p;
    while (*c != ' ' && *c != '\t' && *c != '\n' && *c != '\r' && *c != 0)
        c++;
    *p = c;
}

static const char * line_find_value(const char * str, const char * key, char * buffer, unsigned buf_max_len) {
	const char * c = str;
	size_t len = strlen(key);
	int keyfound;
	for(;;) {
	    skip_whitespace(&c);
	    keyfound = 1;
		for (int i = 0; i < len; i++) {
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
	while (*++str != '\n' && *str != 0)
		;
	if (*str == 0)
		return NULL;
	return ++str;
}

static void resolve_path_name(const char * path, const char * file, char * buf, unsigned max_len) {
	int pathlen = strlen(path);
	int last_sep_index = pathlen;
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
    int filelen = strlen(file);
    if (filelen + pathlen + 1 > max_len)
        uerr("Buffer overflow.");

    strncpy(buf, path, max_len);
    strncpy(buf + (c - path ) + 1, file, max_len - (c - path) - 1);
}

static int shader_tex_loc;
static int shader_mvp_loc;
static int shader_color_loc;
static int shader_smoothing_loc;
static int shader_threshold_loc;

static unsigned fntdef_count = 0;
static Program text_shader_program;

static const char text_shader_source[] =
"#version 330 core\n"
"\n"
"uniform mat4 mvp;\n"
"uniform sampler2D tex;\n"
"uniform vec4 textcolor;\n"
"uniform float smoothing;\n"
"uniform float threshold;\n"
"\n"
"#ifdef VERTEX\n"
"\n"
"layout (location = 0) in vec2 position;\n"
"layout (location = 1) in vec2 texcoord;\n"
"\n"
"smooth out vec2 vtexcoord;\n"
"\n"
"void main() {\n"
    "gl_Position = mvp * vec4(position, 0.0, 1.0);\n"
    "vtexcoord = texcoord;\n"
"}\n"
"\n"
"#endif\n"
"\n"
"#ifdef FRAGMENT\n"
"\n"
"in vec2 vtexcoord;\n"
"\n"
"out vec4 fragcolor;\n"
"\n"
"void main() {\n"
    "float a = texture(tex, vtexcoord).a;\n"
    "fragcolor = vec4(textcolor.rgb, smoothstep(threshold - smoothing, threshold + smoothing, a) * textcolor.a);\n"
"}\n"
"\n"
"#endif\n";

static void text_shader_init() {
    program_init_quick(&text_shader_program, text_shader_source);

    shader_tex_loc = glGetUniformLocation(text_shader_program.id, "tex");
    shader_mvp_loc = glGetUniformLocation(text_shader_program.id, "mvp");
    shader_color_loc = glGetUniformLocation(text_shader_program.id, "textcolor");
    shader_smoothing_loc = glGetUniformLocation(text_shader_program.id, "smoothing");
    shader_threshold_loc = glGetUniformLocation(text_shader_program.id, "threshold");
}

static void text_shader_deinit() {
    program_deinit(&text_shader_program);
}

FontDef * fnt_init(FontDef * fd, const char * resource) {

	#define BUFLEN 200

    char path[BUFLEN];
    platform_res2file(resource, path, BUFLEN);

    if (fntdef_count == 0) {
        text_shader_init();
    }
    fntdef_count++;


	char buf[BUFLEN];

	long source_len;
	unsigned vallen;
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

	FontCharDef * cd = fd->chars = calloc(128, sizeof(FontCharDef));

	while ((srcp = line_next(srcp))) {

		char id;
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

		cd[id].valid = 1;
		cd[id].x = x;
		cd[id].y = y;
		cd[id].w = w;
		cd[id].h = h;
		cd[id].xoffset = xoff;
		cd[id].yoffset = yoff;
		cd[id].xadvance = xadvance;

    }

	free(source);

	return fd;
}

void fnt_deinit(FontDef * fd) {
	free(fd->chars);
	texture_deinit(&fd->tex);
    if (--fntdef_count == 0) {
        text_shader_deinit();
    }
}

// Count the number of lines that need to be rendered in order to fit
// in the space given. This inlcudes newlines and a linebreaks inserted
// when a word goes outside the clip width.
static void calc_wrap(Text * t, const char * text) {

    const FontDef * fd = t->fontdef;
    float max_width = t->max_width;
    float scale = t->pt / (double) fd->size;
    unsigned capacity = 10;
    TextLine * lines = malloc(capacity * sizeof(TextLine));

    unsigned line = 0;
    float current_length = 0.0f;
    const char * first = text;
    const char * last = text;
    while (*first != '\0') {
        if (line >= capacity) {
            capacity = line * 2;
            lines = realloc(lines, capacity * sizeof(TextLine));
        }
        current_length = 0.0f;
        last = first;
        while ((max_width == 0 || current_length <= max_width) && *last != '\0' && *last != '\n') {
            FontCharDef * fcd = fd->chars + *last++;
            if (!fcd->valid) {
                fcd = fd->chars + ' ';
            }
            current_length += fcd->xadvance * scale;
        }
        const char * old_last = last;
        while (*last != ' '  &&
               *last != '\t' &&
               *last != '\r' &&
               *last != '\n' &&
               *last != '\0' &&
               last > first) {
            FontCharDef * fcd = fd->chars + *--last;
            if (!fcd->valid) {
                fcd = fd->chars + ' ';
            }
            current_length -= fcd->xadvance * scale;
        }
        if (last == first) {
            last = old_last - 1;
        } else {
            last--;
        }

        lines[line].first = first - text;
        lines[line].last = last - text;
        lines[line].width = current_length;
        line++;
        first = last + 1;
        skip_whitespace_notabs(&first);
    }
    t->lines = lines = realloc(lines, line * sizeof(TextLine));
    t->line_count = line;

    // Calculate the number of drawn characters
    unsigned num_quads = 0;
    for (int i = 0; i < t->line_count; i++) {
        num_quads += lines[i].last - lines[i].first + 1;
    }
    t->num_quads = num_quads;
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
        for (int j = tl.first; j <= tl.last; j++) {

            char c = t->text[j];
            FontCharDef * fcd = t->fontdef->chars + c;
            if (!fcd->valid) {
                fcd = t->fontdef->chars + ' ';
            }

            float x1 = xcurrent + fcd->xoffset * scale;
            float x2 = x1 + fcd->w * scale;
            float y1 = ycurrent + fcd->yoffset * scale;
            float y2 = y1 + fcd->h * scale;

            float u1 = fcd->x * invtw;
            float u2 = u1 + fcd->w * invtw;
            float v1 = fcd->y * invth;
            float v2 = v1 + fcd->h * invth;

            // Neg x, Neg y corner
            vs[0] = x1; vs[1] = y1; vs[2] = u1; vs[3] = v1;

            // Neg x, Pos y corner
            vs[4] = x1; vs[5] = y2; vs[6] = u1; vs[7] = v2;

            // Pos x, Neg y corner
            vs[8] = x2; vs[9] = y1; vs[10] = u2; vs[11] = v1;

            // Pos x, Pos y corner
            vs[12] = x2; vs[13] = y2; vs[14] = u2; vs[15] = v2;

            els[0] = index + 0;
            els[1] = index + 1;
            els[2] = index + 2;
            els[3] = index + 1;
            els[4] = index + 3;
            els[5] = index + 2;

            // Increment the pointers for the next letter quad.
            els += 6;
            vs += 16;
            index += 4;

            xcurrent += fcd->xadvance * scale;
        }

        ycurrent += t->fontdef->lineHeight * scale;

    }
}

void text_loadbuffer(Text * t) {

    if (t->flags & FNTDRAW_TEXT_LOADED_BIT) {
        return;
    }

    t->flags |= FNTDRAW_TEXT_LOADED_BIT;
    unsigned slen = t->text_length;

    glGenBuffers(1, &t->VBO);
    glGenBuffers(1, &t->EBO);
    glGenVertexArrays(1, &t->VAO);

    glBindVertexArray(t->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, t->VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, t->EBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 16 * slen, t->vertexBuffer, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 6 * slen, t->elementBuffer, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GL_FLOAT), 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GL_FLOAT), (GLvoid *)(2 * sizeof(GL_FLOAT)));

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

Text * text_init(Text * t, const FontDef * fd, const char * text, float pt, TextAlign halign, TextAlign valign, float max_width) {

    size_t slen = strlen(text);

    // Initilaize basic variables
    t->flags = 0;
    t->fontdef = fd;
    t->text_length = slen;
    t->pt = pt;
    t->halign = halign;
    t->valign = valign;
    t->max_width = max_width;
    t->smoothing = 1.0f / 16.0f;
    t->threshold = 0.5f;
    t->color[0] = 1.0f;
    t->color[1] = 1.0f;
    t->color[2] = 1.0f;
    t->color[3] = 1.0f;

    calc_wrap(t, text);

    size_t vbuf_size = sizeof(GLfloat) * 16 * t->num_quads;
    size_t ebuf_size = sizeof(GLushort) * 6 * t->num_quads;

    void * ptr = malloc(vbuf_size + ebuf_size + slen + 1);
    t->vertexBuffer = (ptr);
    t->elementBuffer = (ptr + vbuf_size);
    t->text = (ptr + vbuf_size + ebuf_size);
    memcpy(t->text, text, slen + 1);

    fill_buffers(t);

    text_loadbuffer(t);

    return t;
}

void text_deinit(Text * t) {

    text_unloadbuffer(t);

    free(t->vertexBuffer);
    free(t->lines);
}

static inline void bind_shader(const Text * t, mat4 mvp) {

    glUseProgram(text_shader_program.id);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, t->fontdef->tex.id);
    glUniform1i(shader_tex_loc, 0);
    glUniform4fv(shader_color_loc, 1, t->color);
    glUniformMatrix4fv(shader_mvp_loc, 1, GL_FALSE, mvp);
    glUniform1f(shader_smoothing_loc, t->smoothing);
    glUniform1f(shader_threshold_loc, t->threshold);

}

void text_draw(Text * t, mat4 mvp) {

    bind_shader(t, mvp);

    glBindVertexArray(t->VAO);
    glDrawElements(GL_TRIANGLES, t->text_length * 6, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);

}

void text_draw_range(Text * t, mat4 mvp, unsigned start, unsigned length) {

    if (start + length > t->text_length) {
        uerr("Range not renderable.");
    }

    bind_shader(t, mvp);

    glBindVertexArray(t->VAO);
    glDrawElements(GL_TRIANGLES, length * 6, GL_UNSIGNED_SHORT, (GLvoid *)(start * 6 * sizeof(GLushort)));
    glBindVertexArray(0);

}
