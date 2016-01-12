#include "ldoom.h"

int main(int argc, char* argv[]) {
	game_init();
	game_mainloop(&arenastate);
    return 0;
}
