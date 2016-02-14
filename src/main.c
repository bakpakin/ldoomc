#include "ldoom.h"

int main(int argc, char* argv[]) {
	platform_init();
	platform_mainloop(&arenastate);
	platform_deinit();
    return 0;
}
