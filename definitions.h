#pragma once

#define _USE_MATH_DEFINES
#define SCREEN_WIDTH	1280
#define SCREEN_HEIGHT	720
#define GOD_MODE_TIME	5
#define ENEMIES			5
#define CAR_SPEED		250
#define DATE_FORMAT		"%d-%m-%Y_%H-%M-%S.dat"
#define OUT_ROAD		120
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
		Coord coord;
		double powerTime[2];
		int colorIndex;
		int speed;
		int turn;
		int lives;
		int liveGain;
	} player;

	struct {
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
	double saveMesTime;
	double totalDistance;
	double roadWidth;
	double score;
	bool pause;
};


struct CarInfo {
	Coord coord;
	double speed;
	bool isEnemy;
	int colorIndex;
};


struct SDL {
	SDL_Event event;
	SDL_Surface* screen;
	SDL_Surface* charset;
	SDL_Surface* bullet;
	SDL_Surface* cars[6];
	SDL_Surface* playerCars[5];
	SDL_Surface* powerup[2];
	SDL_Surface* liveIcon, * infinityIcon, * fireIcon;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;
	struct {
		int greyLight;
		int grey;
		int greyDark;
		int green;
		int red;
		int blue;
	} color;
};


struct Save {
	Game game;
	CarInfo cars[5];
};


void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y);


// Resets game variables
void NewGame(Game* game, CarInfo* cars, SDL* sdl);


// Moves the player and removes cars from the screen
void SpawnPlayer(Game* game, CarInfo* cars);


// Sets the bullet sprite and position
void addBullet(Game* game, SDL* sdl);


// Updates timers and road width
void changeTimers(Game* game);