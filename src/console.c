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
static Logger logger;

static Text fpsText;

static char console_visible = 1;

static inline float textpad(Text * t) {
    return t->line_count * (console_fd.lineHeight / console_fd.size * t->pt + CONSOLE_PADDING);
}

static inline Text * nth_history(unsigned n) {
    return history + (history_start + n) % history_capacity;
}

static void history_put(const char * msg) {
    char pred[80];

    time_t now = time(0);
    struct tm * info = localtime(&now);
    strftime(pred, 80, "$@F00%T:$@FFF ", info);

    Text * next = nth_history(history_len);
    if (history_len == history_capacity) {
        history_start = (history_start + 1) % history_capacity;
        history_ysize -= textpad(next);
        text_set_multi(next, 2, pred, msg);
        history_ysize += textpad(next);
    } else {
        history_len++;
        text_init_multi(next, &console_text_options, 2, pred, msg);
        history_ysize += textpad(next);
    }
}

static void console_log_fn(void * user, const char * message) {
    history_put(message);
}

static void console_log_clear(void * user) {
    for (unsigned i = 0; i < history_len; i++) {
        Text * t = nth_history(i);
        text_deinit(t);
    }
    history_ysize = 0;
}

void console_draw() {

    static float old_fps = 0;

    if (!console_visible) return;

    if (old_fps != platform_fps()) {
        old_fps = platform_fps();
        text_format(&fpsText, 25, "fps: %.0f", old_fps);
    }

    fpsText.position[0] = platform_width() - 505;
    text_draw(&fpsText, platform_screen_matrix());

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
        text_draw(t, platform_screen_matrix());
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

    logger.log = console_log_fn,
    logger.clear = console_log_clear,
    logger.user = NULL,
    logger.enabled = 1;

    ldlog_logger(&logger);

    console_text_options.halign = ALIGN_RIGHT;
    text_init(&fpsText, &console_text_options, "fps:     ");
    console_text_options.halign = ALIGN_LEFT;
    fpsText.position[1] = 5.0f;
    fpsText.position[0] = platform_width() - 505;
}

void console_deinit() {

    fnt_deinit(&console_fd);
    for(unsigned i = 0; i < history_len; i++) {
        Text * t = history + ((history_start + i) % history_capacity);
        text_deinit(t);
    }
    text_deinit(&fpsText);
    free(history);

}
