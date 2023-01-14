#define _USE_MATH_DEFINES
#include "SpyHunter.h"
#include "appearance.h"
#define SECONDS_BETWEEN_REFRESH 0.5
#define REFRESH_RATE 1/SECONDS_BETWEEN_REFRESH

/*
ПЛАН НА 15.01
- пересмотреть функции, упростить их
- дописать комментарии
- найти ошибка и исправить их
- сделать уведомление когда человек сохраняет файл во время игры
- найти уязвимости в памяти, оптимизировать игру (заменить рисование прямоугольника на встроенную функцию)

Пофиксить:
- при паузе пуля продолжает лететь и может убить авто


Проверить:
- спавн PowerUp'ов / авто
- автомобили трутся друг на друга
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
	int quit = 0;
	welcomeMenu(&sdl, &bestResults, &game, cars, &quit);
	double fpsTimer = 0, fps = 0;
	int timeStart = SDL_GetTicks(), timeEnd, frames = 0, roadMarkingPos = SCREEN_HEIGHT - 100;
	while (!quit) {
		// Fill window with green color;
		if (!game.pause) SDL_FillRect(sdl.screen, NULL, SDL_MapRGB(sdl.screen->format, 107, 142, 35));
		DrawDest(&game, &sdl, &roadMarkingPos);
		
		timeEnd = SDL_GetTicks();
		game.time.delta = (timeEnd - timeStart) * 0.001;
		timeStart = timeEnd;
		if (!game.pause) {
			changeTimers(&game);
			game.totalDistance += game.time.delta - game.player.speed * game.time.delta;
			// WARN - моэно поменять; дефолтно скорость 0, ибо авто не едет

			game.player.coord.y += (game.player.speed > 0 ? 400 : 100) * game.time.delta * game.player.speed;
			fixCoordY(&game.player.coord.y);
			game.player.coord.x += game.player.turn * game.time.delta * CAR_SPEED * (game.player.powerTime[1] > 0 ? 1.8 : 1.2);
			
			// ADD SCORE
			if (onTheRoad(&game.player.coord.x, &game) && !game.time.scoreFreeze)
				game.score += (game.player.speed < 0 ? 50 : 0 + 50) * game.time.delta * (game.player.powerTime[1] > 0 ? 3 : 1);
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
