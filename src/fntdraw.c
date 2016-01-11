#include "fntdraw.h"

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

static void skip_nonwhitespace(const char ** p) {
    const char * c = *p;
    while (*c != ' ' && *c != '\t' && *c != '\n' && *c != '\r')
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

static unsigned fntdef_count = 0;
static Program text_shader_program;

static void text_shader_init() {
    program_init_file(&text_shader_program, "../resources/fontdraw.glsl");

    shader_tex_loc = glGetUniformLocation(text_shader_program.id, "tex");
    shader_mvp_loc = glGetUniformLocation(text_shader_program.id, "mvp");
    shader_color_loc = glGetUniformLocation(text_shader_program.id, "textcolor");
    shader_smoothing_loc = glGetUniformLocation(text_shader_program.id, "smoothing");
}

static void text_shader_deinit() {
    program_deinit(&text_shader_program);
}

FontDef * fnt_init(FontDef * fd, const char * path) {

    if (fntdef_count == 0) {
        text_shader_init();
    }
    fntdef_count++;

	#define BUFLEN 40

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

static void fill_buffers(Text * t) {

    float xcurrent = 0;
    float ycurrent = 0;

    unsigned slen = t->text_length;
    GLushort * els = t->elementBuffer;
    GLfloat * vs = t->vertexBuffer;

    float tw = t->fontdef->tex.w;
    float th = t->fontdef->tex.h;
    float invtw = 1.0f / tw;
    float invth = 1.0f / th;

    const float scale = t->pt / (double) t->fontdef->size;

    for (unsigned i = 0; i < slen; i++) {

        vs = t->vertexBuffer + 16 * i;
        els = t->elementBuffer + 6 * i;

        char c = t->text[i];
        FontCharDef * fcd = &t->fontdef->chars[c];

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

        els[0] = 4 * i + 0;
        els[1] = 4 * i + 1;
        els[2] = 4 * i + 2;
        els[3] = 4 * i + 1;
        els[4] = 4 * i + 3;
        els[5] = 4 * i + 2;

        if (c == '\n') {
            ycurrent += t->fontdef->lineHeight * scale;
            xcurrent = 0;
        } else {
            xcurrent += fcd->xadvance * scale;
        }
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

    // Count the number of lines
    int lines = 1;
    for (int i = 0; i < slen - 1; i++) {
        if (text[i] == '\n') lines++;
    }
    t->line_count = lines;

    size_t vbuf_size = sizeof(GLfloat) * 16 * slen;
    size_t lbuf_size = sizeof(float) * lines;
    size_t ebuf_size = sizeof(GLushort) * 6 * slen;

    void * ptr = malloc(vbuf_size + lbuf_size + ebuf_size + slen + 1);
    t->vertexBuffer = (ptr);
    t->line_widths = (ptr + vbuf_size);
    t->elementBuffer = (ptr + vbuf_size + lbuf_size);
    t->text = (ptr + vbuf_size + lbuf_size + ebuf_size);
    memcpy(t->text, text, slen + 1);

    t->flags = 0;
    t->fontdef = fd;
    t->text_capacity = slen;
    t->text_length = slen;

    // Calc line widths
    do {
        float scale = pt / (double) fd->size;
        int current_line = 0;
        float current_length = 0.0f;
        for (int i = 0; i < slen; i++) {
            char c = text[i];
            if (c == '\n') {
                t->line_widths[current_line++] = current_length * scale;
                current_length = 0.0f;
            } else if (c >= 0) {
                FontCharDef * fcd = fd->chars + c;
                current_length += fcd->xadvance;
            } else {
                uerr("Unsupported character in text.");
            }
        }
        if (current_length > 0 || current_line == 0) {
            t->line_widths[current_line++] = current_length * scale;
        }
    } while (0);

    // Set metrics
    t->pt = pt;
    t->halign = halign;
    t->valign = valign;
    t->max_width = max_width;
    t->smoothing = 1.0f / 16.0f;
    t->color[0] = 1.0f;
    t->color[1] = 1.0f;
    t->color[2] = 1.0f;
    t->color[3] = 1.0f;

    fill_buffers(t);

    text_loadbuffer(t);

    return t;
}

void text_deinit(Text * t) {

    text_unloadbuffer(t);

    free(t->vertexBuffer);
}

void text_draw(Text * t, mat4 mvp) {

    glUseProgram(text_shader_program.id);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, t->fontdef->tex.id);
    glUniform1i(shader_tex_loc, 0);
    glUniform4fv(shader_color_loc, 1, t->color);
    glUniformMatrix4fv(shader_mvp_loc, 1, GL_FALSE, mvp);
    glUniform1f(shader_smoothing_loc, t->smoothing);

    glBindVertexArray(t->VAO);
    glDrawElements(GL_TRIANGLES, t->text_length * 6, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);

}

void text_draw_range(Text * t, mat4 mvp, unsigned start, unsigned length) {

    if (start + length > t->text_length) {
        uerr("Range not renderable.");
    }

    glUseProgram(text_shader_program.id);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, t->fontdef->tex.id);
    glUniform1i(shader_tex_loc, 0);
    glUniform4fv(shader_color_loc, 1, t->color);
    glUniformMatrix4fv(shader_mvp_loc, 1, GL_FALSE, mvp);
    glUniform1f(shader_smoothing_loc, t->smoothing);

    glBindVertexArray(t->VAO);
    glDrawElements(GL_TRIANGLES, length * 6, GL_UNSIGNED_SHORT, (GLvoid *)(start * 6 * sizeof(GLushort)));
    glBindVertexArray(0);

}
