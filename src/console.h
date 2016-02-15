#ifndef CONSOLE_HEADER
#define CONSOLE_HEADER

void console_init();
void console_deinit();
void console_draw();
void console_set_history(unsigned length);
int console_get_history();

int console_get_visible();
void console_set_visible(int visible);

#endif
