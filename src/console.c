#include "console.h"
#include "platform.h"
#include "fntdraw.h"
#include "ldmath.h"
#include "quickdraw.h"
#include "util.h"
#include "luainterop.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// Console Rendering Definitions
#define CONSOLE_FONT_POINT 14
#define CONSOLE_FONT_WIDTH 600
#define CONSOLE_FONT_SMOOTHING 0.25f
#define CONSOLE_FONT_THRESHOLD 0.3f
#define CONSOLE_FONT_NAME "consolefont.txt"
#define CONSOLE_BORDER_TOP 4
#define CONSOLE_BORDER_BOTTOM 2
#define CONSOLE_BORDER_LEFT 5
#define CONSOLE_BORDER_RIGHT 5
#define CONSOLE_PADDING 8

static FontDef console_fd;
static TextOptions console_text_options;

static Text * history;
static size_t history_len;
static size_t history_start;
static size_t history_capacity;
static float history_ysize = 0;

static char * console_write_buffer = NULL;
static size_t console_write_buffer_capacity = 0;
static size_t console_write_buffer_len = 0;

static char console_visible = 1;

static inline float textpad(Text * t) {
    return t->line_count * (console_fd.lineHeight / console_fd.size * t->pt + CONSOLE_PADDING);
}

static inline Text * nth_history(unsigned n) {
    return history + (history_start + n) % history_capacity;
}

void console_log(const char * format, ...) {
    va_list list;
    va_start(list, format);
    console_logv(format, list);
    va_end(list);
}

void console_logv(const char * format, va_list args) {
    Text * next = nth_history(history_len);
    if (history_len == history_capacity) {
        history_start = (history_start + 1) % history_capacity;
        float old_tpad = textpad(next);
        text_set_formatv(next, format, args);
        history_ysize += textpad(next) - old_tpad;
    } else {
        history_len++;
        text_init_formatv(next, &console_text_options, format, args);
        history_ysize += textpad(next);
    }
    if (next->line_count == 0) {
        text_set(next, " ");
        history_ysize += textpad(next);
    }
}

void console_clear() {
    for (unsigned i = 0; i < history_len; i++) {
        Text * t = nth_history(i);
        text_deinit(t);
    }
    history_ysize = 0;
}

void console_draw() {

    if (!console_visible) return;

    // Get out of the way if there is nothing to draw.
    if (history_len == 0) return;

    qd_rgba(0.2f, 0.2f, 0.2f, 0.8f);
    qd_rect(
            -1,
            -1,
            CONSOLE_FONT_WIDTH + CONSOLE_BORDER_LEFT + CONSOLE_BORDER_RIGHT + 1,
            history_ysize + CONSOLE_BORDER_TOP + CONSOLE_BORDER_BOTTOM + 1,
            QD_FILL);
    qd_rgba(1, 1, 1, 1);

    float y = CONSOLE_BORDER_TOP;
    for (unsigned i = 0; i < history_len; i++) {
        Text * t = nth_history(i);
        t->position[0] = CONSOLE_BORDER_LEFT;
        t->position[1] = y;
        text_draw_screen(t);
        y += textpad(t);
    }

}

void console_set_history(unsigned length) {

    if (length < history_len) { // Delete the oldest text logs
        for (unsigned i = length; i < history_len; i++) {
            Text * t = nth_history(i);
            history_ysize -= textpad(t);
            text_deinit(t);
        }
        history_len = length;
    }
    Text * history_tmp = malloc(sizeof(Text) * length);
    for (unsigned i = 0; i < history_len; i++) {
        memcpy(history_tmp + i, nth_history(i), sizeof(Text));
    }
    history = history_tmp;
    history_start = 0;

    history_capacity = length;
}

int console_get_history() {
    return history_capacity;
}

int console_get_visible() {
    return console_visible;
}

void console_set_visible(int visible) {
    console_visible = visible;
}

void console_push(const char * string) {
    console_pushn(string, strlen(string));
}

void console_pushn(const char * string, size_t n) {
    size_t oldlen = console_write_buffer_len;
    console_write_buffer_len = oldlen + n;
    if (console_write_buffer_len > console_write_buffer_capacity) {
        console_write_buffer_capacity = console_write_buffer_len * 1.5 + 1;
        console_write_buffer = realloc(console_write_buffer, console_write_buffer_capacity + 1);
    }
    strcpy(console_write_buffer + oldlen, string);
}

void console_flush(int useMarkup) {
    if (console_write_buffer_len == 0) return;
    if (!useMarkup) {
        size_t newlen = textutil_get_escapedlength(console_write_buffer);
        if (newlen > console_write_buffer_capacity) {
            console_write_buffer_capacity = newlen;
            console_write_buffer = realloc(console_write_buffer, newlen + 1);
        }
        textutil_escape_inplace(console_write_buffer, newlen + 1);
    }
    console_log("%s", console_write_buffer);
    console_write_buffer_len = 0;
}

void console_clearflush() {
    console_write_buffer_len = 0;
}

// LUA INTEROP

int luai_console_log_impl(lua_State * L) {
    int n = lua_gettop(L);  /* number of arguments */
    int i;
    lua_getglobal(L, "tostring");
    for (i=1; i<=n; i++) {
        const char *s;
        size_t l;
        lua_pushvalue(L, -1);  /* function to be called */
        lua_pushvalue(L, i);   /* value to print */
        lua_call(L, 1, 1);
        s = lua_tolstring(L, -1, &l);  /* get result */
        if (s == NULL) {
            console_clearflush();
            return luaL_error(L,
                    LUA_QL("tostring") " must return a string to " LUA_QL("print"));
        }
        if (i>1) console_pushn("\t", 1);
        console_pushn(s, l);
        lua_pop(L, 1);  /* pop result */
    }
    return 0;
}

int luai_console_log(lua_State * L) {
    console_clearflush();
    int result = luai_console_log_impl(L);
    console_flush(0);
    return result;
}

int luai_console_logc(lua_State * L) {
    console_clearflush();
    int result = luai_console_log_impl(L);
    console_flush(1);
    return result;
}

void luai_console_loadlib() {
    const luaL_Reg module[] = {
        {"log", luai_console_log},
        {"logc", luai_console_logc},
        {NULL, NULL}
    };
    luai_addsubmodule("console", module);
}

// INITIALIZATION / DEINITIALIZATION

void console_init() {
    fnt_init(&console_fd, CONSOLE_FONT_NAME);
    fnt_default_options(&console_fd, &console_text_options);
    console_text_options.width = CONSOLE_FONT_WIDTH;
    console_text_options.pt = CONSOLE_FONT_POINT;
    console_text_options.valign = ALIGN_TOP;
    console_text_options.halign = ALIGN_LEFT;
    console_text_options.threshold = CONSOLE_FONT_THRESHOLD;
    console_text_options.smoothing = 1.0f / 4.0f;

    history_capacity = 10;
    history_len = 0;
    history_start = 0;
    history = malloc(sizeof(Text) * history_capacity);

    console_write_buffer_capacity = 63;
    console_write_buffer_len = 0;
    console_write_buffer = malloc(console_write_buffer_capacity + 1);

    luai_console_loadlib();
}

void console_deinit() {
    fnt_deinit(&console_fd);
    for(unsigned i = 0; i < history_len; i++) {
        Text * t = history + ((history_start + i) % history_capacity);
        text_deinit(t);
    }
    free(history);
    free(console_write_buffer);
}
