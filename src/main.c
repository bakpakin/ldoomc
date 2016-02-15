#include "ldoom.h"

int main() {
	platform_init();
	platform_mainloop(&teststate);
	platform_deinit();
    return 0;
}
