#pragma once

#define _USE_MATH_DEFINES
#define SCREEN_WIDTH	1280
#define SCREEN_HEIGHT	720
#define GOD_MODE_TIME	5
#define ENEMIES			5
#define CAR_SPEED		250
#define DATE_FORMAT		"%d-%m-%Y_%H-%M-%S.dat"
#define LEFT_BORDER		SCREEN_WIDTH / 2 - game->roadWidth/2
#define RIGHT_BORDER	SCREEN_WIDTH / 2 + game->roadWidth/2
#define OUT_ROAD		90
#define RES_PER_PAGE	3
#define SORT_TYPE		1
#define SPACING			40
#define POWER_TIME		5


#include<math.h>
#include<stdio.h>
#include <time.h>
#include <windows.h>
#include<string.h>
#include <stdlib.h>
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
#include "vector.h"


struct Coord {
	double x;
	double y;
};


struct Game {
	struct {
		SDL_Surface* sprite;
		int colorIndex;

		Coord coord;
		// powerTime[0] - действие powerup_1 (стрельба), powerTime[1] - действие powerup_2 (скорость)
		double powerTime[2];

		int speed;
		int turn;
		int lives;
		int liveGain;
	} player;

	struct {
		SDL_Surface* sprite;
		struct {
			double x;
			double y;
			double y2;
		} coord;
		int speed;
		double distance;
	} bullet;

	struct {
		double startGame;
		double total;
		double delta;
		double scoreFreeze;
		double killMessage;
	} time;

	Coord powerCoord[2];
	double totalDistance;
	double roadWidth;
	double score;
	bool pause;
};


struct CarInfo {
	SDL_Surface* car;
	Coord coord;

	double speed;
	bool isEnemy;
	int colorIndex;
};


struct Save {
	Game game;
	CarInfo cars[5];
};


struct SDL {
	SDL_Event event;
	SDL_Surface* screen;
	SDL_Surface* charset;
	SDL_Surface* bullet;
	SDL_Surface* cars[6];
	SDL_Surface* playerCars[6];
	SDL_Surface* powerup[2];
	SDL_Surface* liveIcon, * infinityIcon;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;
};


void NewGame(Game* game, CarInfo* cars, SDL* sdl);


void SpawnPlayer(Game* game, CarInfo* cars);


void addBullet(Game* game, SDL* sdl);


void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y);


void changeTimers(Game* game);