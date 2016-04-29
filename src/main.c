#include "platform.h"

int main() {

	platform_init();
	platform_mainloop(EMPTY_GAMESTATE);
	platform_deinit();

    return 0;
}
