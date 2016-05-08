#include "console.h"
#include "platform.h"
#include "fntdraw.h"
#include "ldmath.h"
#include "quickdraw.h"
#include "util.h"
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

static struct {
    FontDef fd;
    TextOptions text_options;
    Text * history;
    size_t history_len;
    size_t history_start;
    size_t history_capacity;
    float history_ysize;
    char * write_buffer;
    size_t write_buffer_capacity;
    size_t write_buffer_len;
    char visible;
} console_globals;

static inline float textpad(Text * t) {
    return t->line_count * (console_globals.fd.lineHeight / console_globals.fd.size * t->pt + CONSOLE_PADDING);
}

static inline Text * nth_history(unsigned n) {
    return console_globals.history + (console_globals.history_start + n) % console_globals.history_capacity;
}

void console_lograw(const char * message) {
    Text * next = nth_history(console_globals.history_len);
    if (console_globals.history_len == console_globals.history_capacity) {
        console_globals.history_start = (console_globals.history_start + 1) % console_globals.history_capacity;
        float old_tpad = textpad(next);
        text_set(next, message);
        console_globals.history_ysize += textpad(next) - old_tpad;
    } else {
        console_globals.history_len++;
        text_init(next, &console_globals.text_options, message);
        console_globals.history_ysize += textpad(next);
    }
    if (next->line_count == 0) {
        text_set(next, " ");
        console_globals.history_ysize += textpad(next);
    }
}

void console_log(const char * format, ...) {
    va_list list;
    va_start(list, format);
    int neededlen = vsnprintf(NULL, 0, format, list);
    if (neededlen < 0) return;
    uqfree_if_needed();
    char * buffer = uqmalloc(neededlen + 1);
    va_end(list);
    va_start(list, format);
    vsnprintf(buffer, neededlen + 1, format, list);
    console_lograw(buffer);
    va_end(list);
    uqfree_if_needed();
}

void console_clear() {
    for (unsigned i = 0; i < console_globals.history_len; i++) {
        Text * t = nth_history(i);
        text_deinit(t);
    }
    console_globals.history_ysize = 0;
}

void console_draw() {

    if (!console_globals.visible) return;

    // Get out of the way if there is nothing to draw.
    if (console_globals.history_len == 0) return;

    qd_rgba(0.2f, 0.2f, 0.2f, 0.8f);
    qd_rect(
            -1,
            -1,
            CONSOLE_FONT_WIDTH + CONSOLE_BORDER_LEFT + CONSOLE_BORDER_RIGHT + 1,
            console_globals.history_ysize + CONSOLE_BORDER_TOP + CONSOLE_BORDER_BOTTOM + 1,
            QD_FILL);
    qd_rgba(1, 1, 1, 1);

    float y = CONSOLE_BORDER_TOP;
    for (unsigned i = 0; i < console_globals.history_len; i++) {
        Text * t = nth_history(i);
        t->position[0] = CONSOLE_BORDER_LEFT;
        t->position[1] = y;
        text_draw_screen(t);
        y += textpad(t);
    }

}

void console_set_history(unsigned length) {

    if (length < console_globals.history_len) { // Delete the oldest text logs
        for (unsigned i = length; i < console_globals.history_len; i++) {
            Text * t = nth_history(i);
            console_globals.history_ysize -= textpad(t);
            text_deinit(t);
        }
        console_globals.history_len = length;
    }
    Text * history_tmp = malloc(sizeof(Text) * length);
    for (unsigned i = 0; i < console_globals.history_len; i++) {
        memcpy(history_tmp + i, nth_history(i), sizeof(Text));
    }
    console_globals.history = history_tmp;
    console_globals.history_start = 0;

    console_globals.history_capacity = length;
}

int console_get_history() {
    return console_globals.history_capacity;
}

int console_get_visible() {
    return console_globals.visible;
}

void console_set_visible(int visible) {
    console_globals.visible = visible;
}

void console_push(const char * string) {
    console_pushn(string, strlen(string));
}

void console_pushn(const char * string, size_t n) {
    size_t oldlen = console_globals.write_buffer_len;
    console_globals.write_buffer_len = oldlen + n;
    if (console_globals.write_buffer_len > console_globals.write_buffer_capacity) {
        console_globals.write_buffer_capacity = console_globals.write_buffer_len * 1.5 + 1;
        console_globals.write_buffer = realloc(console_globals.write_buffer, console_globals.write_buffer_capacity + 1);
    }
    strncpy(console_globals.write_buffer + oldlen, string, n);
}

void console_flush(int useMarkup) {
    if (console_globals.write_buffer_len == 0) return;
    console_globals.write_buffer[console_globals.write_buffer_len] = '\0';
    if (!useMarkup) {
        size_t newlen = textutil_get_escapedlength(console_globals.write_buffer);
        if (newlen > console_globals.write_buffer_capacity) {
            console_globals.write_buffer_capacity = newlen;
            console_globals.write_buffer = realloc(console_globals.write_buffer, newlen + 1);
        }
        textutil_escape_inplace(console_globals.write_buffer, newlen + 1);
    }
    console_lograw(console_globals.write_buffer);
    console_globals.write_buffer_len = 0;
}

void console_clearflush() {
    console_globals.write_buffer_len = 0;
}

// INITIALIZATION / DEINITIALIZATION

void console_init() {

    // Set globals
    console_globals.visible = 1;

    fnt_init(&console_globals.fd, CONSOLE_FONT_NAME);
    fnt_default_options(&console_globals.fd, &console_globals.text_options);
    console_globals.text_options.width = CONSOLE_FONT_WIDTH;
    console_globals.text_options.pt = CONSOLE_FONT_POINT;
    console_globals.text_options.valign = ALIGN_TOP;
    console_globals.text_options.halign = ALIGN_LEFT;
    console_globals.text_options.threshold = CONSOLE_FONT_THRESHOLD;
    console_globals.text_options.smoothing = 1.0f / 4.0f;

    console_globals.history_capacity = 10;
    console_globals.history_len = 0;
    console_globals.history_start = 0;
    console_globals.history = malloc(sizeof(Text) * console_globals.history_capacity);

    console_globals.write_buffer_capacity = 63;
    console_globals.write_buffer_len = 0;
    console_globals.write_buffer = malloc(console_globals.write_buffer_capacity + 1);
}

void console_deinit() {
    fnt_deinit(&console_globals.fd);
    for(unsigned i = 0; i < console_globals.history_len; i++) {
        Text * t = console_globals.history + ((console_globals.history_start + i) % console_globals.history_capacity);
        text_deinit(t);
    }
    free(console_globals.history);
    free(console_globals.write_buffer);
}
