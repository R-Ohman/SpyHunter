#define _USE_MATH_DEFINES
#include "SpyHunter.h"
#define SECONDS_BETWEEN_REFRESH 0.5
#define REFRESH_RATE 1/SECONDS_BETWEEN_REFRESH

/*
ПЛАН НА 10.01
- пересмотреть функции, убрать все ненужное

БАГИ:

Изменить:


Недочеты:
- при паузе пуля продолжает лететь и может убить авто


Проверить:
- пофиксить спавн PowerUp'а
- при сбитии вражеского авто, которое уперлось (снизу) в другое авто, оно отодвигается не в ту сторону
- когда 2 атакующих авто прилегают друг к другу, они не атакуют
*/


int main(int argc, char** argv) {
	vector_t bestResults;
	init(&bestResults);
	SDL sdl = { NULL };
	Game game = { NULL };
	CarInfo cars[5];
	if (initGame(&sdl)) return 1;
	// initialize the game
	LoadResults(&bestResults);
	for (int i = 0; i < ENEMIES; i++) {
		cars[i].car = sdl.cars[0];
		cars[i].colorIndex = 0;
	}
	int quit = 0, frames = 0, roadMarkingPos = SCREEN_HEIGHT - 100;
	double fpsTimer = 0, fps = 0;
	welcomeMenu(&sdl, &bestResults, &game, cars, &quit);
	int timeStart = game.time.startGame, timeEnd;
	while (!quit) {
		// Fill window with green color;
		if (!game.pause) SDL_FillRect(sdl.screen, NULL, SDL_MapRGB(sdl.screen->format, 107, 142, 35));
		DrawDest(&game, &sdl, &roadMarkingPos);
		// info text
		
		timeEnd = SDL_GetTicks();
		game.time.delta = (timeEnd - timeStart) * 0.001;
		timeStart = timeEnd;
		if (!game.pause) {
			
			changeTimers(&game);
			game.totalDistance += game.time.delta - game.player.speed * game.time.delta;
			// WARN - моэно поменять; дефолтно скорость 0, ибо авто не едет

			game.player.coord.y += (game.player.speed > 0 ? 400 : 100) * game.time.delta * game.player.speed;
			fixCoordY(&game.player.coord.y);
			game.player.coord.x += game.player.turn * game.time.delta * 300;
			
			// ADD SCORE
			if (onTheRoad(&game.player.coord.x, &game) && !game.time.scoreFreeze)
				game.score += (game.player.speed < 0 ? 50 : 0 + 50) * game.time.delta;
		}
			DrawBullet(cars, &game, &sdl);
			DrawRandomPower(cars, &game, &sdl);
			DrawRandomCar(cars, &game, &sdl);
			// draw player
			if (DrawPlayer(&game, &sdl)) {
				AddResult(&game, &bestResults, &sdl);
				welcomeMenu(&sdl, &bestResults, &game, cars, &quit);
			}
		//DrawSurface(sdl.screen, game.player.player, game.player.coord.x, game.player.coord.y);
		DrawHeader(sdl.screen, game, sdl, fps);

		fpsTimer += game.time.delta;
		if (fpsTimer > SECONDS_BETWEEN_REFRESH) {
			fps = frames * REFRESH_RATE;
			frames = 0;
			fpsTimer -= SECONDS_BETWEEN_REFRESH;
		};
		
		RenderSurfaces(&sdl);
		
		// handling of events (if there were any)
		getEvent(&game, cars, &sdl, &quit, &timeStart);
		frames++;
	};

	SaveResults(&bestResults);
	// freeing all surfaces
	FreeSurfaces(sdl);
	return 0;
};
