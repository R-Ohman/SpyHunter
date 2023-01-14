#pragma once
#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include <time.h>
#include <windows.h>
#include<string.h>
#include <stdlib.h>
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
#include "vector.h"


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


struct Coord {
	int x;
	int y;
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
			int x;
			int y;
			int y2;
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
	SDL_Surface* liveIcon, *infinityIcon;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;
};


int initGame(SDL* sdl);


// draw a single pixel
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color);


// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color);


// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k,
	Uint32 outlineColor, Uint32 fillColor);


// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface* screen, int x, int y, const char* text,
	SDL_Surface* charset);


// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y);


void DrawDest(Game* game, SDL* sdl, int* roadMarkingPos);


void DrawMenu(SDL sdl);


void DrawRoadRectangle(SDL_Surface* screen, int y);


void DrawHeader(SDL_Surface* screen, Game game, SDL sdl, double fps);


void DrawCommunicates(SDL_Surface* screen, Game game, SDL sdl);


void DrawMenu(SDL sdl);


int DrawPlayer(Game* game, SDL* sdl);


void DrawBullet(CarInfo* cars, Game* game, SDL* sdl);


void DrawRandomCar(CarInfo* cars, Game* game, SDL* sdl);


void DrawRandomPower(CarInfo* cars, Game* game, SDL* sdl);


void NewGame(Game* game, CarInfo* cars, SDL* sdl);


void SaveGame(Game* game, CarInfo* cars, SDL* sdl);


void LoadGame(Game* game, CarInfo* cars, SDL* sdl, char filePath[250]);


void ShowSavedGames(Game* game, CarInfo* cars, SDL* sdl);


//void GetFileName(Game* game, CarInfo* cars, SDL* sdl);


void SpawnPlayer(Game* game, CarInfo* cars);


void AddResult(Game* game, vector_t* resultsList, SDL* sdl);


void SaveResults(vector_t* resultsList);


void LoadResults(vector_t* resultsList);


void getEvent(Game* game, CarInfo* cars, SDL* sdl, int* quit, int* time);


void topResultsMenu(SDL* sdl, vector_t* resultsList, Game* game, CarInfo* cars);


void welcomeMenu(SDL* sdl, vector_t* resultsList, Game* game, CarInfo* cars, int* quit);


void getResultsMenuAction(int* page, SDL* sdl, vector_t* resultsList, const int place, Game* game, CarInfo* cars);


void getWelcomeMenuAction(SDL* sdl, vector_t* resultsList, Game* game, CarInfo* cars, int* quit);


void changeTimers(Game* game);


// check if X and Y coordinates are inside a screen
bool inArray(int x, int y);


// Check ifhorizontal position is out of screen and do it in the range.
bool onTheRoad(int* horizontal, Game* game);


// Check if vertical position is out of screen and do it in the range.
void fixCoordY(int* vertical);


// x,y - координата на периметре машины; x2, y2 - центр объекта
bool numbersInArray(int x, int y, CarInfo* object);


// Проверка для пикселей по периметру машины, находится ли позиция в другом объекте.
bool touchObject(Game* game, CarInfo* object, const double deltaTime, CarInfo* cars, SDL* sdl);


bool isDestroyed(CarInfo* car, SDL* sdl, Game* game);



bool isFreePlace(CarInfo* car, CarInfo* cars, int turn);


double canAttack(CarInfo* car, Game* game, CarInfo* cars);


bool canRide(CarInfo* car, CarInfo* cars);


bool freeSpace(CarInfo* car, CarInfo* cars);


bool inFault(int num1, int num2, int fault);


int modul(int num);


bool canGo(CarInfo* car, CarInfo* cars, int direction);


bool canSpawn(Game* game, CarInfo* cars, SDL* sdl, const int powerIndex);


void addBullet(Game* game, SDL* sdl);


int carIsKilled(Game* game, CarInfo* cars, SDL* sdl, int y);


void RenderSurfaces(SDL* sdl);


void FreeSurfaces(SDL sdl);