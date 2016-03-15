#include "defines.h"
#include "ldoom.h"

int main() {
    printf("Initializing platform...\n");
	platform_init();
	printf("Beginning main loop...\n");
	platform_mainloop(&arenastate);
	printf("Deinitializing platform...\n");
	platform_deinit();
    return 0;
}
