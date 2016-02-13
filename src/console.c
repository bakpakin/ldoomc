#include "console.h"
#include "platform.h"
#include "fntdraw.h"
#include "ldmath.h"
#include "quickdraw.h"
#include "log.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// Console Rendering Definitions
#define CONSOLE_FONT_POINT 14
#define CONSOLE_FONT_WIDTH 600
#define CONSOLE_FONT_NAME "consolefont.txt"
#define CONSOLE_BORDER_TOP 4
#define CONSOLE_BORDER_BOTTOM 2
#define CONSOLE_BORDER_LEFT 5
#define CONSOLE_BORDER_RIGHT 5
#define CONSOLE_PADDING 8

static FontDef fd;

static char * charbuf;
static size_t charbuf_len;
static size_t charbuf_capacity;

static Text * history;
static size_t history_len;
static size_t history_start;
static size_t history_capacity;
static float history_ysize = 0;
static Logger logger;

static int console_visible = 1;

static inline float textpad(Text * t) {
    return t->line_count * (fd.lineHeight / CONSOLE_FONT_POINT + CONSOLE_PADDING);
}

static inline Text * nth_history(int n) {
    return history + (history_start + n) % history_capacity;
}

static void text_config(Text * t) {
    t->threshold = 0.3f;
    t->smoothing = 1.0f / 4.0f;
}

static void history_put(const char * msg) {
    char pred[80];

    time_t now = time(0);
    struct tm * info = localtime(&now);
    strftime(pred, 80, "%T: ", info);

    Text * next = nth_history(history_len);
    if (history_len == history_capacity) {
        history_start = (history_start + 1) % history_capacity;
        history_ysize -= textpad(next);
        text_set_multi(next, 2, pred, msg);
        history_ysize += textpad(next);
    } else {
        history_len++;
        text_config(text_init_multi(next, &fd, CONSOLE_FONT_POINT, ALIGN_LEFT, ALIGN_TOP, CONSOLE_FONT_WIDTH, 0, 2, pred, msg));
        history_ysize += textpad(next);
    }
}

static void console_log_fn(void * user, const char * message, va_list args) {

    int done = 0;
    while (!done) {
        int result = vsnprintf(charbuf, charbuf_capacity, message, args);
        if (result == -1) {
            charbuf_capacity *= 2;
            charbuf = realloc(charbuf, charbuf_capacity);
        } else {
            done = 1;
            charbuf_len = result;
        }
    }

    history_put(message);

}

static void console_log_clear(void * user) {
    for (int i = 0; i < history_len; i++) {
        Text * t = nth_history(i);
        text_deinit(t);
    }
    history_ysize = 0;
}

void console_draw() {

    qd_rgba(0.2f, 0.2f, 0.2f, 0.8f);
    qd_rect(
            -1,
            -1,
            CONSOLE_FONT_WIDTH + CONSOLE_BORDER_LEFT + CONSOLE_BORDER_RIGHT,
            history_ysize + CONSOLE_BORDER_TOP + CONSOLE_BORDER_BOTTOM,
            QD_TRIANGLEFAN);
    qd_rgba(1, 1, 1, 1);

    float y = CONSOLE_BORDER_TOP;
    for (int i = 0; i < history_len; i++) {
        Text * t = nth_history(i);
        t->position[0] = CONSOLE_BORDER_LEFT;
        t->position[1] = y;
        text_draw(t, platform_screen_matrix());
        y += textpad(t);
    }

}

void console_set_history(int length) {

    if (length < history_len) { // Delete the oldest text logs
        for (int i = length; i < history_len; i++) {
            Text * t = nth_history(i);
            history_ysize -= textpad(t);
            text_deinit(t);
        }
        history_len = length;
    }
    Text * history_tmp = malloc(sizeof(Text) * length);
    for (int i = 0; i < history_len; i++) {
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

void console_init() {

    fnt_init(&fd, CONSOLE_FONT_NAME);

    charbuf_capacity = 80;
    charbuf_len = 0;
    charbuf = malloc(charbuf_capacity);

    history_capacity = 35;
    history_len = 0;
    history_start = 0;
    history = malloc(sizeof(Text) * history_capacity);

    logger.log = console_log_fn,
    logger.clear = console_log_clear,
    logger.user = NULL,
    logger.enabled = 1;

    ldlog_logger(&logger);

}

void console_deinit() {

    fnt_deinit(&fd);
    for(int i = 0; i < history_len; i++) {
        Text * t = history + ((history_start + i) % history_capacity);
        text_deinit(t);
    }
    free(history);
    free(charbuf);

}
