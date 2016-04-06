#ifndef LOGGING_HEADER
#define LOGGING_HEADER

#include <stdarg.h>
#include <stdio.h>

typedef void (*LogFn)(void * user, const char * message);
typedef void (*LogClearFn)(void * user);

extern char ldlog_logging_enabled;

typedef struct {
    LogFn log;
    LogClearFn clear;
    void * user;
    unsigned char enabled;
} Logger;

void ldlog_init();

void ldlog_deinit();

int ldlog_stdout_get();
void ldlog_stdout_set(int enabled);

void ldlog(const char * message, ...);

void ldlog_write(const char * piece);

void ldlog_flush();

void ldlog_clear();

void ldlog_logger(Logger * l);

Logger * ldlog_filelogger(FILE * fp);

#endif
