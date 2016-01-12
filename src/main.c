#include "ldoom.h"

int main(int argc, char* argv[]) {
	platform_init();
	game_init();
	game_mainloop(&arenastate);
    return 0;
}
