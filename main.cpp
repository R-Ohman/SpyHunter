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
- загружа файл не в том формате - предлагает сохранить результат


Проверить:
- спавн PowerUp'ов / авто
- автомобили трутся друг на друга
*/


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
	double fpsTimer = 0, fps = 0;
	int timeStart = SDL_GetTicks(), timeEnd, frames = 0, roadMarkingPos = SCREEN_HEIGHT - 100;
	while (!quit) {
		// Fill window with green color;
		if (!game.pause) SDL_FillRect(sdl.screen, NULL, SDL_MapRGB(sdl.screen->format, 107, 142, 35));
		DrawDest(&game, &sdl, &roadMarkingPos);

		timeEnd = SDL_GetTicks();
		// чтобы в случае открытого меню не начислялось время
		if (timeEnd - timeStart < 100) game.time.delta = (timeEnd - timeStart) * 0.001;
		timeStart = timeEnd;
		
		movePlayerCar(&game);
		DrawBullet(cars, &game, &sdl);
		DrawRandomPower(cars, &game, &sdl);
		DrawRandomCar(cars, &game, &sdl);
		if (DrawPlayer(&game, &sdl)) {
			AddResult(&game, &bestResults, &sdl);
			welcomeMenu(&sdl, &bestResults, &game, cars, &quit);
		}
		DrawInterface(game, &sdl, fps);

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
	free(bestResults.ptr);
	return 0;
};
