#include "ldoom.h"

int main(int argc, char* argv[]) {
	platform_init();
	platform_mainloop(&teststate);
	platform_deinit();
    return 0;
}
