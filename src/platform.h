#ifndef PLATFORM_HEADER
#define PLATFORM_HEADER

#include <stdio.h>
#include <stdlib.h>

void platform_init();

int platform_res2file(const char * resource, char * pathbuf, unsigned bufsize);

#endif
