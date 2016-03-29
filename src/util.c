#include "util.h"
#include "ldmath.h"

const char * util_filename_ext(const char * path) {
    const char * dot = strrchr(path, '.');
    if(!dot || dot == path) return "";
    return dot + 1;
}

// Slurped data musted be freed via free.
char * util_slurp(const char * path, long * length) {
    FILE * fp = fopen(path, "rb");
    if (!fp) {
        uerr("Could not open file.");
    }
    fseek(fp, 0L, SEEK_END);
    long fsize = ftell(fp);
    char * data = malloc(fsize + 1);
    fseek(fp, 0L, SEEK_SET);
    if (!fread(data, fsize, 1, fp)) {
        uerr("Could not read file.");
    }
    // Null terminate the string. This is an extra precaution
    // to prevent string errors down the line. Returned data is binary, though,
    // so there could be embeded 0s in the data.
    data[fsize] = 0;
    fclose(fp);
    if (length)
        *length = fsize;
    return data;
}

static char * staticbuffer = NULL;
static size_t staticbuffer_size = 0;
static int buffer_in_use = 0;

// Allocates some temprary data. Only one chunk of data at a time can be allocated like this.
void * uqmalloc(size_t size) {
    if (buffer_in_use)
        uerr("uqfree must be called before uqmalloc can be called again.");
    if (staticbuffer_size == 0) {
        staticbuffer = malloc(size);
        staticbuffer_size = size;
    } else if (size > staticbuffer_size)
        staticbuffer = realloc(staticbuffer, size);
        staticbuffer_size = size;
    buffer_in_use = 1;
    return staticbuffer;
}

// realloc the memory from uqmalloc.
void * uqrealloc(size_t size) {
    if (size > staticbuffer_size)
        staticbuffer = realloc(staticbuffer, size);
    return staticbuffer;
}

// Frees data allocated with uqmalloc.
void uqfree(void * ptr) {
    if (ptr != staticbuffer)
        uerr("Trying to free memory not allocated with uqmalloc.");
    if (!buffer_in_use)
        uerr("Trying to free memory not in use.");
    buffer_in_use = 0;
}

// Debug printing

void mat4_print(mat4 m) {
    for (int i = 0; i < 4; i++, m += 4)
        printf("%f, %f, %f, %f\n", m[0], m[1], m[2], m[3]);
}
