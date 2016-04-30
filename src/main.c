#include "platform.h"

int main() {

	platform_init();
	platform_mainloop();
	platform_deinit();

    return 0;
}
