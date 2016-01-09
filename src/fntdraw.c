#include "fntdraw.h"

#include <stdlib.h>
#include "shader.h"
#include "mesh.h"
#include "util.h"

static const char * line_find_value(const char * str, const char * key, char * buffer, unsigned buf_max_len) {
	const char * c = str;
	size_t len = strlen(key);
	int keyfound = 0;
	for(;;) {
		keyfound = 1;
		for (int i = 0; i < len; i++) {
			if (c[i] != key[i])	{
				keyfound = 0;
				continue;
			}
		}
		if (keyfound) {
			if (*(c + len) == '=') {
				c += len + 1;
				break;
			}
		}
		while (*c++ != ' ' && *c != '\t' && *c != '\n')
			;
	}
	const char * end = c;
	while (*++end != ' ' && *end != '\t' && *end != '\n')
		;
	end--;
	if ((*c == '"') && (*end == '"')) {
		c++;
		end--;
	}
	unsigned length = end - c;
	if (length >= buf_max_len) {
		uerr("String buffer overflow.");
	}
	strncpy(buffer, c, length);
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
	while (*--c != sep && c != path)
		;
	if (c == path)
        uerr("Could not find file from path.");
    int filelen = strlen(file);
    if (filelen + pathlen + 1 > max_len)
        uerr("Buffer overflow.");

    strncpy(buf, path, c - path);
    strncpy(buf + pathlen, file, filelen + 1);
}

static int shader_tex_loc;
static int shader_mvp_loc;
static int shader_color_loc;

static unsigned fntdef_count = 0;
static Program text_shader_program;

static void text_shader_init() {
    program_init_file(&text_shader_program, "../resources/fontdraw.glsl");
    shader_tex_loc = program_find_uniform(&text_shader_program, "tex");
    shader_mvp_loc = program_find_uniform(&text_shader_program, "mvp");
    shader_color_loc = program_find_uniform(&text_shader_program, "color");
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
	printf("%d\n", fd->size);
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

		cd[id] = (FontCharDef) {
			x,
			y,
			w,
			h,
			xoff,
			yoff,
			xadvance
		};

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

    unsigned slen = t->text_length;
    GLushort * els = t->elementBuffer;
    GLfloat * vs = t->vertexBuffer;

    float tw = t->fontdef->tex.w;
    float th = t->fontdef->tex.h;
    float invtw = 1 / tw;
    float invth = 1 / th;

    for (unsigned i = 0; i < slen; i++, vs += 16, els += 6) {

        FontCharDef * fcd = &t->fontdef->chars[t->text[i]];

        vs[0] = xcurrent; vs[1] = 0; vs[2] = fcd->x * invtw; vs[3] = fcd->y * invth;
        vs[4] = xcurrent; vs[5] = fcd->h; vs[6] = fcd->x * invtw; vs[7] = fcd->y * invth + fcd->h * invth;
        vs[8] = xcurrent + fcd->w; vs[9] = 0; vs[10] = fcd->x * invtw + fcd->w * invtw; vs[11] = fcd->y * invth;
        vs[12] = xcurrent + fcd->w; vs[13] = fcd->h; vs[14] = fcd->x * invtw + fcd->w * invtw; vs[15] = fcd->y * invth + fcd->h * invth;

        els[0] = i;
        els[1] = els[3] = i + 1;
        els[2] = els[4] = i + 2;
        els[5] = i + 3;

        xcurrent += fcd->xadvance;
    }
}

void text_loadbuffer(Text * t) {

    size_t slen = t->text_length;

    glGenBuffers(1, &t->VBO);
    glGenBuffers(1, &t->EBO);
    glGenVertexArrays(1, &t->VAO);

    //TODO
}

void text_unloadbuffer(Text * t) {
    //TODO
}

Text * text_init(Text * t, const FontDef * fd, const char * text) {

    size_t slen = strlen(text);

    t->fontdef = fd;
    t->text = malloc(slen + 1);
    strcpy(t->text, text);
    t->vertexBuffer = malloc(sizeof(GLfloat) * 16 * slen);
    t->elementBuffer = malloc(sizeof(GLushort) * 6 * slen);
    t->text_capacity = slen;
    t->text_length = slen;

    fill_buffers(t);

    glGenBuffers(1, &t->VBO);
    glGenBuffers(1, &t->EBO);
    glGenVertexArrays(1, &t->VAO);

    glBindVertexArray(t->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, t->VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, t->EBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLushort) * 4 * slen, t->vertexBuffer, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 4 * slen, t->elementBuffer, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GL_FLOAT), 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GL_FLOAT), (GLvoid *)(2 * sizeof(GL_FLOAT)));

    glBindVertexArray(0);

    t->text_length = 0;
    t->text_capacity = 0;

    text_append(t, text);

    return t;
}

void text_deinit(Text * t) {

}

void text_draw(Text * t, mat4 mvp, vec4 color) {

    glUseProgram(text_shader_program.id);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, t->fontdef->tex.id);
    glUniform1i(shader_tex_loc, 0);
    glUniform4f(shader_color_loc, color[0], color[1], color[2], color[3]);
    glUniformMatrix4fv(shader_mvp_loc, 1, GL_FALSE, mvp);

    glBindVertexArray(t->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, t->VBO);
    glDrawElements(GL_TRIANGLES, t->text_length * 6, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);

}
