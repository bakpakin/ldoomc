#include "log.h"
#include <stdio.h>
#include "vector.h"

VECGEN(Logger *, logger);

char ldlog_logging_enabled = 1;
static Vector loggers;
static Logger stdoutlogger;

static void filelogger_log(void * user, const char * message, va_list args);

void ldlog_init() {
    vector_init_logger(&loggers, 4);
    stdoutlogger.log = filelogger_log;
    stdoutlogger.clear = NULL;
    stdoutlogger.user = NULL;
    stdoutlogger.enabled = 1;
    ldlog_logger(&stdoutlogger);
}

void ldlog_deinit() {
    vector_deinit_logger(&loggers);
}

void ldlog(const char * message, ...) {
    if (ldlog_logging_enabled) {
        va_list l;
        va_start(l, message);

        for (int i = 0; i < loggers.count; i++) {
            Logger * lgr = vector_get_logger(&loggers, i);
            if (!lgr->enabled) continue;
            lgr->log(lgr->user, message, l);
        }

        va_end(l);
    }
}

void ldlog_clear() {
    if (ldlog_logging_enabled) {
        for (int i = 0; i < loggers.count; i++) {
            Logger * lgr = vector_get_logger(&loggers, i);
            if (!lgr->enabled || !lgr->clear) continue;
            lgr->clear(lgr->user);
        }
    }
}

void ldlog_logger(Logger * l) {
    vector_push_logger(&loggers, l);
}

static void filelogger_log(void * user, const char * message, va_list args) {
    FILE * fp = (FILE *) user;
    vfprintf(fp, message, args);
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
