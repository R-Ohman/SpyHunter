#define _USE_MATH_DEFINES
#include "SpyHunter.h"
#include "appearance.h"

#define SECONDS_BETWEEN_REFRESH 0.5
#define REFRESH_RATE 1/SECONDS_BETWEEN_REFRESH


int main(int argc, char** argv) {
	srand(time(0));
	vector_t bestResults;
	init(&bestResults);
	SDL sdl = { NULL };
	Game game = { NULL };
	CarInfo cars[5];
	// Initialize the game
	if (initGame(&sdl)) return 1;
	LoadResults(&bestResults);
	for (int i = 0; i < ENEMIES; i++)
		cars[i].colorIndex = 0;
	
	int quit = 0;
	welcomeMenu(&sdl, &bestResults, &game, cars, &quit);
	int timeStart = SDL_GetTicks(), timeEnd, roadMarkingPos = SCREEN_HEIGHT;
	while (!quit) {
		timeEnd = SDL_GetTicks();
		// Avoid adding time if the timer has not been updated for a some unexpected time (the cycle was paused)
		if (timeEnd - timeStart < 100) game.time.delta = (timeEnd - timeStart) * 0.001;
		timeStart = timeEnd;
		
		DrawDest(&game, &sdl, &roadMarkingPos);
		movePlayerCar(&game, cars);
		DrawBullet(cars, &game, &sdl);
		DrawRandomPower(cars, &game, &sdl);
		DrawRandomCar(cars, &game, &sdl);
		
		if (DrawPlayer(&game, &sdl)) {
			// The player has no lives left
			AddResult(&game, &bestResults, &sdl);
			welcomeMenu(&sdl, &bestResults, &game, cars, &quit);
		}
		DrawInterface(game, &sdl);
		RenderSurfaces(&sdl);

		// handling of events (if there were any)
		getEvent(&game, cars, &sdl, &quit, &timeStart);
	};

	SaveResults(&bestResults);
	free(bestResults.ptr);
	FreeSurfaces(&sdl);
	return 0;
};
