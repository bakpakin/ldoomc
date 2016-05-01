#ifndef CONSOLE_HEADER
#define CONSOLE_HEADER

#include <stdarg.h>
#include <stdlib.h>

void console_log(const char * format, ...);

void console_pushn(const char * string, size_t n);
void console_push(const char * string);
void console_flush(int useMarkup);
void console_clearflush();

void console_init();
void console_deinit();
void console_draw();
void console_clear();
void console_set_history(unsigned length);
int console_get_history();

int console_get_visible();
void console_set_visible(int visible);

#endif
