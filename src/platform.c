#include "platform.h"
#include "util.h"

// Mac Platform
#ifdef __APPLE__

#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CFData.h>

static CFBundleRef mainBundle;

void platform_init() {
    mainBundle = CFBundleGetMainBundle();
}

int platform_res2file(const char * resource, char * pathbuf, unsigned bufsize) {
    CFURLRef url = CFBundleCopyResourceURL(
        mainBundle,
        CFStringCreateWithCString(
            NULL,
            resource,
            kCFStringEncodingASCII),
        NULL,
        NULL
    );

    if (!url)
        return 0;

    if(!CFURLGetFileSystemRepresentation(url, 1, (unsigned char *) pathbuf, bufsize))
        return 0;

    return 1;
}

#endif
