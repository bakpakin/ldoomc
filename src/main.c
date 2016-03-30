#include "ldoom.h"

int main() {

	platform_init();
	platform_mainloop(&menustate);
	platform_deinit();
    return 0;
}
