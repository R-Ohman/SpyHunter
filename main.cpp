#define _USE_MATH_DEFINES
#include "SpyHunter.h"
#define SECONDS_BETWEEN_REFRESH 0.5
#define REFRESH_RATE 1/SECONDS_BETWEEN_REFRESH

/*
ПЛАН НА 10.01
- пересмотреть функции, убрать все ненужное
- доделать сохранение игры в файл (считывание)

БАГИ:
- сохранение после загрузки сохраненки

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
	char savedGames[10][20];
	for (int i = 0; i < 10; i++)
		savedGames[i][0] = '\0';
	if (initGame(&sdl)) return 1;
	// initialize the game
	NewGame(&game, cars, &sdl);
	for (int i = 0; i < ENEMIES; i++) {
		cars[i].car = sdl.cars[0];
		cars[i].colorIndex = 0;
	}
	int timeStart = game.time.startGame, timeEnd, quit = 0, frames = 0, roadMarkingPos = SCREEN_HEIGHT - 100;
	double fpsTimer = 0, fps = 0;

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
				NewGame(&game, cars, &sdl);
				game.time.deadMessage = 2;
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
		while (SDL_PollEvent(&sdl.event)) {
			switch (sdl.event.type) {
			case SDL_KEYDOWN:
				if (sdl.event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
				else if (sdl.event.key.keysym.sym == SDLK_n) NewGame(&game, cars, &sdl);
				else if (sdl.event.key.keysym.sym == SDLK_p) game.pause = !game.pause;
				else if (sdl.event.key.keysym.sym == SDLK_s)  SaveGame(&game, cars, &sdl);
				else if (sdl.event.key.keysym.sym == SDLK_l) {
					ShowSavedGames(&game, cars, &sdl);
					printf("Player pos: %d %d\n", game.player.coord.x, game.player.coord.y);
					timeStart = SDL_GetTicks();
				}
				else if (sdl.event.key.keysym.sym == SDLK_UP) game.player.speed = -1;
				else if (sdl.event.key.keysym.sym == SDLK_DOWN) game.player.speed = 1;
				else if (sdl.event.key.keysym.sym == SDLK_LEFT) game.player.turn = -1;
				else if (sdl.event.key.keysym.sym == SDLK_RIGHT) game.player.turn = 1;
				else if (sdl.event.key.keysym.sym == SDLK_SPACE) addBullet(&game, &sdl);
				break;
			case SDL_KEYUP:
				if (sdl.event.key.keysym.sym == SDLK_UP) game.player.speed = 0;
				else if (sdl.event.key.keysym.sym == SDLK_DOWN) game.player.speed = 0;
				else if (sdl.event.key.keysym.sym == SDLK_LEFT) game.player.turn = 0;
				else if (sdl.event.key.keysym.sym == SDLK_RIGHT) game.player.turn = 0;
				break;
			case SDL_QUIT:
				quit = 1;
				break;
			};
		};
		frames++;
	};

	// freeing all surfaces
	FreeSurfaces(sdl);
	return 0;
};
