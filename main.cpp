#define _USE_MATH_DEFINES
#include "SpyHunter.h"
#define SECONDS_BETWEEN_REFRESH 0.5
#define REFRESH_RATE 1/SECONDS_BETWEEN_REFRESH

/*
���� �� 7.01
- ������������ �������, ������ ��� ��������
- ��������� ����� (��������� ���������� �� Y)
- �������� ��������� ��� ��������
- �������� NewGame() � ������ ����
- ��������� ������� ����� ���� ��� ���� �� ������

����:


��������:
- �������� ��������� ����

���������:
- ��� ������ ���������� ����, ������� �������� (�����) � ������ ����, ��� ������������ �� � �� �������
- ����� 2 ��������� ���� ��������� ���� � �����, ��� �� �������
*/


int main(int argc, char** argv) {
	SDL sdl = { NULL };
	Game game = { NULL };
	CarInfo cars[5] = { NULL };
	if (initGame(&sdl)) return 1;
	// initialize the game
	NewGame(&game, cars);
	int timeStart = game.time.startGame, timeEnd, quit = 0, frames = 0, roadMarkingPos = SCREEN_HEIGHT - 100;
	double fpsTimer = 0, fps = 0;
	sdl.player = SDL_LoadBMP("./assets/car.bmp");

	while (!quit) {
		// Fill window with green color;
		SDL_FillRect(sdl.screen, NULL, SDL_MapRGB(sdl.screen->format, 107, 142, 35));
		DrawDest(&game, &sdl, &roadMarkingPos);
		// info text
		
		timeEnd = SDL_GetTicks();
		game.time.delta = (timeEnd - timeStart) * 0.001;
		timeStart = timeEnd;
		game.time.total += game.time.delta;
		if (game.time.scoreFreeze > 0) game.time.scoreFreeze -= game.time.delta;
		if (game.time.scoreFreeze < 0) game.time.scoreFreeze = 0;
		if (game.time.killMessage > 0) game.time.killMessage -= game.time.delta;
		
		game.totalDistance += game.car.speed * game.time.delta + game.time.delta;
		// WARN - ����� ��������; �������� �������� 0, ��� ���� �� ����
		if (!game.time.scoreFreeze) game.score += modul(game.car.speed) * game.totalDistance * game.time.delta / 2;
		
		game.car.coord.y += (game.car.speed > 0 ? 400 : 100) * game.time.delta * game.car.speed;
		fixCoordY(&game.car.coord.y);
		game.car.coord.x += game.car.turn * game.time.delta * 300;
		fixCoordX(&game.car.coord.x);

		drawBullet(cars, &game, &sdl);
		drawRandomCar(cars, &game, &sdl);
		// draw player
		DrawSurface(sdl.screen, sdl.player, game.car.coord.x, game.car.coord.y);
		DrawHeader(sdl.screen, game, sdl, fps);

		fpsTimer += game.time.delta;
		if (fpsTimer > SECONDS_BETWEEN_REFRESH) {
			fps = frames * REFRESH_RATE;
			frames = 0;
			fpsTimer -= SECONDS_BETWEEN_REFRESH;
		};
		
		renderSurfaces(&sdl);
		
		// handling of events (if there were any)
		while (SDL_PollEvent(&sdl.event)) {
			switch (sdl.event.type) {
			case SDL_KEYDOWN:
				if (sdl.event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
				else if (sdl.event.key.keysym.sym == SDLK_n) NewGame(&game, cars);
				else if (sdl.event.key.keysym.sym == SDLK_UP) game.car.speed = -1;
				else if (sdl.event.key.keysym.sym == SDLK_DOWN) game.car.speed = 1;
				else if (sdl.event.key.keysym.sym == SDLK_LEFT) game.car.turn = -1;
				else if (sdl.event.key.keysym.sym == SDLK_RIGHT) game.car.turn = 1;
				else if (sdl.event.key.keysym.sym == SDLK_SPACE) addBullet(&game);
				break;
			case SDL_KEYUP:
				if (sdl.event.key.keysym.sym == SDLK_UP) game.car.speed = 0;
				else if (sdl.event.key.keysym.sym == SDLK_DOWN) game.car.speed = 0;
				else if (sdl.event.key.keysym.sym == SDLK_LEFT) game.car.turn = 0;
				else if (sdl.event.key.keysym.sym == SDLK_RIGHT) game.car.turn = 0;
				break;
			case SDL_QUIT:
				quit = 1;
				break;
			};
		};
		frames++;
	};

	// freeing all surfaces
	freeSurfaces(sdl);
	return 0;
};
