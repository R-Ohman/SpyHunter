#define _CRT_SECURE_NO_WARNINGS
#include "definitions.h"

void NewGame(Game* game, CarInfo* cars, SDL* sdl) {
	SpawnPlayer(game, cars);
	game->player.sprite = sdl->playerCars[0];
	game->player.colorIndex = 0;
	game->player.lives = 1;
	game->player.liveGain = 0;
	game->time.startGame = SDL_GetTicks();
	game->time.killMessage = 0;
	game->time.scoreFreeze = 0;
	game->time.delta = 0;
	game->time.total = 0;
	game->score = 0;
	game->totalDistance = 0;
	game->bullet.speed = 0;
	game->bullet.coord.y = -200;
	game->bullet.coord.y2 = SCREEN_HEIGHT + 200;
	game->pause = false;
	game->roadWidth = SCREEN_WIDTH / 4;
	for (int i = 0; i <= 1; i++) {
		game->powerCoord[i] = { 0, 0 };
		game->player.powerTime[i] = 0;
	}
};


void SpawnPlayer(Game* game, CarInfo* cars) {
	game->player.coord.x = SCREEN_WIDTH / 2;
	game->player.coord.y = SCREEN_HEIGHT * 2 / 3;
	game->player.speed = 0;
	game->player.turn = 0;
	for (int i = 0; i < ENEMIES; i++) {
		cars[i].coord.x = 0;
	}
};


void addBullet(Game* game, SDL* sdl) {
	if (game->bullet.speed != 0) return;
	game->bullet.sprite = sdl->bullet;
	game->bullet.coord.x = game->player.coord.x;
	game->bullet.coord.y = game->player.coord.y - 30;
	if (game->player.powerTime[0] > 0) game->bullet.coord.y2 = game->player.coord.y + 30;
	else game->bullet.coord.y2 = SCREEN_HEIGHT + 200;
	game->bullet.speed = 2 * CAR_SPEED;
}


void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
};


void changeTimers(Game* game) {
	game->time.total += game->time.delta;
	if (game->time.scoreFreeze > 0) game->time.scoreFreeze -= game->time.delta;
	if (game->time.scoreFreeze < 0) game->time.scoreFreeze = 0;
	if (game->time.killMessage > 0) game->time.killMessage -= game->time.delta;
	for (int i = 0; i <= 1; i++) {
		if (game->player.powerTime[i] > 0) game->player.powerTime[i] -= game->time.delta;
		if (game->player.powerTime[i] < 0) game->player.powerTime[i] = 0;
	}
	if (game->time.killMessage > 0) game->time.killMessage -= game->time.delta;
	if ((int)game->time.total % 30 > 17 && (int)game->time.total < 20) game->roadWidth -= game->time.delta * 70;
	else if ((int)game->time.total % 30 > 2 && (int)game->time.total % 30 < 5) game->roadWidth += game->time.delta * 70;
	else if ((int)game->time.total % 30 > 10 && (int)game->time.total % 30 < 14) game->roadWidth -= game->time.delta * 30;
	else if ((int)game->time.total % 30 > 25 && (int)game->time.total % 30 < 29) game->roadWidth += game->time.delta * 30;
}