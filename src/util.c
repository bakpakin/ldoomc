#include "util.h"

const char * util_filename_ext(const char * path) {
    const char * dot = strrchr(path, '.');
    if(!dot || dot == path) return "";
    return dot + 1;
}

// Slurped data musted be freed via free.
char * util_slurp(const char * path, long * length) {
    FILE * fp = fopen(path, "rb");
    fseek(fp, 0L, SEEK_END);
    long fsize = ftell(fp);
    char * data = malloc(fsize + 1);
    fseek(fp, 0L, SEEK_SET);
    fread(data, fsize, 1, fp);
    // Null terminate the string. This is an extra precaution
    // to prevent string errors down the line. Returned data is binary, though,
    // so there could be embeded 0s in the data.
    data[fsize] = 0;
    fclose(fp);
    if (length)
        *length = fsize;
    return data;
}
