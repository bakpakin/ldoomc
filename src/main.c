#include "ldoom.h"

int main() {

	platform_init();
	platform_mainloop(&arenastate);
	platform_deinit();
    return 0;
}
