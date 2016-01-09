#ifndef UTIL_HEADER__
#define UTIL_HEADER__

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define uerr(msg) do { \
    fprintf(stderr, "Error in %s, line %d, %s: %s\n", __FILE__, __LINE__, __func__, msg); \
} while (0)

#define uerr2(msg, args...) do { \
    fprintf(stderr, "Error in %s, line %d, %s: " #msg "\n", __FILE__, __LINE__, __func__, __VA_ARGS__); \
} while (0)

#define uassert(expr) do { \
    if (!(expr)) \
        uerr("Assertion failed."); \
} while (0)

const char * util_filename_ext(const char * path);

// Slurped data musted be freed via free.
char * util_slurp(const char * path, long * length);

void util_spit(const char * path, const char * data, long length);

#endif
