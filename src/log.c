#include "log.h"
#include <stdio.h>
#include "vector.h"

VECGEN(Logger *, logger);

char ldlog_logging_enabled = 1;
static Vector loggers;
static Logger stdoutlogger;
static char * logbuffer;
static size_t logbuffer_size;
static size_t logbuffer_capacity;

static void filelogger_log(void * user, const char * message, va_list args) {
    FILE * fp = (FILE *) user;
    vfprintf(fp, message, args);
}

void ldlog_init() {
    vector_init_logger(&loggers, 4);
    stdoutlogger.log = filelogger_log;
    stdoutlogger.clear = NULL;
    stdoutlogger.user = stdout;
    stdoutlogger.enabled = 1;
    ldlog_logger(&stdoutlogger);
    logbuffer = malloc(100);
    logbuffer_size = 0;
    logbuffer_capacity = 100;
}

void ldlog_deinit() {
    vector_deinit_logger(&loggers);
}

void ldlog(const char * message, ...) {
    if (ldlog_logging_enabled) {
        va_list l;
        va_start(l, message);

        for (unsigned i = 0; i < loggers.count; i++) {
            Logger * lgr = vector_get_logger(&loggers, i);
            if (!lgr->enabled) continue;
            lgr->log(lgr->user, message, l);
        }

        va_end(l);
    }
}

void ldlog_clear() {
    if (ldlog_logging_enabled) {
        for (unsigned i = 0; i < loggers.count; i++) {
            Logger * lgr = vector_get_logger(&loggers, i);
            if ((!lgr->enabled) || (!lgr->clear)) continue;
            lgr->clear(lgr->user);
        }
    }
}

void ldlog_logger(Logger * l) {
    vector_push_logger(&loggers, l);
}

Logger * ldlog_filelogger(FILE * fp) {
    Logger * l = malloc(sizeof(Logger));
    l->clear = NULL;
    l->user = fp;
    l->log = filelogger_log;
    l->enabled = 1;
    ldlog_logger(l);
    return l;
}

int ldlog_stdout_get() {
    return stdoutlogger.enabled;
}

void ldlog_stdout_set(int enabled) {
    stdoutlogger.enabled = enabled;
}

static void logbuffer_push(const char * str, size_t len) {
    if (logbuffer_size + len + 1 >= logbuffer_capacity) {
        logbuffer = realloc(logbuffer, ((logbuffer_size + len) * 1.5) + 2);
    }
    if (logbuffer_size > 0) {
        logbuffer[logbuffer_size++] = '\t';
    }
    strncpy(logbuffer + logbuffer_size, str, len);
    logbuffer_size += len;
}

void ldlog_write(const char * piece) {
    logbuffer_push(piece, strlen(piece));
}

void ldlog_flush() {
    logbuffer[logbuffer_size] = 0;
    ldlog(logbuffer);
    logbuffer_size = 0;
}
